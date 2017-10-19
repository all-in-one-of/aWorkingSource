
ARNOLD_PATH=/mnt/work/software/arnold/arnold-4.2.10.0
ALEMBIC_HOME=/mnt/proj/software/develop/usd/dependences/alembic-1.5.5
HDF5_HOME=/mnt/proj/software/develop/usd/dependences/HDF5-1.8.9
ILMBASE_HOME=/mnt/proj/software/develop/usd/dependences/IlmBase-2.2.0
BOOST_HOME=/mnt/proj/software/develop/usd/dependences/boost_155

export LD_LIBRARY_PATH=$ARNOLD_PATH/bin:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ALEMBIC_HOME/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HDF5_HOME/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ILMBASE_HOME/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$BOOST_HOME/lib:$LD_LIBRARY_PATH