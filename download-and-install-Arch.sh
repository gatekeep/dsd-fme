#! /bin/bash
clear
echo DSD-FME Digital Speech Decoder - Florida Man Edition Auto Installer For Arch Linux
echo This will install the required packages, clone, build, and install DSD-FME only.
echo This has been tested on Arch 2023.08.01 and Manjaro XFCE 22.1.3 Minimal.

read -p "Press enter to continue"

sudo pacman -Syu #always run a full update first, partial upgrades aren't supported in Arch -- including downloading dependencies, that may require an updated dependency, and breaks same dependency on another package, a.k.a, dependency hell
sudo pacman -S libpulse cmake ncurses lapack perl fftw rtl-sdr codec2 base-devel libsndfile git wget rtl-sdr

echo Installing itpp 4.3.1 from Arch Strike --Tested on Arch 2023.08.01 and Manjaro XFCE 22.1.3 Minimal
wget https://mirror.archstrike.org/x86_64/archstrike/itpp-4.3.1-3-x86_64.pkg.tar.xz
sudo pacman -U itpp-4.3.1-3-x86_64.pkg.tar.xz
sudo rm -R itpp-4.3.1-3-x86_64.pkg.tar.xz

git clone https://github.com/lwvmobile/mbelib
cd mbelib
git checkout ambe_tones
mkdir build
cd build
cmake ..
make -j `nproc`
sudo make install
sudo ldconfig
#sudo cp /usr/local/lib/libmbe.a /usr/lib/libmbe.a
sudo cp /usr/local/lib/libmbe.so /usr/lib/libmbe.so #this is the only one we need to copy 
#sudo cp /usr/local/lib/libmbe.so.1 /usr/lib/libmbe.so.1
#sudo cp /usr/local/lib/libmbe.so.1.3 /usr/lib/libmbe.so.1.3
cd ..
cd ..

git clone https://github.com/lwvmobile/dsd-fme
### For branch v2.1b - git clone --branch v2.1b https://github.com/lwvmobile/dsd-fme
cd dsd-fme
git checkout v2.1b
sudo cp tone8.wav /usr/share/
sudo cp tone24.wav /usr/share/
sudo cp tone48.wav /usr/share/
sudo chmod 777 /usr/share/tone8.wav
sudo chmod 777 /usr/share/tone24.wav
sudo chmod 777 /usr/share/tone48.wav
mkdir build
cd build
cmake ..
make -j `nproc`
sudo make install
sudo ldconfig
sudo rm -R dsd-fme

echo Any issues, Please report to either
echo https://github.com/lwvmobile/dsd-fme/issues
echo https://forums.radioreference.com/threads/dsd-fme.438137/
