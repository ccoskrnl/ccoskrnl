# CcLoader

## 设计思路

ccoskrnl使用UEFI引导内核。

当UEFI应用程序调用`ExitBootServices()`后，Intel 64 CPU（x86-64架构）将处于以下状态：

- **单处理器模式**：CPU运行在单处理器环境中，即使是多核心处理器也只有一个核心被激活。
- **长模式**：CPU处于64位的长模式，这意味着它可以访问64位的指令集和寻址能力。
- **分页启用**：分页机制被启用，所有页面都被标识映射，这意味着虚拟地址直接映射到相同的物理地址。
- **平坦模式的段选择器**：段选择器（如CS、DS等）被设置为平坦模式，这意味着段的基址为0，段的限长为最大值，从而允许访问整个64位的线性地址空间。

在`ExitBootServices()`调用之后，UEFI不再提供引导服务，操作系统加载器需要接管所有硬件资源的管理。这包括初始化硬件设备、设置中断处理、配置内存管理等。操作系统的内核需要根据这个状态来继续启动过程，建立自己的环境并开始执行¹。

## Q&A

**为什么要先构建一个临时的页表再把控制权移交给内核?**

临时页表需要将物理内存中预留的内核空间内存映射到虚拟地址空间的高地址部分(以0xfffff00000000000为基地址)。加载器程序默认当前机器的物理内存的总大小至少为 128 MBytes，内核空间的大小一般设置为 **总空闲物理内存的大小的四分之一**。分页机制从`UefiMain()`函数开始之前就已经启用，假如不使用临时页表机制而直接将控制权移交给内核，那么在执行内核镜像的代码期间将很难再将自身的镜像部分映射到虚拟地址空间的高地址部分。所以临时页表至少要将内核镜像提前映射到虚拟地址空间的高地址部分，并预留一部分空间供内核使用。UEFI程序为`ccldr`程序预留了一部分内存空间供`ccldr`来存储临时页表(这部分的内存的大小根据内核空间的大小调整，它的计算公式为 $ CcldrSpaceSize = (KernelSpaceSize / 4096) * 8 + 1 MByte$ )。

## References

### UEFI

1. **内存是怎么映射到物理地址空间的？内存是连续分布的吗？** https://zhuanlan.zhihu.com/p/66288943?utm_id=0

2. **UEFI启动流程概览** https://zhuanlan.zhihu.com/p/483888207

3. **UEFI Specification** https://uefi.org/specs/UEFI/2.10/01_Introduction.html

4. **ACPI Software Programming Model** https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html

5. **ACPI Overview** https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/Frontmatter/Overview/Overview.html