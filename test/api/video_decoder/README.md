# MLU Video to Video decoder

## Prerequisites

- Ubuntu 16.04 or more
- GCC (gcc and g++) ver. 5.4 or later
- Cambricon MLU Driver (v1.3.0 or later)
- Cambricon MLU SDK (v1.3.0 or later)

## Getting Started

### Link Dynamic Library
1. go into the dir *ffmpeg-mlu_video_decoder* and mkdir *3rdparty/ffmpeg*
```bash
cd ffmpeg-mlu_video_decoder
mkdir 3rdparty/ffmpeg
```
2. move or copy FFmpeg *include/* and *lib/* under the dir *3rdparty/ffmpeg*
```bash
  mv ffmpeg_dir/ .../3rdparty/ffmpeg
```
3. export neuware and ffmpeg dynamic library file path in env LD_LIBRARY_PATH
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/neuware/lib64:<your ffmpeg lib path>
```

### Build
```bash
mkdir build
cd build
cmake .. && make -j
```

### Run
```bash
./decoder <input file> <device_id> <dump flag> <thread_num>
```
