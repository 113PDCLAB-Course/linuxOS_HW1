#include <linux/kernel.h>
#include <linux/syscalls.h>
#define DEBUG

// 使用 marco 的方式並透過 (<type>, <name>) 進行 map 來完成多個參數應用。 https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L117
// backtrace:
/*
__SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L242
SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L226
*/
SYSCALL_DEFINE1(demo, void *, ptr_addr)
{
    unsigned long v_addr = (unsigned long)ptr_addr;
    pgd_t *pgd;
    // 使用 `arch` 來查看目前電腦的架構是那一種，以實驗機為例是使用 x86_64 架構，asm insturction set 架構
    // current 為目前的 process pointer https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/current.h#L52
    // current 型態是 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/sched.h#L758
    // current->mm 表達 the process address space，src https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/sched.h#L758
    // mm struct https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/mm_types.h#L779
    // mm->pgd_t 表達 global page table，https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/mm_types.h#L806
    // pgd_t 的結構 https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/pgtable_types.h#L295
    // pgd_t->pgdval_t 的型態 https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/pgtable_64_types.h#L18

    // pgd_offset，可直接從 mm 去 get pgd_t https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L147.
    pgd = pgd_offset(current->mm, v_addr);

#ifdef DEBUG
    printk("pgd_val = 0x%lu\n", pgd->pgd);
    // pgd_index，找出當前 page gloab table index entry. https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L90
    printk("pgd_index = %lu\n", pgd_index(v_addr));
#endif

    /*  在 x86-64 processors 使用 long mode 模式，採用 PAE 模式。
        它使用 page 12 bit offset, 4 層 9bit 的 page directory，共用到 48bits.
        共用到 48bits，高位元則等待被設計、擴展。
        9bit 的原因是因為，一個 page 4kb 共 2^12 byte.
        在 32bits processors 情況下，一個 word 是 4bytes 共用，能夠存 2^10 個
        在 32bits processors 情況下，一個 word 是 8bytes 共用，能夠存 2^9 個
        因此要觀察 pointer 的 48bits~39bits 是否與 index 一樣 。
    */
    return pgd->pgd;
}