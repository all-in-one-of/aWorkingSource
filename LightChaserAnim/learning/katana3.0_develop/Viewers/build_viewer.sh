
export BUILD_DIR="/home/xukai/Git/build_repo/katana3_viewers"
export SRC_DIR="/mnt/work/software/katana/katana3.0v1b2/plugins/Src/Viewers"
export DEST_DIR="/home/xukai/Git/git_repo/katana/resource/3.0"
export DEPS_DIR="/mnt/work/software/develop/dependences/open_exr"

if [ ! -d ${BUILD_DIR} ]; then
      mkdir ${BUILD_DIR}
fi

cd ${BUILD_DIR}

cmake ${SRC_DIR} \
        -DCMAKE_INSTALL_PREFIX="${DEST_DIR}" \
        -DGLEW_INCLUDE_DIR="/mnt/work/software/develop/dependences/glew-2.1.0/include" \
        -DGLEW_LIBRARIES="/mnt/work/software/develop/dependences/glew-2.1.0/lib64" \
        -DGLEW_LIBRARY="/mnt/work/software/develop/dependences/glew-2.1.0/lib64/libGLEW.so" \
        -DGLEW_LIBRARIES="/mnt/work/software/develop/dependences/glew-2.1.0/lib" \
        -DOPENEXR_ILMIMF_LIBRARY="/mnt/work/software/develop/dependences/open_exr/lib/libIlmImf.so" \
        -DOPENEXR_INCLUDE_DIR="/mnt/work/software/develop/dependences/open_exr/include"

