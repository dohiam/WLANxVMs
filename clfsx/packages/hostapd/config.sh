#!/bin/bash
## @file hostapd/config.sh
## @brief set the source URL & file, and target build directory for hostapd
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="hostapd"
VERSION="-2.9"
TAR=".tar.gz"
DOWNLOAD_URL="https://w1.fi/releases/hostapd-2.9.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"

PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
