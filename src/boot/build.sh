#!/usr/bin/bash

# [Components]
#   MdeModulePkg/Application/CcLoader/CcLoader.inf
#   MdeModulePkg/Application/HelloWorld/HelloWorld.inf

set -e
CCOSKRNL=${HOME}/src/ccoskrnl
BUILD_DIR=${HOME}/src/ccoskrnl/esp/EFI/ChengChengOS
EDK2_DIR=${HOME}/src/edk2
EDK2_CCLDR_DIR=${EDK2_DIR}/MdeModulePkg/Application/CcLoader

mkdir -pv ${EDK2_DIR}/MdeModulePkg/Application/CcLoader/
rm -rvf ${EDK2_DIR}/MdeModulePkg/Application/CcLoader/*
cp -rvf ${CCOSKRNL}/src/boot/* ${EDK2_DIR}/MdeModulePkg/Application/CcLoader

pushd ${EDK2_DIR}
source edksetup.sh BaseTools
build
popd
mkdir -pv ${BUILD_DIR}
echo ""
nasm -f bin -o ${BUILD_DIR}/ccldr ${EDK2_CCLDR_DIR}/ccldr.asm
nasm -f bin -o ${BUILD_DIR}/ap ${EDK2_CCLDR_DIR}/ap_startup.asm
echo "NASM	${BUILD_DIR}/ccldr"
echo "NASM	${BUILD_DIR}/ap"
echo ""
cp -vf ${HOME}/src/edk2/Build/MdeModule/DEBUG_GCC5/X64/CcLoader.efi ${BUILD_DIR}/../BOOT/BOOTX64.EFI
echo ""
echo "Compiled successfully"
echo ""
