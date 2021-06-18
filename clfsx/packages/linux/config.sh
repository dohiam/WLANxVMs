#!/bin/bash
## @file linux/config.sh
## @brief set the source URL & file, and target build directory for linux
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="linux"
VERSION="-4.14.180"
TAR=".tar.gz"
DOWNLOAD_URL="https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/linux-4.14.180.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
