# DSP Bench

Tool for prototyping DSP code.

### Building
For now there are two heavy dependencies : clang/llvm and Intel Performance Primitives. Sooooo, please use the release for now. 

### usage 
```
DSP_bench.exe audio_file.wav plugin_code.cpp 
```

### TODO

Loading screen \
Compiler errors display \
Self contained slider widget \
Parameter ctrl precision (hide mouse)
Parameter skew factor \
FFT processing available to the plugins \
~~cli : optional wav~~ \
~~Display min,max,value~~ \
~~window + zero padding fft~~\
~~Background thread compilation~~ \
~~Background wav loading~~ \
~~Play/Stop/Loop Wav, enable/disable plugin (header ?)~~ \
Command queue to audio thread : play, stop, parameter change (? full parameter state ?)
Plugin allocator and tracking -> automatic collection \
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Review Synchronization code \
Options Blackman, Hamming, etc \
Zoomable IR \
Proper memory layer \
Plugin Hot-Swap on save \
Beautiful UI \
Audio and Source file browsing \
Plugin chain, maybe graph 
