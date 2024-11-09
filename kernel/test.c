#include <linux/kernel.h>
#include <linux/syscalls.h>
#define DEBUG
#define QUICK_SEARCH

// 使用 marco 的方式並透過 (<type>, <name>) 進行 map 來完成多個參數應用。 https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L117
// backtrace:
/*
__SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L242
SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L226
*/
SYSCALL_DEFINE1(my_get_physical_addresses, void *, ptr_addr)
{
    // TODO: copy_from_user
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

#ifndef QUICK_SEARCH
    // pgd_offset，可直接從 mm 去 get pgd_t https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L147.
    pgd = pgd_offset(current->mm, v_addr);

#ifdef DEBUG
    printk("pgd_val = 0x%lx\n", pgd->pgd); // pgd_offset 的內容
    printk("pgd_val = 0x%lx\n", *pgd);     // pgd_offset 的內容
    printk("pgd_val = 0x%lx\n", pgd_val(*pgd));

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
    // x86, asm 的特殊架構，用意是 to find an entry in a page-table-directory.
    p4d_t *p4d;
    p4d = p4d_offset(pgd, v_addr);

    pud_t *pud;
    pud = pud_offset(p4d, v_addr);
#ifdef DEBUG
    printk("pud_val = 0x%lx\n", pud->pud);
    printk("pud_val = 0x%lx\n", *pud);
    printk("pud_val = 0x%lx\n", pud_val(*pud));
    printk("pud_index = %lx\n", pud_index(v_addr));
#endif
    pmd_t *pmd;
    pmd = pmd_offset(pud, v_addr);
#endif

#ifdef QUICK_SEARCH
    pmd_t *pmd;
    pmd = pmd_off(current->mm, v_addr);
#endif
    if (pmd_none(*pmd))
    {
        printk("doesn't have memory space for this virtual address\n");
        return (void *)pmd->pmd; // TODO: 用 return NULL 看看回傳的結果會不會是 0x0
    }

#ifdef DEBUG
    printk("pmd_val = 0x%lx\n", pmd->pmd);
    printk("pmd_val = 0x%lx\n", *pmd);
    printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_val with pmd_off_k method = 0x%lx\n", *pmd_off(current->mm, v_addr)); // pmd 內容

    // 在 kernel v6.11.5 中有提供 pmd_off， 可直接用 current->mm call 到 pmd_off.
    printk("pmd_address with pmd_off_k method = 0x%lx\n", pmd_off(current->mm, v_addr)); // pmd 位置
    printk("pmd_index = %lx\n", pmd_index(v_addr));
#ifndef QUICK_SEARCH
    printk("pmd_base 0x%lx\n", pud_pgtable(*pud));
#endif
#endif

    pte_t *pte;
    pte = pte_offset_kernel(pmd, v_addr);

#ifdef DEBUG
    printk("pte_val = 0x%lx\n", pte->pte);
    printk("pte_val = 0x%lx\n", *pte);
    printk("pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_index = %lx\n", pte_index(v_addr));
    printk("pte_base = 0x%lx\n", pmd_page_vaddr(*pmd));
#endif

    unsigned long offset = v_addr & ((unsigned long)4096 - (unsigned long)1); // 用 2^12-1 來保留最後的 12bit

    unsigned long p_addr = (pte->pte & PAGE_MASK) | offset;

#ifdef DEBUG
    printk("(pte->pte & PAGE_MASK) = 0x%lx\n", (pte->pte & PAGE_MASK));
    printk("(pte->pte & PAGE_MASK) = %p\n", (pte->pte & PAGE_MASK));
    printk("p_addr = 0x%lx\n", p_addr);
#endif

    // cp_addr = copy_to_user(p_addr)
    // TODO: copy_to_user

    return p_addr;
}