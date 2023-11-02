# result
```
[39669.084224] general protection fault, maybe for address 0x0: 0000 [#1] PREEMPT SMP PTI
[39669.084229] CPU: 4 PID: 134155 Comm: insmod Kdump: loaded Tainted: G           OE      6.5.8-200.fc38.x86_64 #1
[39669.084232] Hardware name: MECHREVO Z2 Air Series GK5CP5X/GK5CP5X, BIOS N.1.05 07/15/2019
[39669.084234] RIP: 0010:my_init+0xe/0xff0 [cet_check]
[39669.084242] Code: Unable to access opcode bytes at 0xffffffffc210aff4.
[39669.084243] RSP: 0018:ffffabeac1ddfc50 EFLAGS: 00010246
[39669.084245] RAX: 0000000000000000 RBX: ffffffffc210b010 RCX: 00000000000006a0
[39669.084247] RDX: 0000000000000000 RSI: ffffffffc210b010 RDI: ffffabeac1ddfc38
[39669.084248] RBP: ffff9d6bbc566400 R08: 0000000000000020 R09: 0000000000000000
[39669.084249] R10: 0000000000039080 R11: 0000000000000dc0 R12: ffffabeac1ddfc58
[39669.084251] R13: 0000000000000000 R14: ffff9d6bad122900 R15: ffff9d6cb5349820
[39669.084252] FS:  00007fb10a9ec740(0000) GS:ffff9d6edad00000(0000) knlGS:0000000000000000
[39669.084254] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[39669.084255] CR2: ffffffffc210aff4 CR3: 000000013c78c006 CR4: 00000000003726e0
[39669.084257] Call Trace:
...
```

# objdump -S 
```
0000000000000010 <init_module>:
{
  10:   f3 0f 1e fa             endbr64
  14:   e8 00 00 00 00          call   19 <init_module+0x9>
        asm volatile("1: rdmsr\n"
  19:   b9 a0 06 00 00          mov    $0x6a0,%ecx
  1e:   0f 32                   rdmsr           <---- CAUSE #GP
        return EAX_EDX_VAL(val, low, high);
  20:   48 c1 e2 20             shl    $0x20,%rdx
        pr_info("IA32_U_CET val (%lx) \n", val);
  24:   48 c7 c7 00 00 00 00    mov    $0x0,%rdi
        return EAX_EDX_VAL(val, low, high);
  2b:   48 89 d6                mov    %rdx,%rsi
  2e:   48 09 c6                or     %rax,%rsi
        pr_info("IA32_U_CET val (%lx) \n", val);
  31:   e8 00 00 00 00          call   36 <init_module+0x26>
}
  36:   31 c0                   xor    %eax,%eax
  38:   e9 00 00 00 00          jmp    3d <_entry.5+0x1d>
~
```
