

BUILD_DIR=/home/xukai/Git/build_repo/LightViewerModifier2.5
INSTALL_DIR=/home/xukai/Git/git_repo/katana/resource/2.5
KATANA_HOME=/mnt/work/software/katana/katana2.5v5
SRC_DIR=$(dirname "$(readlink -e "$BASH_SOURCE")")
echo ${SRC_DIR}
if [ ! -d ${BUILD_DIR} ]; then
      mkdir ${BUILD_DIR}
fi
cd ${BUILD_DIR}
cmake ${SRC_DIR} \
      -G "Unix Makefiles" \
      -DKATANA_HOME=${KATANA_HOME} \
      -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \

make install -j16