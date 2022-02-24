#!/bin/bash

NPROC=$(nproc)

if [[ ( -n "$1" ) && ( "$1" == '-'* ) ]] 
then
sss=1
else
echo "Misses arg1: specify what to do!"
exit 1
fi



if [ -n "$2" ] 
then
strPack="$2"
else
#echo "Misses arg2: package directory! Default value: Package"
strPack="Package"
fi



if [ -n "$3" ] 
then
strComp="$3()"
else
#echo "Misses arg3: root instance name! Default value: Compound()"
strComp="Compound()"
fi


if [[ "$1" == *['c']* ]]
then
echo "Compiling ..."
rm -fr output/
mkdir -p output
ls
echo "=========================================="
echo "==============   BIPCompile ===================="
echo "=========================================="
bipc.sh -I . -p $strPack -d "$strComp" --gencpp-output-dir output --gencpp-ld-l rt  --gencpp-cc-I $PWD/ext-cpp \
    --gencpp-cc-I $XL_VIP_HOME/lib/lib_remote --gencpp-cc-I $UVMC_HOME/src/connect/rawc/tlm_lite  \
    --gencpp-ld-L $XL_VIP_HOME/lib/lib_remote/$XL_BUILD_PLATFORM  --gencpp-ld-L $UVMC_HOME/lib/rawc/client/$XL_BUILD_PLATFORM \
    --gencpp-ld-l uvmc_tlm_fabric  --gencpp-ld-l xl_vip_open_kit_stubs  --gencpp-ld-l xl_vip_tlm_xactors --gencpp-ld-l xl_vip_open_kit \
    --gencpp-follow-used-packages --gencpp-cc-extra-opt -DXL_TLM_REMOTE_CLIENT \
    --gencpp-ld-l opencv_imgcodecs --gencpp-ld-l opencv_imgproc --gencpp-ld-l opencv_core
mkdir -p output/build
cd output/build
pwd
echo "=========================================="
echo "==============    CMAKE     ====================="
echo "=========================================="
cmake ..

echo "=========================================="
echo "==============    CodeGen   ===================="
echo "=========================================="
make -j$NPROC
pwd
rm -f ../../system
cp  ./system ../..
cd ../..
fi




