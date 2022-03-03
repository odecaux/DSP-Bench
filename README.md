# DSP Bench

Tool for prototyping DSP code.

### Building
For now there are two heavy dependencies : clang/llvm and Intel Performance Primitives. Sooooo, please use the release for now. 

### usage 
```
DSP_bench.exe audio_file.wav plugin_code.cpp 
```

### TODO

Background wav loading \
Loading screen \
Compiler errors display \
Command queue to audio thread : play, stop, parameter change (? full parameter state ?)
~~cli : optional wav~~ \
~~Display min,max,value~~ \
~~window + zero padding fft~~\
~~Background thread compilation~~ \
~~Play/Stop/Loop Wav, enable/disable plugin (header ?)~~ \
Plugin allocator and tracking -> automatic collection \
Parameter skew factor \
Parameter ctrl precision (hide mouse)
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Review Synchronization code \
FFT processing available to the plugins \
Options Blackman, Hamming, etc \
Zoomable IR \
Proper memory layer \
Plugin Hot-Swap on save \
Beautiful UI \
Audio and Source file browsing \
Plugin chain, maybe graph 
