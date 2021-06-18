#!/bin/bash
## @file wpa_supplicant/config.sh
## @brief set the source URL & file, and target build directory for the wpa supplicant
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

PACKAGE_NAME="wpa_supplicant"
VERSION="-2.9"
TAR=".tar.gz"
DOWNLOAD_URL="http://w1.fi/releases/wpa_supplicant-2.9.tar.gz"
SOURCE_FILE="$PACKAGE_NAME$VERSION$TAR"
PACKAGE_DIRECTORY="$PACKAGE_NAME$VERSION"
