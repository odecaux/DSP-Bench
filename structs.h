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

//~ DSP/IR/FFT

typedef struct {
    i32 fft_order;
    i32 buffer_size;
    u8* spec_holder;
    void* spec;
} Ipp_Order_Context;

//TODO rename, c'est nul comme nom ORDER, en 
typedef struct {
    i32 highest_order;
    u8 *work_buffer;
    real32 *temp_perm_buffer; //TODO rename, c'est pas clair si on connait pas IPP
    Ipp_Order_Context *order_to_ctx;
} Ipp_Context;

typedef Ipp_Context Plugin_FFT_Context;

typedef struct {
    u32 ir_sample_count;
    u32 ir_num_channels;
    Ipp_Context ipp_context;
    real32 **IR_buffer;
    real32** windowed_zero_padded_buffer;
    Vec2* fft_out;
    real32 *magnitudes;
} Analysis;

#endif //STRUCTS_H
