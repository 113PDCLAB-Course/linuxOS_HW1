#include <linux/kernel.h>
#include <linux/syscalls.h>
#define ORIGANAL_KERNEL

SYSCALL_DEFINE1(my_get_physical_addresses, void *, ptr_addr)
{
    // printk("ptr_addr = 0x%lx\n", ptr_addr); // pgd_offset 的內容
    // unsigned long v_addr = 0;
    // copy_from_user(&v_addr, ptr_addr, sizeof(*ptr_addr));
    unsigned long v_addr = (unsigned long)ptr_addr;
    // unsigned long v_addr = (unsigned long)vp_addr;

#ifdef ORIGANAL_KERNEL

    pgd_t *pgd;
    pgd = pgd_offset(current->mm, v_addr);
    printk("pgd_val using pointing to = 0x%lx\n",
           pgd->pgd); // pgd_offset 的內容
    printk("pgd_val using dereferencing = 0x%lx\n",
           *pgd); // pgd_offset 的內容
    printk("pgd_val using function = 0x%lx\n", pgd_val(*pgd));

    printk("pgd_addr = 0x%lx\n", pgd);              // pgd_offset 的位置。
    printk("pgd_addr = 0x%lx\n", current->mm->pgd); // pgd_offset 的位置。
    printk("pgd_index = %lx\n", pgd_index(v_addr));
    p4d_t *p4d;
    p4d = p4d_offset(pgd, v_addr);
    printk("p4d_val using pointing to = 0x%lx\n",
           p4d->p4d); // p4d_offset 的內容

    pud_t *pud;
    pud = pud_offset(p4d, v_addr);
    printk("pmd_addr = 0x%lx\n", pud_pgtable(*pud));

    pmd_t *pmd;
    pmd = pmd_offset(pud, v_addr);
    if (pmd_none(*pmd))
    {
        printk("doesn't have memory space for this virtual address\n");
        return pmd
            ->pmd; // TODO: 用 return NULL 看看回傳的結果會不會是 0x0
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
    printk("pmd_base = 0x%lx\n", pmd_page_vaddr(*pmd));
    printk("pmd_index = %lx\n", pmd_index(v_addr));

    pte_t *pte;
    pte = pte_offset_kernel(pmd, v_addr);
    if (pte_none(*pte))
    {
        printk("doesn't have memory space for this virtual address\n");
        return (void *)pte->pte;
    }

    printk("pte_val = 0x%lx\n", pte->pte);
    printk("pte_val = 0x%lx\n", *pte);
    printk("pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_index = %lx\n", pte_index(v_addr));

    unsigned long offset =
        (unsigned long)v_addr &
        ((unsigned long)4096 -
         (unsigned long)1); // 用 2^12-1 來保留最後的 12bit
    unsigned long p_addr = (pte->pte & PAGE_MASK) |
                           offset; // PAGE_MASK 移除 page_table 的資訊

    printk("(pte->pte & PAGE_MASK) = 0x%lx\n", (pte->pte & PAGE_MASK));
    printk("(pte->pte & PAGE_MASK) = 0x%lx\n", (pte->pte & PAGE_MASK));
    printk("p_addr = 0x%lx\n", p_addr);

    copy_to_user(ptr_addr, &p_addr, sizeof(unsigned long));
    return ptr_addr;
}