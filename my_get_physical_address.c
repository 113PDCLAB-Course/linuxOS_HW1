#include <stdio.h>
#include <linux/kernel.h>
#include <linux/module.h>
// #include <linux/init.h>
// #include <linux/mm.h>
// #include <linux/highmem.h>
// #include <asm/page.h>
// #include <asm/io.h>

void *my_get_physical_addresses(void *p)
{ // logical address
    printf("%p\n", p);
    unsigned long long vaddr = (unsigned long long)vaddr;
    unsigned long long paddr = virt_to_phys(vaddr);

    printf("%lld\n", paddr);
}
