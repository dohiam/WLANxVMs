#!/bin/bash

set +x
cd crosstool-ng-1.24.0
cp ../x86_64.config ./.config
./ct-ng oldconfig
./ct-ng build.12
mv ~/x-tools ../../
