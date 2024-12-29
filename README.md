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

### 上面 code 的一些補充理論，可略過
* marco 定義
```c
// 使用 marco 的方式並透過 (<type>, <name>) 進行 map 來完成多個參數應用。 https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L117
// backtrace:
/*
__SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L242
SYSCALL_DEFINEx https://elixir.bootlin.com/linux/v6.1.51/source/include/linux/syscalls.h#L226
*/
```
* page directory 
```c
// 使用 `arch` 來查看目前電腦的架構是那一種，以實驗機為例是使用 x86_64 架構，asm insturction set 架構
// current 為目前的 process pointer https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/current.h#L52
// current 型態是 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/sched.h#L758
// current->mm 表達 the process address space，src https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/sched.h#L758
// mm struct https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/mm_types.h#L779
// mm->pgd_t 表達 global page table，https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/mm_types.h#L806
// pgd_t 的結構 https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/pgtable_types.h#L295
// pgd_t->pgdval_t 的型態 https://elixir.bootlin.com/linux/v6.11.5/source/arch/x86/include/asm/pgtable_64_types.h#L18
// pgd_offset，可直接從 mm 去 get pgd_t https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L147.
// pgd_index，找出當前 page gloab table index entry. https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L90
// 關於 pgd_index 的解釋 https://elixir.bootlin.com/linux/v6.11.5/source/include/linux/pgtable.h#L56
// #define pgd_index(a)  (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
// pgd_index 的原理是將右邊的 39bit 右移掉，在與 255 做 &，保證數字必定在 255 以內符合 page table 2^9 架構
```
* 有相關聯的的計算機組織
```c
/*  在 x86-64 processors 使用 long mode 模式，採用 PAE 模式。
        它使用 page 12 bit offset, 4 層 9bit 的 page directory，共用到 48bits.
        共用到 48bits，高位元則等待被設計、擴展。
        9bit 的原因是因為，一個 page 4kb 共 2^12 byte.
        在 32bits processors 情況下，一個 word 是 4bytes 共用，能夠存 2^10 個
        在 64bits processors 情況下，一個 word 是 8bytes 共用，能夠存 2^9 個
    */
    // x86, asm 的特殊架構，用意是 to find an entry in a page-table-directory.

    /*
    page table 是使用 0~64bit 紀錄資料，12~64 bit 資料是 physical page address.
    0~11 bit 則是用於記這個 page table 的其他供用，例如 read/write, dirty, cache disable 之類的
    page table 圖表網址連結：https://stackoverflow.com/questions/68025493/the-size-of-a-page-table-entry
    */

    // PAGE_MASK 用於 ignore page table 0~11bit 的 page 資訊。
    // 將 offset 與 memory physical address 合併就是完整的 physical address
```

## 實驗結果
### 實驗設定
* `x86` 架構
* `asm` 組合語言
* 12 核心
* 32 GB 

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

### 記憶體驗證
* 最後 3 位數必須相同，因為 offset 是 12bit，virtual address and physical address 12 bits 相同。 
* 記憶體大小限制，以實驗機為例 32GB，是 `2^(30+5)` bytes 換算成 16 進制為 800000000 bytes，所以實體 memory 位置不可以大於 800000000. 

## 助教題目回答
1. 甚麼時候會用到system call
需要使用到 system call 去使用 kernel space 的程式時
2. fork 的marco是甚麼
macro 本質上是文字替換，它會在 complier 時進行替換，將一個命令換成一系列指令
3. copy from user/copy to user 是在做什麼
將 user space variable 安全的 copy to kernel space, vice versa 
4. make 那三條指令是什麼意思
    * `make -j12` complier kernel with 12 core.
    * `make modules_install -j12` install kernel module 
    * `make install -j12` install kernel. 
5. bss segment /data segment 是指甚麼 
    * bss segment 用於儲存尚未初始化的 global variable 
    * data segment 用於儲存已經初始化的 global variable 
6. mm_struct本身是什麼struct -> task struct
用於管理 memory 的 struct，在 task struct 下的一個參數，task strcut 用於表達當前執行的 process 
7. page fault 甚麼時候會發生
當 virtual memory can't map to physical memory. 
8. 我給忘了

## 參考連結
* [linux kernel 編譯 system call](https://hackmd.io/aist49C9R46-vaBIlP3LDA?view#%E6%B8%AC%E8%A9%A6-syscall) 
* [實作一個回傳物理位址的系統呼叫](https://hackmd.io/@Mes/make_phy_addr_syscall)
