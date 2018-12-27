

ALEMBIC_DIR=/mnt/work/software/develop/dependences/Alembic-1.7.0
HDF5_DIR=/mnt/work/software/develop/dependences/hdf5-1.8.18

export PATH=${ALEMBIC_DIR}/bin:${PATH}
export PATH=${HDF5_DIR}/bin:${PATH}

export LD_LIBRARY_PATH=${ALEMBIC_DIR}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${HDF5_DIR}/lib:${LD_LIBRARY_PATH}