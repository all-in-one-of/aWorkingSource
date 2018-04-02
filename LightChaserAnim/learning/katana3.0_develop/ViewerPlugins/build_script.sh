
export SRC_DIR=$(dirname "$(readlink -e "$BASH_SOURCE")")
export BUILD_DIR="/home/xukai/Git/build_repo/katana3_viewer_plugins"
export DEST_DIR="/home/xukai/Git/git_repo/katana/resource/3.0"

if [ -d ${BUILD_DIR} ]; then
        rm -rf ${BUILD_DIR}
fi
mkdir ${BUILD_DIR}

cd ${BUILD_DIR}

cmake ${SRC_DIR} \
        -DCMAKE_INSTALL_PREFIX="${DEST_DIR}" \
        -DKatana_SDK_ROOT="/mnt/work/software/katana/katana3.0v1b2" \
        -DGLEW_INCLUDE_DIR="/mnt/work/software/develop/dependences/glew-2.1.0/include" \
        -DGLEW_LIBRARIES="/mnt/work/software/develop/dependences/glew-2.1.0/lib64" \
        -DGLEW_LIBRARY="/mnt/work/software/develop/dependences/glew-2.1.0/lib64/libGLEW.so" \
        -DGLEW_LIBRARIES="/mnt/work/software/develop/dependences/glew-2.1.0/lib" \
        -DOPENEXR_ILMIMF_LIBRARY="/mnt/work/software/develop/dependences/open_exr/lib/libIlmImf.so" \
        -DOPENEXR_INCLUDE_DIR="/mnt/work/software/develop/dependences/open_exr/include"