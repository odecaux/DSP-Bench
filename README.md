# DSP Bench

Tool for prototyping DSP code.

### Building
For now there are two heavy dependencies : clang/llvm and Intel Performance Primitives. Sooooo, please use the release for now. 

### TODO

Important : \
Audio device unplug/change \
Input audio device as source \
Disable hot-reload button \
Offline rendering \
Bug : wrong field macro \
Bug : Exiting while hot reloading crashes \
If the struct layout has not changed, and if initial_state produces the same thing, then do not swap \
Draggable IR \
Time and frequency markers on the graphs \
Plugin browser \
Reset State button \
Beautiful UI \
Plugin chain, maybe graph \
\
Done : \
\
Scale font \
FFT processing available to the plugins \
Plugin can allocate memory \
Panel toggle : compiler log and IR \
Rewrote the error reporting system \
Plugin allocator \
cli : optional plugin source \
Plugin loading abstraction \
Moved the UI layer to its own dll on debug builds \
Display custom compiler errors \
Plugin hot-swap on save \
Single-executable Release build \
Display compiler errors \
Plugin cold-swap \
Linear vs Logarithmic slider customization \
Audio file swap \
Open File prompt \
Zoomable IR \
Loading screen \
Self contained slider widget \
Left-Ctrl -> increase parameter precision  \
Parameter tweaking hides mouse  \
cli : optional wav \
Min, max and value labels on parameters \
FFT windowing & zero padding \
Background thread compilation \
Background wav loading \
Play, Pause, Enable\Disable buttons\
Footer and header
