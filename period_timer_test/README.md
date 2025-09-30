
## Reproduction steps
1. Build Linux Kernel at link<sup>1</sup>, then install and reboot
2. Build the module in this dir and load it by adding target_cpu=16.
3. Build kvm-unit-test at link<sup>2</sup>, to start a vm that always enable
   period timer and run it at cpu 16
   ```
   taskset -c 16 ./x86-run x86/apic.flat
   ```
5. Observe the dmesg output

## DMESG

* patch not merged(not define MERGE_PATCH macro in source<sup>1</sup>)
  ```
  [ 2494.475518] hrtimer fired on cpu 16, now(244c6846d40) delta(b1)
  [ 2494.493336] kvm: sw_exp times delta 0
  [ 2494.493484] kvm: hw_exp times delta 3621
  [ 2494.493619] kvm: the delta(fffffffffef3aa16) tscdeadline(a666490e2fd8f6b2) delta_cyc_u(45722546) delta_cyc_s(-45722546)
  [ 2494.493890] kvm_intel: the delta_tsc is too large (a6664909fd304ce5)
  [ 2495.475518] hrtimer fired on cpu 16, now(245021f389e) delta(20f)
  [ 2496.475518] hrtimer fired on cpu 16, now(2453dba0358) delta(2c9)
  [ 2496.475885] kvm: sw_exp times delta 9536
  [ 2496.476033] kvm: hw_exp times delta 0
  [ 2496.476180] kvm: the delta(fffffffffffebc55) tscdeadline(a666490f65cc82e9) delta_cyc_u(215432) delta_cyc_s(-215432)
  [ 2497.475518] hrtimer fired on cpu 16, now(2457954cbc6) delta(137)
  [ 2498.475518] hrtimer fired on cpu 16, now(245b4ef95e1) delta(152)
  [ 2498.475897] kvm: sw_exp times delta 9537
  [ 2498.476058] kvm: hw_exp times delta 0
  ```
* patch merged(define MERGE_PATCH macro in source<sup>1</sup>)
   ```
   [ 2693.475510] hrtimer fired on cpu 16, now(2731bd7737f) delta(f0)
   [ 2693.493454] kvm: sw_exp times delta 0
   [ 2693.493568] kvm: hw_exp times delta 1163
   [ 2693.493676] kvm: the delta(fffffffffef18960) tscdeadline(3e03d9138) delta_cyc_u(46085104) delta_cyc_s(-46085104)
   [ 2694.475510] hrtimer fired on cpu 16, now(27357723d6e) delta(df)
   [ 2694.828220] kvm: sw_exp times delta 0
   [ 2694.828350] kvm: hw_exp times delta 6449
   [ 2694.828473] kvm: the delta(ffffffffffff662a) tscdeadline(4b1d50cba) delta_cyc_u(102392) delta_cyc_s(-102392)
   [ 2695.475511] hrtimer fired on cpu 16, now(273930d08a5) delta(216)
   [ 2696.475511] hrtimer fired on cpu 16, now(273cea7d2d6) delta(247)
   [ 2696.493472] kvm: sw_exp times delta 0
   [ 2696.493615] kvm: hw_exp times delta 7857
   ```

When the patch is not merged, if hyp finds that the delta is less than 0 when
the timer expires, it will continue to use the software timer afterwards. After
the patch is merged, this issue no longer occurs.

## Related Link
1. [kernel: add some debug](https://github.com/cai-fuqiang/linux/tree/fix_period_timer_add_deb)
2. [kvm-unit-test: keep the period timer enabled](https://github.com/cai-fuqiang/kvm-unit-tests/tree/loop_period_timer)
