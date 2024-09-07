# Cheng Cheng's Operating System Kernel

## 概述

ccos 是一个基于x86_64架构开发的64位操作系统，尽可能地提供现代设备的驱动(我不保证能实现出来:)。ccos 项目的创立和开发主要来自我个人对操作系统的兴趣。内核的大部分设计思路都来源于Windows NT内核。

***
![Sample](./conf/sample.png)
***

## 目录

- [Cheng Cheng's Operating System Kernel](#cheng-chengs-operating-system-kernel)
  - [概述](#概述)
  - [目录](#目录)
  - [安装](#安装)
  - [特性](#特性)
  - [进度](#进度)
  - [贡献](#贡献)
  - [联系](#联系)

## 安装

安装以及调试运行的教程已在Wiki给出。

## 特性

**UEFI引导**

UEFI，也称为统一可延伸固件接口，是一种公开可用的规范，它定义了操作系统和平台固件之间的软件接口。 UEFI 取代了原来存在于所有 IBM 电脑兼容个人电脑中的传统基本输入输出系统（BIOS）固件接口，大多数 UEFI 固件实现都提供对传统 BIOS 服务的支持。使用UEFI接口开发操作系统的加载器将会大大减少操作系统开发者的工作量。相比于传统引导的操作系统加载器开发，UEFI无疑是一种更好的选择。在UEFI期间，引导器主要负责收集当前设备的各种信息以及ACPI表传递给内核。

**APIC支持 (开发中)**

Intel 公司开发的高级可编程中断控制器(APIC)提供了以下功能：

- 处理大量中断，将每个中断路由到特定的 CPU 集合。
- 支持 CPU 间的通信，无需多个设备来共享单个中断行。

操作系统中断使用APIC统一管理，并且尽可能地提供对APIC硬件的支持。

**TrueType**

TrueType是由美国苹果公司和微软公司共同开发的一种电脑轮廓字体（曲线描边字）类型标准。操作系统使用思源黑体(Adobe Source Han Sans SC VF)，作为默认字体。操作系统解析TTF，并直接在屏幕上绘制出字形的轮廓(没有实现填充算法，小字号字体显示时空隙相对不明显，并不影响字体的辨识，故暂时并没有实现)。


**Windows NT内存管理器**

操作系统的内存管理来自Windows NT的内存管理器设计，这其中包括页目录自映射方案，分层的内存池管理，并使用Lookaside机制提供对更精细颗粒度的内存管理。

## 进度

- [ ] 自旋锁实现
- [ ] Application Processors 唤醒
- [ ] 键盘驱动
- [ ] HPET 驱动支持
- [ ] APIC 支持
- [ ] 优化内存管理器，实现Tag机制，提供对内存泄露的检测。


## 贡献

感谢 @Estrella 大佬提供的完整的数学库(`reflibs/libm.a`)，在开发过程中他为我提供了很多宝贵的建议和一些技术的讲解。

## 联系

如有疑问或反馈，请通过以下方式联系我：

- Email: 2010705797@qq.com
- Github: ccoskrnl
