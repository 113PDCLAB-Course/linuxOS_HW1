#include <stdio.h>
#include <stdio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <stdlib.h>

void *my_get_physical_addresses(void *p)
{ // logical address
    printf("%p\n", p);
    unsigned long long vaddr = (unsigned long long)vaddr;
    unsigned long long paddr = virt_to_phys(vaddr);

    printf("%lld\n", paddr);
}

int main(int argc, char *argv[])
{
    int a = 4;
    int *b = &a;
    my_get_physical_addresses(b);
}
