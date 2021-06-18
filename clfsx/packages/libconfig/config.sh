#!/bin/bash
## @file libconfig/config.sh
## @brief set the source URL & file, and target build directory for libconfig
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="libconfig"
VERSION="-1.7.2"
TAR=".tar.gz"
DOWNLOAD_URL="https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.2.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
