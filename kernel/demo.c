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
    printk("pgd_val = 0x%lx\n", pgd->pgd); // pgd_offset 的內容
    printk("pgd_val = 0x%lx\n", *pgd);     // pgd_offset 的內容

    printk("pgd_val = 0x%lx\n", pgd);              // pgd_offset 的位置。
    printk("pgd_val = 0x%lx\n", current->mm->pgd); // pgd_offset 的位置。

    // pgd_index，找出當前 page gloab table index entry. https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L90
    // 關於 pgd_index 的解釋 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L56
    // #define pgd_index(a)  (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
    // pgd_index 的原理是將右邊的 39bit 右移掉，在與 255 做 &，保證數字必定在 255 以內符合 page table 2^9 架構
    printk("pgd_index = %lx\n", pgd_index(v_addr));

    printk("PGDIR_SHIFT = %lx\n", PGDIR_SHIFT);
    printk("PTRS_PER_PGD = %lx\n", PTRS_PER_PGD - 1);
#endif

    /*  在 x86-64 processors 使用 long mode 模式，採用 PAE 模式。
        它使用 page 12 bit offset, 4 層 9bit 的 page directory，共用到 48bits.
        共用到 48bits，高位元則等待被設計、擴展。
        9bit 的原因是因為，一個 page 4kb 共 2^12 byte.
        在 32bits processors 情況下，一個 word 是 4bytes 共用，能夠存 2^10 個
        在 64bits processors 情況下，一個 word 是 8bytes 共用，能夠存 2^9 個
    */

    //     pud_t *pud;
    //     pud = pud_offset(pgd, v_addr);
    // #ifdef DEBUG
    // 	printk("pud_val = 0x%lx\n", pud->pud); // pgd_offset 的內容
    //     printk("pud_val = 0x%lx\n", pud); // pgd_offset 的位置

    //     // pgd_index，找出當前 page gloab table index entry. https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L90
    //     // 關於 pgd_index 的解釋 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L56
    // 	// printk("pgd_index = %lx\n", pgd_index(v_addr));
    //     printk("pud_index = %lx\n", pud_index(v_addr));
    // #endif

    //     pmd_t *pmd;
    //     pmd = pmd_offset(pud, v_addr);
    // #ifdef DEBUG
    // 	printk("pmd_val = 0x%lx\n", pmd->pmd); // pgd_offset 的內容
    //     printk("pmd_val = 0x%lx\n", pmd); // pgd_offset 的位置

    //     // pgd_index，找出當前 page gloab table index entry. https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L90
    //     // 關於 pgd_index 的解釋 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L56
    // 	// printk("pgd_index = %lx\n", pgd_index(v_addr));
    //     printk("pmd_index = %lx\n", pmd_index(v_addr));
    // #endif

    //     pte_t *pte;

    return pgd->pgd;
}
