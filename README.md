# DSP Bench

Tool for prototyping DSP code.

### Building
For now there are two heavy dependencies : clang/llvm and Intel Performance Primitives. Sooooo, please use the release for now. 

### usage 
```
DSP_bench.exe audio_file.wav plugin_code.cpp 
```

### TODO

Core: \
\
Disable hot-reload button \
If the struct layout has not changed, and if initial_state produces the same thing, then do not swap \
Plugin Loading Manager abstraction
Proper memory layer \
No CLI plugin -> Load Plugin File (implies 3 states : was_failed, was_in_use, was_not_loaded) \
FFT processing available to the plugins \
Plugin allocator \
Display custom compiler errors \
\
Optional: \
\
Draggable IR \
Time and frequency markers on the graphs \
Plugin browser \
Command queue to audio thread : play, stop, parameter change (? full parameter state ?) \
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Beautiful UI \
Plugin chain, maybe graph \
\
Done : \
\
~~Plugin reload on save~~ \
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