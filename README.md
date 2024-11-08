# linux_OS
The Project of Linux Operation System on Falling 2023.  

> [作業要求1](./hw1.html)
> [作業連結 hackmd](https://hackmd.io/@wysh/Bk92kxRgJl)

## 解釋 `sys_my_get_physical_addresses`  
### 此 system call 主要流程
* 使用 `SYSCALL_DEFINE1` 來為此 system call 放入一個參數
* 透過 `ptr_addr` 來傳入 virtual address
* 使用 `pmd_off` 這個 linux 提供的 shortcut function 快速得到 `pmd` 這個 table
    * page-table 走訪紀錄 `pgd->p4d->pud->pmd->pte->physical_address`，如使用 shortcut 就以 pmd 開始
* 最後 `pte` 得到的 page，需要將最後 12bit 用於紀錄 page information 資訊給去除，並加上 logical address 上的 offset. 
* 圖表：logical address 轉換成 phyiscal address
![](https://hackmd-prod-images.s3-ap-northeast-1.amazonaws.com/uploads/upload_b69c4f3202f8c334e1d7d6777b259b9a.png?AWSAccessKeyId=AKIA3XSAAW6AWSKNINWO&Expires=1731075734&Signature=JF41Sn0eakvm%2FHApeTw0Uv2pLRE%3D)

### Code 
* `<page_table_name>_add` 表示 `<page_table_name>` 的 address
* `<page_table_name>_val` 表示 `<page_table_name>` 的 value
* `<page_table_name>_index` 表示 `<page_table_name>` 的 index

```c
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
```

## 實驗結果
### 實驗設定
* `x86` 架構
* `asm` 組合語言
### Q1 
* [題目提供的程式碼位置](./q1.c) 
* 大綱：在 fork 以後，加上由於 copy-on-write 技術，global 變數會被共用，但當任一個 process 修改 gloabl variable 時，則 child process 會與 parent process 不再共用,當虛擬位置會相同。 
* [更詳細的說明](https://stackoverflow.com/questions/4298678/after-forking-are-global-variables-shared)
* [題目執行後結果](./q1.txt)

### Q2
* [題目提供的程式碼位置](./q2.c) 
* 大綱：當提供一個尚未初始化的 global variable 時，會被編譯器放入 bss segment 而非 data segment，此時採用 lazy allocation 策略，會等到 page fault 才載入記憶體，因此在 physical address 會顯示 error。
* 關鍵字：lazy allocation. 
* [更詳細的說明](https://www.quora.com/When-does-a-global-variable-get-memory-allocation-in-C-language)
* [題目執行後結果](./q2.txt)


## 參考連結
* [linux kernel 編譯 system call](https://hackmd.io/aist49C9R46-vaBIlP3LDA?view#%E6%B8%AC%E8%A9%A6-syscall) 
* [實作一個回傳物理位址的系統呼叫](https://hackmd.io/@Mes/make_phy_addr_syscall)
