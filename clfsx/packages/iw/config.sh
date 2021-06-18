#!/bin/bash
## @file iw/config.sh
## @brief set the source URL & file, and target build directory for iw
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="wireless_tools"
VERSION=".29"
TAR=".tar.gz"
DOWNLOAD_URL="https://hewlettpackard.github.io/wireless-tools/wireless_tools.29.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
