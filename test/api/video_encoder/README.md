# MLU Video to Video encoder

## Prerequisites

- Ubuntu 16.04 or more
- GCC (gcc and g++) ver. 5.4 or later
- Cambricon MLU Driver (v1.3.0 or later)
- Cambricon MLU SDK (v1.3.0 or later)

## Getting Started

### Make env

**make 3rdparty**

make *3rdparty* library directory like as follows:
```
ff_mlu_encoder
    └── 3rdparty
        └── ffmpeg
            ├── include
            └── lib
```

set env:

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NEUWARE_HOME}/lib64:<your projs>/3rdparty/ffmpeg/lib
```

### Build

```bash
mkdir build
cd build
cmake ..
make -j
```

### Run

The ff_mlu_encoder params:

```bash
Usage:./mlu_encoder input.txt
```
