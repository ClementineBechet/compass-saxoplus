
# CUDA default definitions
export CUDA_ROOT=/usr/local/cuda
export CUDA_INC_PATH=$CUDA_ROOT/include
export CUDA_LIB_PATH=$CUDA_ROOT/lib
export CUDA_LIB_PATH_64=$CUDA_ROOT/lib64

# CULA default definitions
export CULA_ROOT=/usr/local/cula
export CULA_INC_PATH=$CULA_ROOT/include
export CULA_LIB_PATH=$CULA_ROOT/lib
export CULA_LIB_PATH_64=$CULA_ROOT/lib64

# YOGA default definitions
export YOGA_DIR=$(pwd)
export YOGA_AO_DIR=$YOGA_DIR/yoga_ao
export YOGA_AO_TOP=$YOGA_AO_DIR

export LD_LIBRARY_PATH=$YOGA_DIR/libyoga:$YOGA_AO_DIR/libyoga_ao:$CUDA_LIB_PATH_64:$CUDA_LIB_PATH:$CULA_LIB_PATH_64:$CULA_LIB_PATH #:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/usr/local/cuda/include

alias yorick=~/yorick.git/relocate/bin/yorick