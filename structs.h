/* date = November 29th 2021 9:48 am */

#ifndef STRUCTS_H
#define STRUCTS_H

typedef void*(*buffer_allocator_t)(unsigned int);

typedef void(*audio_callback_t)(void*, void*, float**, unsigned int, unsigned int, float);
typedef void(*default_parameters_t)(void*);
typedef void(*initialize_state_t)(void*, void*, unsigned int, float, buffer_allocator_t);

internal void* malloc_allocator(u32 byte_size)
{
    return malloc(byte_size);
}

enum Plugin_Parameter_Type{
    Int,
    Float,
    Enum
};

typedef struct{
    i32 min;
    i32 max;
} Parameter_Int;

typedef struct {
    real32 min;
    real32 max;
} Parameter_Float;

typedef struct{
    i32 value;
    String name;
} Parameter_Enum_Entry;


typedef struct {
    Parameter_Enum_Entry *entries;
    u32 num_entries;
} Parameter_Enum;

typedef struct {
    String name;
    u32 offset;
    
    Plugin_Parameter_Type type;
    union {
        Parameter_Int int_param;
        Parameter_Float float_param;
        Parameter_Enum enum_param;
    };
    
} Plugin_Descriptor_Parameter;

typedef struct {
    //String name;
    //le nom c'est le filename ?
    struct { 
        u32 size;
        u32 alignment;
    } parameters_struct;
    
    struct { 
        u32 size;
        u32 alignment;
    } state_struct;
    
    Plugin_Descriptor_Parameter *parameters;
    u32 num_parameters;
} Plugin_Descriptor;

typedef struct {
    union{
        int int_value;
        float float_value;
        int enum_value;
    };
} Plugin_Parameter_Value;


//TODO pour l'instant il y a un bug : si l'ui arrive à emettre 4096 blocs pendant que le thread audio fait 1 copie
typedef struct{
    Plugin_Parameter_Value *buffer;
    volatile Plugin_Parameter_Value **head;
    u32 writer_idx;
    u32 buffer_size;
    u32 num_fields_by_plugin;
} Plugin_Parameters_Ring_Buffer;


typedef struct {
    bool worked;
    
    void* llvm_jit_engine;
    audio_callback_t audio_callback_f;
    default_parameters_t default_parameters_f;
    initialize_state_t initialize_state_f;
    Plugin_Descriptor descriptor;
} Plugin_Handle;

typedef Plugin_Handle(*try_compile_t)(const char*, const void*);

typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

typedef struct 
{
    volatile i8 file_is_valid;
    
    real32** audio_file_buffer;
    u64 audio_file_length;
    u64 audio_file_read_cursor;
    u64 audio_file_num_channels;
    
    volatile i8 plugin_is_valid;
    audio_callback_t audio_callback_f;
    
    char* plugin_parameters_holder;
    char* plugin_state_holder;
    
    //TODO il se passe quoi si le ui thread s'arrête et que l'audio thread continue ?
    Plugin_Descriptor* descriptor; 
    Plugin_Parameter_Value* parameter_values_audio_side;
    Plugin_Parameters_Ring_Buffer* ring;
} Audio_Context;


typedef struct
{
    u64 frame_count;
    
    real32 delta_time;
    real32 time;
    
    bool mouse_down;
    bool mouse_clicked;
    bool mouse_released;
    
    bool right_mouse_down;
    bool mouse_double_clicked;
    bool right_mouse_clicked;
    
    bool delete_pressed;
    
    Vec2 mouse_position;
    Vec2 mouse_pos_prev;
    Vec2 mouse_delta;
    
    real32 mouse_double_click_time;
    real32 mouse_down_time;
    real32 right_mouse_down_time;
    real32 mouse_clicked_time;
} IO;


typedef u32 Color;

enum Colors : u32 {
    Color_Front = 0xffffffff,
    Color_Back = 0x00000000
};


typedef struct{
    i64 selected_parameter_idx;
} UI_State;


//TODO types 
typedef struct {
    real32 X0, Y0, X1, Y1;     // Glyph corners
    real32 U0, V0, U1, V1;     // Texture coordinates
    i32 advance_x;
    
    u32 codepoint; 
} Glyph;



typedef struct{
    float font_size; 
    
    i32 *codepoint_to_idx;  
    real32 *codepoint_to_advancex; 
    u32 highest_codepoint;
    
    Glyph *glyphs;
    u32 glyph_count;
    
    Vec2 white_rect_pos;
    //TODO fallback
    
    real32 ascent;
    real32 descent;
    
    u32* atlas_pixels;
    Vec2 atlas_texture_dim;
    u32 atlas_texture_id;
} Font;


typedef struct {
    Vec2 pos;
    Vec2 uv;
    Color col;
} Vertex;

typedef struct{
    Vec2 pos;
    Vec2 quad_pos;
} IR_Vertex;

typedef struct {
    Font font;
    Vertex *draw_vertices;
    u32 draw_vertices_count;
    u32 *draw_indices;
    u32 draw_indices_count;
} Graphics_Context_Atlas;

typedef struct {
    IR_Vertex ir_vertices[6];
    Vec2 *IR_min_max_buffer;
    u32 IR_pixel_count;
} Graphics_Context_IR;

typedef struct {
    IR_Vertex fft_vertices[6];
    real32 *fft_buffer;
    u32 fft_pixel_count;
} Graphics_Context_FFT;

typedef struct {
    Vec2 window_dim;
    Graphics_Context_Atlas atlas;
    Graphics_Context_IR ir;
    Graphics_Context_FFT fft;
} Graphics_Context;

struct Window_Context;

#endif //STRUCTS_H
