#!/bin/bash

set +x
cp default.conf ~/.config/codeblocks/default.conf
codeblocks --build cross_injector.cbp
