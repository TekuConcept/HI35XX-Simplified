# HI35XX-Simplified
A simplified version of the HiSilicon SDK sample code

## Build Steps

```
mkdir Build && cd Build
cmake -DCMAKE_TOOLCHAIN_FILE=../ArmelToolchain.cmake ..
make -j4
```

## Supported Examples

- Raw Video Input
- Video Encoder
- Audio Recording
- Audio Playback
