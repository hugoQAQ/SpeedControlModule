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

