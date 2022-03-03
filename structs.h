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

//TODO should be a struct, and include data, like which parameter is wrong
//I do it the other way around for now : each handle says which type of error it has
//the use case where it doesn't work is signature checking, I don't have a separate handle for every parameter of the signature, but I still want to report the details of what went wrong
//mayber I just need the location of the problem in the source ?

enum Compiler_Error{
    Compiler_Success,
    Compiler_Generic_Error,
    
    Compiler_Initial_Compilation_Error,
    
    Compiler_Too_Many_Fun,
    Compiler_No_Fun,
    Compiler_Wrong_Signature_Fun,
    
    Compiler_Types_Mismatch,
    Compiler_Not_Record_Type,
    Compiler_Polymorphic,
    
    Compiler_Could_Not_Get_Decls,
    
    Compiler_Empty_Annotation,
    Compiler_Invalid_Annotation,
    Compiler_Missing_Min_Max,
    Compiler_Min_Greater_Than_Max,
    Compiler_Invalid_Min_Value,
    Compiler_Invalid_Max_Value,
    Compiler_Annotation_Type_Mismatch,
    
    Compiler_Struct_Parsing_Error,
    
    Compiler_Rewritten_Compilation_Error,
    Compiler_Cant_Launch_Jit,
};

struct Compiler_Errors{
    Compiler_Error *errors;
    u32 count;
    u32 capacity;
};


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
    i64 value;
    String name;
} Parameter_Enum_Entry;


typedef struct {
    Parameter_Enum_Entry *entries;
    u32 num_entries;
} Parameter_Enum;

typedef struct {
    Compiler_Error error;
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
    Compiler_Error error;
    struct { 
        i64 size;
        i64 alignment;
    } parameters_struct;
    
    struct { 
        i64 size;
        i64 alignment;
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
    Compiler_Error error;
    Plugin_Descriptor descriptor;
    
    void* llvm_jit_engine;
    audio_callback_t audio_callback_f;
    default_parameters_t default_parameters_f;
    initialize_state_t initialize_state_f;
} Plugin_Handle;

typedef Plugin_Handle(*try_compile_t)(const char*, const void*, Compiler_Errors*);

typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

typedef struct 
{
    i8 audio_file_valid;
    i8 audio_file_play;
    i8 audio_file_loop;
    
    real32** audio_file_buffer;
    u64 audio_file_length;
    u64 audio_file_read_cursor;
    u64 audio_file_num_channels;
    
    i8 plugin_valid;
    i8 plugin_play;
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
    i64 hovered_id;
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
    Rect bounds;
    real32 *IR_buffer;
    u32 IR_sample_count;
} Graphics_Context_IR;

typedef struct {
    Rect bounds;
    real32 *fft_buffer;
    u32 fft_sample_count;
} Graphics_Context_FFT;

typedef struct {
    Vec2 window_dim;
    Graphics_Context_Atlas atlas;
    Graphics_Context_IR ir;
    Graphics_Context_FFT fft;
} Graphics_Context;

struct Window_Context;

typedef struct {
    i32 fft_order;
    i32 buffer_size;
    u8* spec_holder;
    void* spec;
    u8 *work_buffer;
    real32 *temp_perm_buffer; //TODO rename, c'est pas clair si on connait pas IPP
} Ipp_Order_Context;
//TODO rename, c'est nul comme nom ORDER, en 
typedef struct {
    i32 highest_order;
    Ipp_Order_Context *order_to_ctx;
} Ipp_Context;

typedef struct {
    u32 ir_sample_count;
    u32 ir_num_channels;
    Ipp_Context ipp_context;
    real32** IR_buffer;
    real32** windowed_zero_padded_buffer;
    Vec2* fft_out;
    real32 *magnitudes;
} FFT;

#endif //STRUCTS_H
