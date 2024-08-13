# CCOSKRNL

## 设计思路

[CcLoader](./docs/CcLoader.md)

## 构建测试

在构建该项目之前，请确保您已经安装好了下列软件: 
```
gcc
gdb
python
qemu-system-x86
qemu-system-x86-firmware
qemu-desktop
ovmf
iasl
nasm
python-distutils-extra
```

### 搭建EDKII开发环境

首先克隆edk2仓库：
```bash
git clone https://github.com/tianocore/edk2.git
```
接下来需要编译基本工具
```bash
cd edk2
git submodule update --init
make -C BaseTools
```
设置编译需要的环境变量，它会在Conf目录生成一些配置文件。
```bash
export EDK_TOOLS_PATH=$HOME/src/edk2/BaseTools
source edksetup.sh BaseTools
```
编辑`Conf/target.txt`文件，修改方式如下：
```
ACTIVE_PLATFORM       = MdeModulePkg/MdeModulePkg.dsc
# 您也可以使用RELEASE
TARGET                = DEBUG
# 我们的系统是X64架构，所以这里也要手动修改
TARGET_ARCH           = X64
# 使用GCC工具链
TOOL_CHAIN_TAG        = GCC5
```
在完成上面的修改之后，键入下面这条命令进行构建测试，MdeModulePkg的所有模块都会被构建。
```bash
build
```

### 构建CcLoader

在确保上面所有步骤都正常进行后，接下来就需要使用EDKII来构建我们的加载器了。

```bash
# 进入您自己的edk2项目文件夹，$EDK2是我的edk2项目路径，您需要替换为您自己的路径
cd $EDK2
mkdir -pv MdeModulePkg/Application/CcLoader

# 将`ccoskrnl/src/boot`的所有文件拷贝到CcLoader,$CCOSKRNL变量是我的项目路径
cp -vrf $CCOSKRNL/src/boot/* ./MdeModulePkg/Application/CcLoader/
```

修改`MdeModulePkg/MdeModulePkg.dsc`文件，在`[Components]`描述中添加我们自己的项目。
```
[Components]
  MdeModulePkg/Application/CcLoader/CcLoader.inf
```

回到edk2主目录，重新构建，将构建好的固件复制到`$CCOSKRNL/esp/EFI/BOOT/BOOTX64.EFI`

```bash
export EDK_TOOLS_PATH=$HOME/src/edk2/BaseTools
source edksetup.sh BaseTools
build
cp -vf $EDK2/Build/MdeModule/DEBUG_GCC5/X64/CcLoader.efi $CCOSKRNL/esp/EFI/BOOT/BOOTX64.EFI
```

编译加载器，同样拷贝到`$CCOSKRNL/esp/ccldr`
```bash
nasm -f bin -o $CCOSKRNL/esp/ccldr $EDK2/MdeModulePkg/Application/CcLoader/ccldr.asm
```

在完成上面的工作之后，加载器已经构建完毕，接下来就可以构建内核了！

### 构建ccoskrnl

确保您已经构建好了x86_64架构的交叉编译器，并将它们存放在`$HOME/opt/cross/bin/`目录下。

构建交叉编译器请阅读 OSDev-GCC Cross-Compiler - https://wiki.osdev.org/GCC_Cross-Compiler

进入 `$CCOSKRNL/src`

```bash
cd $CCOSKRNL/src
# make -j $(nproc) 可以加快编译速度
make
# make clean 可以清理构建的文件
```

### 调试

对于qemu模拟器：

进入 `$CCOSKRNL/src`

```bash
cd $CCOSKRNL/src
# make -j $(nproc) 可以加快编译速度
make run
```

打开另一个终端，进入`$CCOSKRNL/src`目录，输入下面这条命令启动gdb调试(注意，由于开启了kvm，所以需要使用hbreak)
```bash
gdb --command=../conf/gdb_start
```

### 在物理机上运行

**十分不推荐您在物理机上运行该项目！！！**

**在物理机上运行具有非常大的风险，由于主办模型是以我的笔记本为基础开发(她比较旧)，我不保证我已经正确处理好所有的其他的主板结构，如果您需要在您的机器上运行，请确保您已经实现了对于的驱动**


您需要一个U盘，清空U盘的所有分区(注意备份您的数据)，新建一块EFI分区(该分区应该为您U盘上的第一个分区)，格式化为Fat32。将`$CCOSKRNL/esp/`的所有文件都复制到刚新建好的分区，重启您的电脑，打开BIOS引导界面，此时您应该能在引导菜单中看到您的U盘被标识为了UEFI引导，按下危险的回车键，内核就运行起来了。

