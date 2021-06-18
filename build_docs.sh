#!/bin/bash

sudo apt-get install doxygen -y
doxygen doxygen.cfg
firefox doc/html/index.html
