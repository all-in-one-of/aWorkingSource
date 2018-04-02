#!/usr/bin/env bash

source open_exr
make clean;make;

# ./EXRRGBAAutoCrop /mnt/public/home/xukai/factory/open_exr/output/test_nocrop.exr  /mnt/public/home/xukai/factory/open_exr/output/exr_data_convert7.exr

./EXRDataAutoCrop \
/mnt/public/home/xukai/factory/open_exr/output/beauty.1002.exr \
/mnt/public/home/xukai/factory/open_exr/output/output.011.exr


# ./EXRDataAutoCrop \
# /mnt/public/home/xukai/factory/open_exr/output/diffuse_albedo.1001.exr \
# /mnt/public/home/xukai/factory/open_exr/output/output.009.exr