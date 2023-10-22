#!/bin/bash

set -x

SOURCE_DIR=`pwd`
BUILD_DIR="./build"
BINARY_DIR="./bin"


# 外部构建
mkdir -p ${BUILD_DIR} \
&& mkdir -p ${BINARY_DIR} \
&& cd ${BUILD_DIR} \
&& cmake ${SOURCE_DIR} \
&& make
