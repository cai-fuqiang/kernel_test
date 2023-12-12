
```
If software modifies a paging-structure entry that identifies the final
 physical address for a linear address(either a PTE or a paging-structure 
entry in which the PS flag is 1) to change the dirty flag from 1 to 0, 
failure to perform an invalidation may result in the processor not setting
 that bit in response to a subsequent write to a linear address whose 
translation uses the entry. Software cannot interpret the bit being clear as 
an indication that such a write has not occurred
```
However, this did not modify the PTE finally. If it is necessary to modify the PTE (such as dirty bit), I also believe that a full check should be done.

I think the Linux kernel code has the characteristics of using the above.

COMMIT:
```
```

When unmapping a page, first modify the pte (clear pte), and if the page is dirty, immediately flush the remote tlb(tlb shootdown). If the page is not, you can proceed with the `pageout()` first, then perform flush.

If the page is dirty, it indicates that someone has recently accessed the page, and if there is no flush tlb remote, other CPUs may still be able to access it normally without generating PF. However, if the page is not dirty, after clearing the pte, even if other CPUs use old tlb to access it (the dirty flag in old tlb is 0), the dirty flags will be updated and a full check will be made to generate PF.
(The above is my own understanding, I cannot guarantee it is correct).

Thanks, Peter.

*** 

The issue comes from a Linux kernel commit. Based on the code logic of the
commit, I tend to believe that the situation mentioned above can cause page
faults.

Information about COMMIT:
```
commit d950c9477d51f0cefc2ed3cf76e695d46af0d9c1
Author: Mel Gorman <mgorman@suse.de>
Date:   Fri Sep 4 15:47:35 2015 -0700

    mm: defer flush of writable TLB entries
```

The code is no longer shown here, and the approximate logic is:

When `kswapd` was performing memory reclamation, it was found that a page could
be reclaimed. First, unmap it The unmap operation will modify the page table
structures After modification, it is necessary to perform a tlb flush (may
execute tlb shootdown) However, the kernel may delay the tlb flush action in
order to perform batch flush (as tlb shootdown requires sending ipi, batch
flush will reduce the number of ipi sent to one). But it needs to be determined
whether the PTE is dirty before the clear PTE,

If it's dirty, you need to flush immediately If it weren't dirty, you can batch
flush it after `page_out()`

I think it may be due to the following reasons:

If the page is dirty, it indicates that someone has recently accessed the page,
and if there is no flush tlb remote, other CPUs may still be able to access it
normally without generating PF. However, if the page is not dirty, after
clearing the pte, even if other CPUs use old tlb to access it (the dirty flag
in old tlb is 0), the dirty flags will be updated and a full check will be made
to generate PF. (The above is my own understanding, I cannot guarantee it is
correct).

If modify dirty bit 1 to 0, but not flush tlb. This situation is 
described in Intel SDM
```
If software modifies a paging-structure entry that identifies the final
 physical address for a linear address(either a PTE or a paging-structure 
entry in which the PS flag is 1) to change the dirty flag from 1 to 0, 
failure to perform an invalidation may result in the processor not setting
 that bit in response to a subsequent write to a linear address whose 
translation uses the entry. Software cannot interpret the bit being clear as 
an indication that such a write has not occurred
```

But regarding the issue mentioned at the beginning, it was not 
found in Intel SDM.


Thanks for Peter's testing suggestion. I have written a kernel module based
on the Linux kernel. The testing code is not specifically presented, but the
program flow is as follows:
> 1. set or clean PTE dirty bit
> 2. flush tlb all
> 3. read the page pointed by this PTE in order to establish new tlb.
> 4. clean P or RW bit of this PTE
> 5. write this page pointed by this PTE.

The test results are as follows:
1. set dirty, clear present, (set RW) : normally execute
2. set dirty, clear RW(read only), (set present) : normally execute
3. set not dirty, clear present, (set RW) : cause #PF
4. set not dirty, clear RW(read only), (set present) : cause #PF

> NOTE
>
> Set/clear the dirty bit is to set the initial state of TLB

> [my test link](https://github.com/cai-fuqiang/kernel_test/tree/master/tlb_test)
