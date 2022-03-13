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
Debug compiler on wrong field macro \

Plugin Loading Manager abstraction
Proper memory layer \
Plugin allocator \
Error message : more info \
FFT processing available to the plugins \

No CLI plugin -> Load Plugin File \
Disable hot-reload button \
Panel toggle : compiler log and IR \
\
Optional: \
\
Bug : Exiting while hot reloading crashes \
If the struct layout has not changed, and if initial_state produces the same thing, then do not swap \
Draggable IR \
Time and frequency markers on the graphs \
Plugin browser \
Reset State button \
Font sizes system (load multiple font sizes or stretch uv) \
Beautiful UI \
Plugin chain, maybe graph \
\
Done : \
\
~~Moved the ui layer to its own dll on debug builds~~ _
~~Display custom compiler errors~~ \
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