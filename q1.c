#include <stdio.h>
#define my_get_physical_addresses(a) syscall(463, a);

// void * my_get_physical_addresses(void *);

int global_a = 123; // global variable

void hello(void)
{
    printf("======================================================================================================\n");
}

int main()
{
    int loc_a;
    void *parent_use, *child_use;

    printf("===========================Before Fork==================================\n");
    parent_use = my_get_physical_addresses(&global_a);
    printf("pid=%d: global variable global_a:\n", getpid());
    printf("Offest of logical address:[%p]   Physical address:[%p]\n", &global_a, parent_use);
    printf("========================================================================\n");

    if (fork())
    {
        /*parent code*/
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvv  After Fork by parent  vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        parent_use = my_get_physical_addresses(&global_a);
        printf("pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", &global_a, parent_use);
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        wait();
    }
    else
    {
        /*child code*/

        printf("llllllllllllllllllllllllll  After Fork by child  llllllllllllllllllllllllllllllll\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("******* pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", &global_a, child_use);
        printf("llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll\n");
        printf("____________________________________________________________________________\n");

        /*----------------------- trigger CoW (Copy on Write) -----------------------------------*/
        global_a = 789;

        printf("iiiiiiiiiiiiiiiiiiiiiiiiii  Test copy on write in child  iiiiiiiiiiiiiiiiiiiiiiii\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("******* pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", &global_a, child_use);
        printf("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
        printf("____________________________________________________________________________\n");
        // sleep(1000);
    }
}