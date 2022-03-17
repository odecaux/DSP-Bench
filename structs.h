/* date = November 29th 2021 9:48 am */

#ifndef STRUCTS_H
#define STRUCTS_H

//~ Assets

enum Asset_File_State : u32 {
    Asset_File_State_NONE,
    
    Asset_File_State_STAGE_BACKGROUND_LOADING,
    Asset_File_State_BACKGROUND_LOADING,
    Asset_File_State_STAGE_VALIDATION,
    Asset_File_State_VALIDATING,
    
    Asset_File_State_STAGE_USAGE,
    Asset_File_State_IN_USE,
    Asset_File_State_STAGE_UNLOADING,
    Asset_File_State_OK_TO_UNLOAD,
    Asset_File_State_UNLOADING,
    
    Asset_File_State_COLD_RELOAD_STAGE_UNUSE,
    Asset_File_State_COLD_RELOAD_UNUSING,
    Asset_File_State_COLD_RELOAD_STAGE_UNLOAD,
    Asset_File_State_COLD_RELOAD_UNLOADING,
    
    Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE,
    Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING,
    Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING,
    Asset_File_State_HOT_RELOAD_STAGE_VALIDATION,
    Asset_File_State_HOT_RELOAD_VALIDATING,
    Asset_File_State_HOT_RELOAD_STAGE_SWAP,
    Asset_File_State_HOT_RELOAD_SWAPPING,
    Asset_File_State_HOT_RELOAD_STAGE_DISPOSE,
    Asset_File_State_HOT_RELOAD_DISPOSING,
    
    Asset_File_State_FAILED,
};



//~ Compiler Error Handling

#define CUSTOM_ERROR_FLAG(flag) flag, 

enum Compiler_Error_Flag{
#include "errors.inc"
};
#undef CUSTOM_ERROR_FLAG


enum Compiler_Failure_Stage{
    Compiler_Failure_Stage_Clang_First_Pass,
    Compiler_Failure_Stage_Finding_Decls,
    Compiler_Failure_Stage_Parsing_Parameters,
    Compiler_Failure_Stage_Clang_Second_Pass,
    Compiler_Failure_Stage_No_Failure
};

struct Compiler_Location{
    u32 line;
    u32 column;
};

struct Custom_Error{
    Compiler_Error_Flag flag;
    Compiler_Location location;
    Compiler_Location location_2;
    Compiler_Location location_3;
};

struct Decl_Search_Log{
    Custom_Error audio_callback;
    Custom_Error default_parameters;
    Custom_Error initialize_state;
    Custom_Error parameters_struct;
    Custom_Error state_struct;
};

struct Clang_Error{
    String message;
    Compiler_Location location;
};
struct Clang_Error_Log{
    Clang_Error *errors;
    u32 count;
    u32 capacity;
};

/*
struct Compiler_Gui_Message{
    String message;
};
*/
struct Compiler_Gui_Log{
    String *messages;
    u32 message_count;
    u32 message_capacity;
    
    char *holder_base;
    char *holder_current;
    u32 holder_capacity;
};


//~ Audio File


enum Wav_Reading_Error{
    Wav_Success,
    Wav_Could_Not_Open_File,
    Wav_Not_A_RIFF,
    Wav_File_Reading_Error,
    Wav_Invalid_Format
};

typedef struct {
    Wav_Reading_Error error;
    u32 num_channels;
    u32 samples_by_channel;
    real32** deinterleaved_buffer;
    u32 read_cursor;
} Audio_File;


//~ Plugin

typedef void(*audio_callback_t)(void*, void*, float**, unsigned int, unsigned int, float);
typedef void(*default_parameters_t)(void*);
typedef void(*initialize_state_t)(void*, void*, unsigned int, float, void*);


enum  Plugin_Parameter_Type{
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
    bool log;
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
    Custom_Error error;
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
    String name;
    //le nom c'est le filename ?
    Custom_Error error;
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
typedef struct  {
    Plugin_Parameter_Value *buffer;
    volatile Plugin_Parameter_Value **head;
    u32 writer_idx;
    u32 buffer_size;
    u32 num_fields_by_plugin;
} Plugin_Parameters_Ring_Buffer;


typedef struct Plugin {
    Compiler_Failure_Stage failure_stage;
    Clang_Error_Log clang_error_log;
    Decl_Search_Log decls_search_log;
    
    Plugin_Descriptor descriptor;
    
    void* llvm_jit_engine;
    audio_callback_t audio_callback_f;
    default_parameters_t default_parameters_f;
    initialize_state_t initialize_state_f;
    
    char* parameters_holder;
    char* state_holder;
    
    Plugin_Parameter_Value* parameter_values_audio_side;
    Plugin_Parameter_Value* parameter_values_ui_side;
    Plugin_Parameters_Ring_Buffer ring;
    
} Plugin;


//~ Audio

typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

struct Plugin_Reloading_Manager;

typedef struct 
{
    i8 audio_file_play;
    i8 audio_file_loop;
    i8 plugin_play;
    
    Asset_File_State *plugin_state;
    Asset_File_State *audio_file_state;
    
    Plugin_Reloading_Manager *m;
    Audio_File *audio_file;
} Audio_Thread_Context;

//~ UI

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
    
    bool left_ctrl_pressed;
    bool left_ctrl_down;
} IO;

typedef u32 Color;

enum Colors : u32 {
    Color_Front = 0xffffffff,
    Color_Back = 0x00000000
};

typedef struct{
    i64 selected_parameter_id;
    i64 previous_selected_parameter_id;
    bool show_error_log;
} UI_State;

struct Window_Context;




//~ Fonts

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

//~ Graphics

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
    real32 zoom_state;
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

//~ DSP/IR/FFT

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
