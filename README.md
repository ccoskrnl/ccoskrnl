# CCOSKRNL

`ccoskrnl`  是一个运行在`x86_64` 架构的64位操作系统项目，该项目只是处于学习和研究的目的一时兴起而开发的，该项目仍然处于早期开发阶段。由于生活和学业等原因，该项目并不会频繁更新。

我之前也学习过玩具级别的操作系统开发，国内也有很多类似的课程，比如在`QEMU`或者`Bochs`上开发一个玩具级的只可在虚拟机上运行的操作系统。这确实一个非常不错的实验项目，通过对这些实验的学习，学生可以亲自编写它们自己的内核，体会到内核开发的乐趣。不过缺点也是显而易见的，现在计算机系统早已与几十年前相去甚远，而现在的多数计算机系统课程仍然只停留在科普层面。在多数情况下，一个通用性处理器系统的设计难度远超过专用处理器系统的设计，通过对个人PC和服务器硬件的操作系统开发，学生可以很明显的感受到这一点。该项目与它们不同之处就在于，在设计之初就不只为了在虚拟机运行，而是一个真正的，可在实际物理机上运行的，并且与现代硬件进行交互。

## 演示

![demo](./demo/sample.png)

## 实现

### x86_64

`x86_64` 是个人PC选用的最多的架构，也是一个历史非常悠久的架构，拥有极强的向前兼容和通用性。`ccoskrnl` 在实现过程中同时参考了 `intel-sdm` 和 `amd64手册`，在开发过程中尽量去为两种架构的细微差异去做适配调整。

### UEFI

`ccoskrnl` 使用UEFI作为引导，UEFI非常适合操作系统加载器的开发。开发者可以直接调用UEFI提供好的接口，与传统引导（legacy）相比，开发者不再需要使用0号扇区，在狭小的512字节中使用BIOS提供的int功能调用来编写紧凑的汇编代码来进行繁琐的硬件探测和内存类型探测等一些启动检查。

事实上，`BOOTX64.efi`在进行一些检查和系统硬件信息收集工作之后就简单的记录和划分内核空间，并进入到内核启动的下一个阶段。`ccoskrnl` 在启动过程中仍然需要一个二进制程序(`ccldr`)，这个二进制程序完全由汇编编写。它被加载到内存中，在UEFI服务结束之后跳转到这个程序。`ccldr` 把内核地址空间映射到虚拟内存的高地址处，并设置GDT等一些必要的数据结构，最后跳转到内核初始化代码。

### 多核

多核支持对我来说是一个艰难的挑战。`ccoskrnl` 将应用核心的初始化例程放在了一个单独的二进制文件`ap_startup`中。这个二进制程序同样由UEFI进行加载，由于`x86_64`的要求，AP的初始化只能在低1MiB的地址空间进行。同时我们需要构造一个临时的页表，在BSP的内存管理初始化时我们设置PML4的首个项（因为`x86_64`要求这段代码必须在低1MiB处）。当BSP需要在虚拟内存空间中对该程序进行手动重定位，修复GDT和IDT项目，并设置AP的初始化代码地址。[多核的激活参考](https://stackoverflow.com/questions/56384291/what-happens-to-a-startup-ipi-sent-to-an-active-ap-that-is-not-in-a-wait-for-sip)

### APIC

APIC 是'x86_64'架构中的一个非常重要的组件。APIC同时也是一个非常复杂的设置，想要完全理解它需要深厚的计算机系统知识。`ccoskrnl` 目前仅实现了中断管理。

### 字符渲染

`ccoskrnl` 一开始并没有选择使用位图字体，而是尝试进行`Truetype` 字体渲染，但是我并没有提供一个良好的字体渲染引擎，而对于字体`Hint`和字符填充则是完全没有实现。并且轮廓线绘制算法也是非常糟糕，涉及到字体缩放时又由于没有进行`Hint`，导致渲染出来的字体非常不美观。后续可能会抛弃`TrueType` 转向位图字体。 

### 内存管理

内存管理的设计来自于早期的Windows NT内核，借鉴了WRK项目。`ccoskrnl` 与Windows NT一样，使用了PFN数据库来管理所有的物理内存。对于动态内存管理则是通过页链表和块查表共同管理。此外`ccoskrnl` 也实现了Windows NT中的页目录自映射机制，这使得每次系统启动，页表的位置都是随机的，增强了操作系统的内存安全性。

### 图形输出

`ccoskrnl` 并没有实现显卡驱动，所有的图像输出都是依赖于CPU的`AVX`指令集来加快帧缓冲区的更新。尽管我打算实现多窗口管理，不过这一功能还处在构想阶段。

## TO-DO

- [ ] Bug fix: 为多窗口更新添加自旋锁
- [x] 为动态管理器添加内存泄露检测
- [x] 系统PTE管理
- [ ] PCIe 管理
- [ ] NVMe 驱动
- [ ] 使用位图字体而不是TrueType字体输出字符。
- [ ] 电源管理
- [ ] CPU功耗管理

## 需求

- QEMU，2 GiB内存或更高
- x86_64 CPU (Intel or AMD) 支持AVX 指令集

测试机：机械革命S2 Air(R5 4600H/16GB/512GB)

## 安装

项目构建和安装请参考 [ccoskrnl build](https://github.com/ccoskrnl/ccoskrnl/wiki/Installation)

## 联系

E-mail: 2010705797@qq.com

ChengCheng OS: https://github.com/ccoskrnl/ccoskrnl

## 相关阅读

- [**Intel® 64 and IA-32 architectures software developer's manuals**](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

- [**AMD64 Architecture Programmer’s Manual Volume 2: System Programming**](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/24593.pdf)

- [**The C Programming Language**](https://en.wikipedia.org/wiki/The_C_Programming_Language)

- [**OSDev Wiki**](https://wiki.osdev.org/)

- [**ACPI Specification**](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/Frontmatter/Overview/Overview.html)

- [**UEFI Specification**](https://uefi.org/specs/UEFI/2.10/01_Introduction.html)

## 许可证

No license.
