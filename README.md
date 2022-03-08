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
Plugin hot swap \
Proper memory layer \
No plugin parameter -> Load Plugin File \
FFT processing available to the plugins \
Plugin allocator and tracking -> automatic collection \
Draggable IR \
Time and frequency markers on the graphs \
Release assertions \
\
Hard : \
\
Plugin browser \
Command queue to audio thread : play, stop, parameter change (? full parameter state ?) \
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Beautiful UI \
Plugin chain, maybe graph \
\
Done : \
\
~~Single executable Release build~~ \
~~Display Compiler errors~~ \
~~Plugin cold swap~~
~~Linear vs Logarithmic slider customization~~ \
~~logarithmic normalization~~ \
~~Wav swap~~ \
~~open file prompt~~
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