# 使用gdb调试`16bit real mode code`
## 1 gdb target 设置
* 原因: 在使用gdb调试实模式代码时，会遇到类似于`x/10i`这样的命令
显示出错误的指令解码。需要在启动gdb时指定target的执行环境的架构(i8086)
* 查看gdb.txt

```
set tdesc filename target_i386_64.xml
set architecture i386:x86-64
```
查看`target_i386_64.xml`
```
<?xml version="1.0"?>
<!DOCTYPE target SYSTEM "gdb-target.dtd">
<target>
        <architecture>i8086</architecture>
        <xi:include href="i386-64bit.xml"/>
</target>
```
该文集那中制定了i8086架构的配置文件为`i386-64bit.xml`，
`i386-64bit.xml`配置文件可在该qemu源码路径中获取:
```
${QEMU_CODE}/gdb-xml/
```
## 2 使用`x/10i`查看内存，以指令格式显示
```
(gdb) x/5i 0x7c00
=> 0x7c00 <mystart>:    (bad)
   0x7c01 <mystart+1>:  add    $0x3100007c,%eax
   0x7c06 <.setcs+1>:   rorb   $0xd0,-0x713f7128(%rsi)
   0x7c0d <.setcs+8>:   mov    $0x8c5c,%sp
   0x7c11 <.setcs+12>:  add    %al,(%rax)
(gdb) set architecture i8086
The target architecture is set to "i8086".
(gdb) x/5i 0x7c00
   0x7c00 <mystart>:    ljmp   $0x0,$0x7c05
   0x7c05 <.setcs>:     xor    %ax,%ax
   0x7c07 <.setcs+2>:   mov    %ax,%ds
   0x7c09 <.setcs+4>:   mov    %ax,%es
   0x7c0b <.setcs+6>:   mov    %ax,%ss
```
在执行命令`set architecture i8086`命令前，显示的指令格式为
64位的指令格式，原因是在之前的gdb.txt脚本中执行了
`set architecture i386:x86-64`指明了`target architecture`的
是`i386:x86-64`64位，设置为i8086后，指令格式显示正常。为什么不
在gdb.txt中设置`architecture`为i8086呢，请看下面

## 3 使用`hb`打断点

### 3.1 使用`target architecture i8086`

```
➜  c_hello_world git:(master) ✗ cat gdb.txt
set tdesc filename target_i386_64.xml
architecture i8086                                      #<<<<<<<<<<<<<<< (1)

➜  c_hello_world git:(master) ✗ ./gdb.command
GNU gdb (GDB) 10.2

Reading symbols from main.elf...
Remote debugging using localhost:1234
0x00000000 in ?? ()                                     #<<<<<<<<<<<<<<< (2)
(gdb) i r
eax            0x0                 0
ecx            0x0                 0
edx            0x0                 0
ebx            0x0                 0
esp            0x0                 0x0
ebp            0x0                 0x0
esi            0x906ea             591594
edi            0x0                 0
eip            0x0                 0x0                  #<<<<<<<<<<<<<<< (3)
(gdb) hb *0x7c00
Hardware assisted breakpoint 1 at 0x7c00: file entry.S, line 5.
(gdb) c
Continuing.

Program received signal SIGTRAP, Trace/breakpoint trap.
0x00000000 in ?? ()                                     #<<<<<<<<<<<<<<< (4)
(gdb) i r
eax            0xaa55              43605
ecx            0x0                 0
edx            0x0                 0
ebx            0x0                 0
esp            0x0                 0x0
ebp            0x0                 0x0
esi            0x80                128
edi            0x0                 0
eip            0x0                 0x0                  #<<<<<<<<<<<<<<< (5)
eflags         0x0                 [ ]
cs             0x0                 0
ss             0x0                 0
ds             0x0                 0
es             0x0                 0
fs             0x6f04              28420
gs             0x0                 0
(gdb) c
Continuing.  Program received signal SIGTRAP, Trace/breakpoint trap.
0x00000000 in ?? ()                                     #<<<<<<<<<<<<<<< (6)
(gdb)
```
可以看出，在使用`target architecture i386:x86-64`时，打断点，和查看寄存
器都会有一些问题, 结合上面标注解释如下:

1. 设置`target architecture`为`i8086`
2. 当gdb client连接到gdb server时，显示的ip不对，这里应该为`0xfff0`, 是
在bios的入口地址。
3. `i r`指令打印的寄存器列表也有问题，同理，从`eip`的值可以看出。
4. 执行完，`hb *0x7c00; c`命令后确实会断住，但是显示的`ip`不对.
5. 寄存器列表显示的值不对.
6. 再次执行`c`指令依然会断住。

PS: 
1. 使用qemu-system-i386启动qemu, 并设置`set architecture i8086`不会有问题，
怀疑是`qemu-system-x86_64`和`qemu-system-i386`中的gdb-server返回的数据不同
路径`oth/pcap`中有在执行gdb target到remote gdb-server时，抓取gdb server port
的tcp报文。发现有关寄存器列表中的报文，有一些差别。
2. 在使用`hb *0x7c00; c`会断住，但是继续`c`仍然断住，第一次断住，确实是断在
了`0x7c00`, 可以通过qemu monitor获取寄存器得到，再次执行`c`还是会卡在这个地方，
原因未知.

### 3.2 使用`target architecture i386:x86-64`

```
➜  c_hello_world git:(master) ✗ ./gdb.command
GNU gdb (GDB) 10.2
...

warning: Selected architecture i386:x86-64 is not compatible with reported target architecture i8086
0x000000000000fff0 in ?? ()                             #<<<<<<<<<<<<<<<<(1)
(gdb) i r                                               #<<<<<<<<<<<<<<<<(2)
rax            0x0                 0
rbx            0x0                 0
rcx            0x0                 0
rdx            0x906ea             591594
rsi            0x0                 0
rdi            0x0                 0
rbp            0x0                 0x0
rsp            0x0                 0x0
r8             0x0                 0
r9             0x0                 0
r10            0x0                 0
r11            0x0                 0
r12            0x0                 0
r13            0x0                 0
r14            0x0                 0
r15            0x0                 0
rip            0xfff0              0xfff0
eflags         0x2                 [ ]
cs             0xf000              61440
ss             0x0                 0
ds             0x0                 0
es             0x0                 0
fs             0x0                 0
gs             0x0                 0
(gdb) hb *0x7c00
Hardware assisted breakpoint 1 at 0x7c00: file entry.S, line 5.
(gdb) c
Continuing.

Breakpoint 1, mystart () at entry.S:5                   #<<<<<<<<<<<<<<<<(3)
5           ljmp $0, $.setcs
(gdb) i r
#<<<<<<<<<<<<<<<<(4)
rax            0xaa55              43605
rbx            0x0                 0
rcx            0x0                 0
rdx            0x80                128
rsi            0x0                 0
rdi            0x0                 0
rbp            0x0                 0x0
rsp            0x6f04              0x6f04
r8             0x0                 0
r9             0x0                 0
r10            0x0                 0
r11            0x0                 0
r12            0x0                 0
r13            0x0                 0
r14            0x0                 0
r15            0x0                 0
rip            0x7c00              0x7c00 <mystart>
eflags         0x202               [ IF  ]
cs             0x0                 0
ss             0x0                 0
ds             0x0                 0
es             0x0                 0
fs             0x0                 0
gs             0x0                 0
(gdb) c                                                 #<<<<<<<<<<<<<<<<(5)
Continuing.

```
1. 虽然ip的格式为64bit，但是值没有问题。
2. `i r`命令显示的寄存器列表值, 没有问题, 从`cs 0xf000 ;rip 0xfff0`可以看出
3. 使用断点，可以断住
4. `i r`可以看出断住的地址是正确的, 从`cs 0x0; rip 0x7c00`可以看出
5. 使用`c`命令可以, 不再会卡住

## 4. 编译gdb

* 原因: 指定target 为i8086时，需要配置文件，该配置文件是xml格式，
而红帽的gdb不支持解析xml.所以要重新编译gdb支持这一选项
* 版本：10.2 （低版本在编译--with-expat时候可能会出错）
* 编译方法：
```
./configure   --with-expat
```

