#!/bin/bash
HFS=/mnt/usr/hfs16.0.557
cd ${HFS}
source ./houdini_setup
mkdir -p $HOME/HDK
cp $HFS/toolkit/samples/standalone/geoisosurface.C .
hcustom -s geoisosurface.C