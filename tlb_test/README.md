# TLB test 
## PURPOSE
Test the impact of dirty bit on write operations 
while the P or RW set in TLB, but they not set in 
memory.

## Program flow
1. set or clean PTE dirty bit
2. flush tlb all
3. read the page pointed by this PTE  in order to establish new tlb.
4. set or clean P/RW bit  of this PTE
5. write this page pointed by this PTE.

## USAGE
```
1. insmod tlb_test.ko
2. echo 1 > /proc/fuqiang/my_page_proc
3. Write the /proc/fuqiang/my_page_misc_proc based on the test conditions
    a. dirty, not present, rw : echo 2 > /proc/fuqiang/my_page_misc_proc
    b. not dirty, not present, rw : echo 3 > /proc/fuqiang/my_page_misc_proc
    c. dirty, present, read only : echo 4 > /proc/fuqiang/my_page_misc_proc
    d. not dirty, present, read only : echo 5 > /proc/fuqiang/my_page_misc_proc
```

## RESULT
* 3.a, 3.c can normally execute.
* 3.b, 3.d cause #PF

# Related Link
[my own analysis of Linux kernel tlb batch flush](https://github.com/cai-fuqiang/my_book/blob/main/mmu/tlb/tlb_batch_flush.md)

[Discussion on this issue on stackhoverflow](https://stackoverflow.com/questions/77393983/will-an-x86-64-cpu-notice-that-a-page-table-entry-has-changed-to-not-present-whi)
