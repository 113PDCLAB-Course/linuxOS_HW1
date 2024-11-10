#include <linux/kernel.h>
#include <linux/syscalls.h>
// #define ORIGANAL_KERNEL

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
    printk("pud_val = 0x%lx\n", pud->pud);
    printk("pud_addr = 0x%lx\n", pud_pgtable(*pud));

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
        return -EFAULT;
    }
#endif

    printk("pmd_val using pointing to = 0x%lx\n", pmd->pmd);
    printk("pmd_val using dereferencing = 0x%lx\n", *pmd);
    printk("pmd_val using function  = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_val with pmd_off method = 0x%lx\n",
           *pmd_off(current->mm, v_addr));
    printk("pmd_address with pmd_off method = 0x%lx\n",
           pmd_off(current->mm, v_addr));
    printk("pmd_index = %lx\n", pmd_index(v_addr));

    pte_t *pte;
    pte = pte_offset_kernel(pmd, v_addr);
    if (pte_none(*pte))
    {
        printk("doesn't have memory space for this virtual address\n");
        return EFAULT;
    }

    printk("pte_val using pointing to = 0x%lx\n", pte->pte);
    printk("pte_val using dereferencing = 0x%lx\n", *pte);
    printk("pte_val using pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_addr with pte_offset_kernel method = 0x%lx\n",
           pte_offset_kernel(pmd, v_addr));
    printk("pte_index = %lx\n", pte_index(v_addr));
    printk("pte_base = 0x%lx\n", pmd_page_vaddr(*pmd));

    printk("pte_val using pte_val = 0x%lx\n", pte_val(*pte));
    unsigned long pfn = pte_pfn(
        *pte); // ignore 58~63bit and 0~12 bit, extract the page physical address
    printk("pfn_val using function = 0x%lx\n", pfn);

    unsigned long offset =
        v_addr &
        ((unsigned long)4096 -
         (unsigned long)1); // using 2^12-1 to retain bits from 0 to 11.
    unsigned long p_addr = (pfn << PAGE_SHIFT) | offset;

    printk("v_addr = 0x%lx\n", v_addr);
    printk("p_addr = 0x%lx\n", p_addr);

    // copy_to_user(ptr_addr, &p_addr, sizeof(unsigned long));
    return (void *)p_addr;
}