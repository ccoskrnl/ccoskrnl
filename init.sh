#!/usr/bin/bash

set -e

cont=false
INPUT=n

CCOSKRNL_RELATIVE_PATH="/src/ccoskrnl"
CCOSKRNL_PATH=$HOME$CCOSKRNL_RELATIVE_PATH

EDK2_RELATIVE_PATH="/src/edk2"
EDK2_PATH=$HOME$EDK2_RELATIVE_PATH

Mde_Module_Pkg_dsc="/MdeModulePkg/MdeModulePkg.dsc"
Mde_Module_Pkg_dsc_file=$EDK2_PATH$Mde_Module_Pkg_dsc
CcLoader_Component="  MdeModulePkg/Application/CcLoader/CcLoader.inf"

echo "Assuming Your EDK2 folder at \${HOME}/src/edk2 and Your ccosknrl folder at \${HOME}/src/ccoskrnl"
echo "If not, please modify the content of the relevant variables"

read -p "Do you want to contine? [y]/[n] : " INPUT

if [ "$INPUT" = "y" ]; then
    cont=true;
fi

if [ "$cont" = true ]; then

    sed -i '/\[Components\]/a\'"$CcLoader_Component" "$Mde_Module_Pkg_dsc_file"

    mkdir -pv ${EDK2_PATH}/MdeModulePkg/Application/CcLoader

    mkdir -pv ${CCOSKRNL_PATH}/build/disk
    qemu-img create ${CCOSKRNL_PATH}/build/disk/nvmen0.img 16M

    echo "Mission Completed!"

fi


