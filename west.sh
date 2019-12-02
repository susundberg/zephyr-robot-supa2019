#!/bin/bash

set -e
set -x
export PATH="/home/pauli/.local/bin:$PATH"
source /stor/pauli/zephyrproject/zephyr/zephyr-env.sh
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr               
export ZEPHYR_SDK_INSTALL_DIR=/stor/pauli/zephyr-sdk/
west $@



