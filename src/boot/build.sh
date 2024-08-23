#!/usr/bin/bash
set -e
CCOSKRNL=${HOME}/projects/ccoskrnl
BUILD_DIR=${HOME}/projects/ccoskrnl/esp
EDK2_DIR=${HOME}/src/edk2
EDK2_CCLDR_DIR=${EDK2_DIR}/MdeModulePkg/Application/CcLoader

rm -rvf ${EDK2_DIR}/MdeModulePkg/Application/CcLoader/*
cp -rvf ${CCOSKRNL}/src/boot/* ${EDK2_DIR}/MdeModulePkg/Application/CcLoader

pushd ${EDK2_DIR}
source edksetup.sh BaseTools
build
popd
mkdir -pv ${BUILD_DIR}
echo ""
nasm -f bin -o ${BUILD_DIR}/ccldr ${EDK2_CCLDR_DIR}/ccldr.asm
echo "NASMing	${BUILD_DIR}/ccldr"
echo ""
cp -vf ${HOME}/src/edk2/Build/MdeModule/DEBUG_GCC5/X64/CcLoader.efi ${BUILD_DIR}/EFI/BOOT/BOOTX64.EFI
echo ""
echo "Compiled successfully"
echo ""
