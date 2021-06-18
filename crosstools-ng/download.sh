#!/bin/bash

set +x
wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.24.0.tar.bz2
tar -xf crosstool-ng-1.24.0.tar.bz2
cd crosstool-ng-1.24.0
./configure --enable-local
make
