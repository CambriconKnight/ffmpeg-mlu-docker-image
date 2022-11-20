# MLU Hwaccel Video to Video decoder

## Prerequisites

- Ubuntu 16.04 or more
- GCC (gcc and g++) ver. 5.4 or later
- Cambricon MLU Driver (v4.9.0 or later)
- Cambricon MLU SDK (v1.5.0 or later)

## Getting Started

### Link Dynamic Library

1. go into the dir *ff_mlu_video_hwdecoder* and mkdir *3rdparty/ffmpeg*

```bash
cd ff_mlu_video_hwdecoder
mkdir 3rdparty/ffmpeg
```

2. move or copy FFmpeg *include/* and *lib/* under the dir *3rdparty/ffmpeg*

3. export neuware and ffmpeg dynamic library file path in env LD_LIBRARY_PATH
   **if you did the step 1, the ffmpeg lib path should be <ff_mlu_video_hwdecoder PATH>/3rdparty/ffmpeg/lib** 

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/neuware/lib64:<your ffmpeg lib path>
```

### Build

```bash
mkdir build
cd build
cmake ..
make -j
```

### Run

The hwdecoder need 4 params, <input file> <device_id> <dump flag> <thread_num>

```bash
./hwdecoder <input file> <device_id> <dump flag> <thread_num>
```
