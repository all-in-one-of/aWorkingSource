#!/usr/bin/env bash

export SRC_DIR=$(dirname "$(readlink -e "$BASH_SOURCE")")
export BUILD_DIR="/home/xukai/Git/build_repo/Ops_Sphere_Maker"
export DEST_DIR="/home/xukai/Git/git_repo/katana/resource/3.0"

if [ ! -d ${BUILD_DIR} ]; then
      mkdir ${BUILD_DIR}
fi

cd ${BUILD_DIR}

cmake ${SRC_DIR} \
        -DCMAKE_INSTALL_PREFIX="${DEST_DIR}" \
        -DKatana_SDK_ROOT="/mnt/work/software/katana/katana3.0v1b2" \