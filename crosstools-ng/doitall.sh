#!/bin/bash

set +x
sudo apt-get update
sudo apt-get install gcc flex texinfo help2man gawk build-essential libtool-bin libncurses-dev -y
wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.24.0.tar.bz2
tar -xf crosstool-ng-1.24.0.tar.bz2
cd crosstool-ng-1.24.0
./configure --enable-local
make
cp ../x86_64.config ./.config
./ct-ng oldconfig
./ct-ng build.12
mv ~/x-tools ../../
