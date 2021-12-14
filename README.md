# Installing dependencies

## 1. OpenCV

``` bash
sudo apt-get install libopencv-dev python3-opencv 
```

## 2. LibTorch

```bash
wget https://download.pytorch.org/libtorch/lts/1.8/cpu/libtorch-cxx11-abi-shared-with-deps-1.8.2%2Bcpu.zip 
unzip libtorch-cxx11-abi-shared-with-deps-1.8.2%2Bcpu.zip
```
## 3. BIP
0. Installing required dependencies for BIP compiler and BIP engines
```bash
sudo apt-get install ant cmake g++ default-jre default-jdk
```
1. Getting BIP source code
```bash
git clone https://gricad-gitlab.univ-grenoble-alpes.fr/verimag/bip/compiler.git
```
2. Compiling source code
```bash
cd compiler/distribution
./single-archive-dist.sh
```
The compilation will then build an archive in the `./build/` directory with the name like `bip-full <version-name> .tar.gz` (where `<version-name>` is a version name automatically generated). How to use such an archive is described [here](http://www-verimag.imag.fr/New-BIP-tools.html?lang=en)




# Building C++ executable for image classification

```bash
cd learn
source setup.sh
cp limit ../../model/ext-cpp
```



# Executing the BIP model

```bash
cd model
source compile_run_bip.sh -c learn Compound
./system -s
```

