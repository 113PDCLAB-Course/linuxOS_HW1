#include <linux/kernel.h>
#include <linux/syscalls.h>
#define TRADTITION_WAY_TO_PMD // 使用 linux 傳統方法得到 pmd table 結果

SYSCALL_DEFINE1(my_get_physical_addresses, void *, ptr_addr)
{
    unsigned long v_addr = (unsigned long)ptr_addr;

#ifdef TRADTITION_WAY_TO_PMD
    pgd_t *pgd;
    pgd = pgd_offset(current->mm, v_addr);
    printk("pgd_val using pointing to = 0x%lx\n",
           pgd->pgd);
    printk("pgd_val using dereferencing = 0x%lx\n",
           *pgd);
    printk("pgd_val using function = 0x%lx\n", pgd_val(*pgd));

    printk("pgd_add = 0x%lx\n", pgd);
    printk("pgd_add = 0x%lx\n", current->mm->pgd);

    /* the index of pgd equal to Add pgd_add to pgd*8.
        mulitply by 8 since the word is 8bytes.
    */
    printk("pgd_index = %lx\n", pgd_index(v_addr));
    p4d_t *p4d;
    p4d = p4d_offset(pgd, v_addr);

    pud_t *pud;
    pud = pud_offset(p4d, v_addr);
    printk("pmd_addr = 0x%lx\n", pud_pgtable(*pud));

    pmd_t *pmd;
    pmd = pmd_offset(pud, v_addr);
    if (pmd_none(*pmd))
    {
        printk("doesn't have memory space for this virtual address\n");
        return (void *)pmd
            ->pmd;
    }
#else
    pmd_t *pmd;
    pmd = pmd_off(current->mm, v_addr);
    if (pmd_none(*pmd))
    {
        printk("doesn't have memory space for this virtual address\n");
        return (void *)pmd->pmd;
    }
#endif

    printk("pmd_val using pointing to = 0x%lx\n", pmd->pmd);
    printk("pmd_val using dereferencing = 0x%lx\n", *pmd);
    printk("pmd_val using function  = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_val with pmd_off_k method = 0x%lx\n",
           *pmd_off(current->mm, v_addr));
    printk("pmd_address with pmd_off_k method = 0x%lx\n",
           pmd_off(current->mm, v_addr));

    printk("pmd_index = %lx\n", pmd_index(v_addr));

    pte_t *pte;
    pte = pte_offset_kernel(pmd, v_addr);

    printk("pte_val = 0x%lx\n", pte->pte);
    printk("pte_val = 0x%lx\n", *pte);
    printk("pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_index = %lx\n", pte_index(v_addr));
    printk("pte_addr = 0x%lx\n", pmd_page_vaddr(*pmd));

    unsigned long offset =
        v_addr & ((unsigned long)4096 -
                  (unsigned long)1); // using 2^12-1 to retain bits from 0 to 11.
    unsigned long p_addr = (pte->pte & PAGE_MASK) |
                           offset; // PAGE_MASK ignores the information of page.

    printk("(pte->pte & PAGE_MASK) = 0x%lx\n", (pte->pte & PAGE_MASK));
    printk("(pte->pte & PAGE_MASK) = %p\n", (pte->pte & PAGE_MASK));
    printk("p_addr = 0x%lx\n", p_addr);
    return p_addr;
}