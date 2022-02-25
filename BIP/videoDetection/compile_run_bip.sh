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
bipc.sh -I . -p $strPack -d "$strComp" --gencpp-output-dir output --gencpp-ld-l rt  --gencpp-cc-I $PWD/ext-cpp  --gencpp-follow-used-packages 
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




