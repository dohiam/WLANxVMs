#!/bin/bash
## @file openssl/config.sh
## @brief set the source URL & file, and target build directory for opnssl
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="openssl"
VERSION="-1.1.1g"
TAR=".tar.gz"
DOWNLOAD_URL="https://www.openssl.org/source/openssl-1.1.1g.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
