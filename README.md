# DSP Bench

Tool for prototyping DSP code.

### Building
For now there are two heavy dependencies : clang/llvm and Intel Performance Primitives. Sooooo, please use the release for now. 

### usage 
```
DSP_bench.exe audio_file.wav plugin_code.cpp 
```

### TODO

Easy : \
\
No plugin parameter -> Load Plugin File \
Display Compiler errors \
FFT processing available to the plugins \
plugin optional hot swap \
Plugin allocator and tracking -> automatic collection \
Draggable IR \
\
Hard : \
\
Options Blackman, Hamming, etc \
Command queue to audio thread : play, stop, parameter change (? full parameter state ?) \
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Review Synchronization code \
Proper memory layer \
Beautiful UI \
Plugin chain, maybe graph \
\
Done : \
\
~~Linear vs Logarithmic slider customization~~ \
~~logarithmic normalization~~ \
~~wav swap~~ \
~~file browsing~~
~~Zoomable IR~~ \
~~don't call opengl_render_fft/ir when plugin isn't live~~ \
~~Loading screen~~ \
~~Self contained slider widget~~ \
~~Parameter ctrl precision (hide mouse)~~ \
~~cli : optional wav~~ \
~~Display min,max,value~~ \
~~window + zero padding fft~~\
~~Background thread compilation~~ \
~~Background wav loading~~ \
~~Play/Stop/Loop Wav, enable/disable plugin (header ?)~~ 