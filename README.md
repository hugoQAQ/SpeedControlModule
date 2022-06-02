# Installing dependencies

## 1. BIP
### 0. Installing dependencies for BIP
```bash
sudo apt-get install ant cmake g++ default-jre default-jdk
```
### 1. Getting BIP source code
```bash
git clone https://gricad-gitlab.univ-grenoble-alpes.fr/verimag/bip/compiler.git
```
### 2. Compiling source code
```bash
cd compiler/distribution
./single-archive-dist.sh
```
The compilation will then build an archive in the `./build/` directory with the name like `bip-full <version-name> .tar.gz` (where `<version-name>` is a version name automatically generated). How to use such an archive is described [here](http://www-verimag.imag.fr/New-BIP-tools.html?lang=en).

# Executing the BIP model

```bash
cd model
source compile_run_bip.sh -c learn Compound
./system -s
```

https://mst.mentor.com/download_public.html?token=m6dK2yE%2FQzUHRfmEG1nfXkpH2u6KZKhZeRmvQCvbgqqBBFQjtY%2BRt55W26lXlU7xJH1Lqr8D7xiu%0D%0ATKf7XTeCROTt3tt%2Bmcg2R2NStIi7GwxvXgW0WU8f9FcKC%2BL0AzrWlCFJUMsCV%2BuZuhhMH8nogTr9%0D%0Ao9fYwv7Dc5GF0E0cjLK89dfq56qutIlw2v4Jiqta
