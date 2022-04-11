/* date = February 1st 2022 8:38 am */

#ifndef OPENGL_H
#define OPENGL_H

#include <GL/GL.h>



typedef struct{
    u32 shader_program;
    u32 vao;
    u32 vertex_buffer;
    u32 index_buffer;
    
    GLint uniform_mvp;
    GLint uniform_texture;
    GLint attribute_pos;
    GLint attribute_uv;
    GLint attribute_col;
    //u32 texture_handle; //TODO c'est chiant, ça veut dire qu'on l'a à deux endroits séparés ????
} OpenGL_Context_Atlas;

typedef struct{
    u32 integrate_shader_program;
    
    u32 integrate_vao;
    u32 integrate_vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 integrate_min_max_buffer;
    u32 integrate_min_max_texture;
    
    GLint integrate_uniform_mvp;
    GLint integrate_attribute_pos;
    GLint integrate_attribute_quad_pos;
    
    Vec2 *integrate_temp_buffer;
    //u32 previous_pixel_length; TODO cache
    
    u32 interpolate_shader_program;
    
    u32 interpolate_vao;
    u32 interpolate_vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 interpolate_buffer;
    u32 interpolate_texture;
    
    GLint interpolate_uniform_mvp;
    GLint interpolate_attribute_pos;
    GLint interpolate_attribute_quad_pos;
} OpenGL_Context_IR;

typedef struct {
    u32 shader_program;
    
    u32 vao;
    u32 vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 data_buffer;
    u32 data_texture;
    
    GLint uniform_mvp;
    GLint attribute_pos;
    GLint attribute_quad_pos;
} OpenGL_Context_FFT;

typedef struct 
{
    HDC window_dc;
    HGLRC opengl_rc;
} OpenGL_Context_Win32;

typedef struct {
    OpenGL_Context_Win32 win32;
    OpenGL_Context_Atlas atlas;
    OpenGL_Context_IR ir;
    OpenGL_Context_FFT fft;
} OpenGL_Context;

//~ OpenGL defines
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ARRAY_BUFFER                   0x8892
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_STATIC_DRAW                    0x88E4
#define GL_STREAM_DRAW                    0x88E0
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
//TODO est-ce qu'on a vraiment besoin de ça ? on a pas tant de triangles que ça à priori
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_R32F                           0x822E
#define GL_RG32F                          0x8230

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

#define OPENGL_PROC(ret, name, param)  typedef ret(* name ## _t)param
#include "opengl_proc.inc"
#undef OPENGL_PROC


#define OPENGL_PROC(ret, name, param)  static name ## _t name = nullptr 
#include "opengl_proc.inc"
#undef OPENGL_PROC

GLenum opengl_check_error_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        const char *error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            default: error = "other error"; break;
        }
        printf("opengl error :  %s in %s:%d\n", error, file, line);
    }
    return errorCode;
}
#define opengl_check_error() opengl_check_error_(__FILE__, __LINE__) 

bool load_opengl_functions()
{
    
#define OPENGL_PROC(ret, function_name, param)  function_name = (function_name##_t) wglGetProcAddress(#function_name); if(!function_name) { printf("failed to load" #function_name "\n"); return false; }
#include "opengl_proc.inc"
#undef OPENGL_PROC
    
    return true;
}

OpenGL_Context_Win32 opengl_initialize_win32(HWND window)
{
    
    HDC window_dc = GetDC(window);
    
    const PIXELFORMATDESCRIPTOR pixel_format = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,        
        32,                   
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   
        8,                   
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    const int pixel_format_idx = ChoosePixelFormat(window_dc, &pixel_format);
    if(pixel_format_idx == 0)
    {
        printf("couldn't find a valid pixel format\n");
        exit(1);
    }
    
    SetPixelFormat(window_dc, pixel_format_idx, &pixel_format);
    
    
    const HGLRC tempContext = wglCreateContext(window_dc);
    if(wglMakeCurrent(window_dc, tempContext) == FALSE)
    {
        printf("couldn't open opengl temp context\n");
        exit(1);
    }
    
    const auto wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");
    
    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, true ? WGL_CONTEXT_DEBUG_BIT_ARB : 0, //TODO debug / release 
        0
    };
    
    const HGLRC opengl_rc = wglCreateContextAttribsARB(window_dc, 0, attribs);
    if (opengl_rc == 0)
    {
        printf("couldn't create the elevated opengl context\n");
        exit(0);
    }
    
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    if(wglMakeCurrent(window_dc, opengl_rc) == FALSE)
    {
        printf("couldn't open opengl elevated context\n");
        exit(1);
    }
    
    if(!load_opengl_functions())
    {
        printf("failed to load opengl functions\n");
        exit(1);
    }
    
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    //printf("opengl version : %d.%d\n", major, minor);
    
    const char* target_openg_version = "#version 330 core";
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    wglSwapIntervalEXT(1);
    return {
        .window_dc = window_dc,
        .opengl_rc = opengl_rc
    };
}

OpenGL_Context_Atlas opengl_initialize_atlas(Font* font)
{
    GLint status = GL_TRUE;
    
    const char *atlas_vertex_shader_source = 
        "#version 330 core\n"
        "uniform mat4 mvp;"
        
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 uv;\n"
        "layout (location = 2) in vec4 col;\n"
        
        "out vec2 frag_uv;\n"
        "out vec4 frag_col;\n"
        
        "void main()\n"
        "{\n"
        "    frag_uv  = uv;\n"
        "    frag_col = col;\n"
        "   gl_Position =  mvp * vec4(pos.x, pos.y, 1.0, 1.0);\n"
        "}\0";
    
    const u32 atlas_vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(atlas_vertex_shader_handle, 1, &atlas_vertex_shader_source, NULL);
    glCompileShader(atlas_vertex_shader_handle);
    glGetShaderiv(atlas_vertex_shader_handle, GL_COMPILE_STATUS, &status);
    ensure(status == GL_TRUE);
    
    const char *atlas_fragment_shader_source =  
        "#version 330 core\n"
        "in vec2 frag_uv;\n"
        "in vec4 frag_col;\n"
        
        "uniform sampler2D texture;\n"
        "layout (location = 0) out vec4 out_col;\n"
        
        "void main()\n"
        "{\n"
        "    out_col = frag_col * texture(texture, frag_uv.st);\n"
        "}\n";
    
    const u32 atlas_fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(atlas_fragment_shader_handle, 1, &atlas_fragment_shader_source, NULL);
    glCompileShader(atlas_fragment_shader_handle);
    glGetShaderiv(atlas_fragment_shader_handle, GL_COMPILE_STATUS, &status);
    ensure(status == GL_TRUE);
    
    const u32 atlas_shader_program = glCreateProgram();
    glAttachShader(atlas_shader_program, atlas_vertex_shader_handle);
    glAttachShader(atlas_shader_program, atlas_fragment_shader_handle);
    glLinkProgram(atlas_shader_program);
    glGetProgramiv(atlas_shader_program, GL_LINK_STATUS, &status);
    ensure(status == GL_TRUE);
    opengl_check_error();
    
    glDeleteShader(atlas_vertex_shader_handle);
    glDeleteShader(atlas_fragment_shader_handle);
    
    
    const GLint atlas_uniform_mvp = glGetUniformLocation(atlas_shader_program, "mvp");
    const GLint 
        atlas_uniform_texture = glGetUniformLocation(atlas_shader_program, "texture");
    const GLint atlas_attribute_pos = glGetAttribLocation(atlas_shader_program, "pos");
    const GLint atlas_attribute_uv = glGetAttribLocation(atlas_shader_program, "uv");	
    const GLint atlas_attribute_col = (GLuint)glGetAttribLocation(atlas_shader_program, "col");
    opengl_check_error();
    
    ////////////////////////////////
    u32 atlas_vao; glGenVertexArrays(1, &atlas_vao);
    u32 atlas_vertex_buffer; glGenBuffers(1, &atlas_vertex_buffer);
    u32 atlas_index_buffer; glGenBuffers(1, &atlas_index_buffer);
    glGenTextures(1, &font->atlas_texture_id);
    
    glBindVertexArray(atlas_vao);
    glBindBuffer(GL_ARRAY_BUFFER, atlas_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atlas_index_buffer);
    glBindTexture(GL_TEXTURE_2D, font->atlas_texture_id);
    
    
    glEnableVertexAttribArray(atlas_attribute_pos);
    glVertexAttribPointer(atlas_attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OffsetOf(Vertex, pos));
    
    glEnableVertexAttribArray(atlas_attribute_uv);
    glVertexAttribPointer(atlas_attribute_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OffsetOf(Vertex, uv));
    
    glEnableVertexAttribArray(atlas_attribute_col);
    glVertexAttribPointer(atlas_attribute_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)OffsetOf(Vertex, col));
    
    //TODO hardcoded buffer size
    glBufferData(GL_ARRAY_BUFFER, ATLAS_MAX_VERTEX_COUNT* sizeof(Vertex), NULL, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ATLAS_MAX_VERTEX_COUNT * sizeof(u32), NULL, GL_STREAM_DRAW);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 font->atlas_texture_dim.x, font->atlas_texture_dim.y, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, font->atlas_pixels);
    
    opengl_check_error();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return {
        .shader_program = atlas_shader_program, 
        .vao = atlas_vao, 
        .vertex_buffer = atlas_vertex_buffer, 
        .index_buffer= atlas_index_buffer,
        
        .uniform_mvp = atlas_uniform_mvp, 
        .uniform_texture = atlas_uniform_texture,
        .attribute_pos = atlas_attribute_pos,
        .attribute_uv = atlas_attribute_uv,
        .attribute_col = atlas_attribute_col,
    };
}

OpenGL_Context_IR opengl_initialize_ir()
{
    GLint status = GL_TRUE;
    
    const char *ir_vertex_shader_source = 
        "#version 330 core\n"
        
        "uniform mat4 mvp;\n"
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 quad_pos_in;\n"
        
        "out vec2 quad_pos;\n"
        
        "void main()\n"
        "{\n"
        "   gl_Position =  mvp * vec4(pos.x, pos.y, 1.0, 1.0);\n"
        "   quad_pos = quad_pos_in;\n"
        "}\0";
    //printf("%s\n", ir_vertex_shader_source);
    const u32 ir_vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ir_vertex_shader_handle, 1, &ir_vertex_shader_source, NULL);
    glCompileShader(ir_vertex_shader_handle);
    glGetShaderiv(ir_vertex_shader_handle, GL_COMPILE_STATUS, &status);
    ensure(status == GL_TRUE);
    
    
    //~ accumulate
    
    const char *ir_integrate_fragment_shader_source =  
        "#version 330 core\n"
        
        "uniform samplerBuffer IR;\n"
        "in vec2 quad_pos;\n"
        
        "float plot(vec2 st, float pct){\n"
        "    return  smoothstep( pct-0.04, pct, st.y) - \n"
        "            smoothstep( pct, pct+0.04, st.y);\n"
        "}\n"
        
        "void main()\n"
        "{\n"
        "    int ir_buffer_size = textureSize(IR);\n"
        "    float when_max = max(sign(quad_pos.y - 0.5f), 0.0f);\n"
        "    float when_min = max(sign(0.5f - quad_pos.y), 0.0f);\n"
        
        "    float x = quad_pos.x * (ir_buffer_size - 1);\n"
        "    vec4 y = texelFetch(IR, int(floor(x)));\n"
        
        "    float value_max = step((quad_pos.y - 0.5) - y.r / 2.0, 0.01f);\n"
        "    float value_min = step(y.g / 2.0 - (quad_pos.y - 0.5), 0.01f);\n"
        
        "    float value = value_min * when_min + value_max * when_max;\n"
        "    gl_FragColor  = value * vec4(1.0f);\n"
        //"    gl_FragColor = when_min * vec4(1.0, 1.0, 1.0, 1.0) + when_max * vec4(1.0, 0.0, 0.0, 1.0);"
        "}\n";
    
    
    const u32 ir_integrate_fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ir_integrate_fragment_shader_handle, 1, &ir_integrate_fragment_shader_source, NULL);
    glCompileShader(ir_integrate_fragment_shader_handle);
    glGetShaderiv(ir_integrate_fragment_shader_handle, GL_COMPILE_STATUS, &status);
    ensure(status == GL_TRUE);
    
    const u32 ir_integrate_shader_program = glCreateProgram();
    glAttachShader(ir_integrate_shader_program, ir_vertex_shader_handle);
    glAttachShader(ir_integrate_shader_program, ir_integrate_fragment_shader_handle);
    glLinkProgram(ir_integrate_shader_program);
    glGetProgramiv(ir_integrate_shader_program, GL_LINK_STATUS, &status);
    ensure(status == GL_TRUE);
    opengl_check_error();
    
    glDeleteShader(ir_integrate_fragment_shader_handle);
    
    const GLint ir_integrate_uniform_mvp = glGetUniformLocation(ir_integrate_shader_program, "mvp");
    const GLint ir_integrate_attribute_pos = glGetAttribLocation(ir_integrate_shader_program, "pos");
    const GLint ir_integrate_attribute_quad_pos = glGetAttribLocation(ir_integrate_shader_program, "quad_pos_in");
    opengl_check_error();
    
    ////////////////////////////////
    
    u32 ir_integrate_vao; glGenVertexArrays(1, &ir_integrate_vao);
    u32 ir_integrate_vertex_buffer; glGenBuffers(1, &ir_integrate_vertex_buffer);
    
    u32 ir_integrate_min_max_buffer; glGenBuffers(1, &ir_integrate_min_max_buffer);
    u32 ir_integrate_min_max_texture; glGenTextures(1, &ir_integrate_min_max_texture);
    
    glBindVertexArray(ir_integrate_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ir_integrate_vertex_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, ir_integrate_min_max_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, ir_integrate_min_max_texture);
    
    glEnableVertexAttribArray(ir_integrate_attribute_pos);
    glVertexAttribPointer(ir_integrate_attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, pos));
    glEnableVertexAttribArray(ir_integrate_attribute_quad_pos);
    glVertexAttribPointer(ir_integrate_attribute_quad_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, quad_pos));
    
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(IR_Vertex) * 6, NULL, GL_STREAM_DRAW);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(Vec2) * IR_BUFFER_LENGTH, NULL, GL_STREAM_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, ir_integrate_min_max_buffer);
    
    
    //~ interpolate
    
    const char *ir_interpolate_fragment_shader_source =  
        "#version 330 core\n"
        
        "uniform samplerBuffer IR;\n"
        "in vec2 quad_pos;\n"
        
        "float plot(vec2 st, float pct){\n"
        "    return  smoothstep( pct-0.04, pct, st.y) - \n"
        "            smoothstep( pct, pct+0.04, st.y);\n"
        "}\n"
        
        "void main()\n"
        "{\n"
        "    int ir_buffer_size = textureSize(IR);\n"
        
        "    float x = quad_pos.x * (ir_buffer_size - 1);\n"
        "    float n = floor(x);\n"  // integer
        "    float f = fract(x);\n"  // fraction
    
        "    vec4 prev = texelFetch(IR, int(n));\n"
        "    vec4 next = texelFetch(IR, int(n) + 1);\n"
        
        "    float y = mix(prev.r, next.r, smoothstep(0.,1.,f));\n"
        "    float value = step(abs(y - (quad_pos.y * 2 - 1)) - 0.02f, 0.0f);\n"
        "    gl_FragColor  = value * vec4(1.0f);\n"
        "}\n";
    
    const u32 ir_interpolate_fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    //TODO fix, charger les deux shaders ???
    glShaderSource(ir_interpolate_fragment_shader_handle, 1, &ir_interpolate_fragment_shader_source, NULL);
    glCompileShader(ir_interpolate_fragment_shader_handle);
    glGetShaderiv(ir_interpolate_fragment_shader_handle, GL_COMPILE_STATUS, &status);
    ensure(status == GL_TRUE);
    
    const u32 ir_interpolate_shader_program = glCreateProgram();
    glAttachShader(ir_interpolate_shader_program, ir_vertex_shader_handle);
    glAttachShader(ir_interpolate_shader_program, ir_interpolate_fragment_shader_handle);
    glLinkProgram(ir_interpolate_shader_program);
    glGetProgramiv(ir_interpolate_shader_program, GL_LINK_STATUS, &status);
    ensure(status == GL_TRUE);
    opengl_check_error();
    
    glDeleteShader(ir_vertex_shader_handle);
    glDeleteShader(ir_interpolate_fragment_shader_handle);
    
    const GLint ir_interpolate_uniform_mvp = glGetUniformLocation(ir_interpolate_shader_program, "mvp");
    const GLint ir_interpolate_attribute_pos = glGetAttribLocation(ir_interpolate_shader_program, "pos");
    const GLint ir_interpolate_attribute_quad_pos = glGetAttribLocation(ir_interpolate_shader_program, "quad_pos_in");
    opengl_check_error();
    
    ////////////////////////////////
    
    u32 ir_interpolate_vao; glGenVertexArrays(1, &ir_interpolate_vao);
    u32 ir_interpolate_vertex_buffer; glGenBuffers(1, &ir_interpolate_vertex_buffer);
    
    u32 ir_interpolate_buffer; glGenBuffers(1, &ir_interpolate_buffer);
    u32 ir_interpolate_texture; glGenTextures(1, &ir_interpolate_texture);
    
    glBindVertexArray(ir_interpolate_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ir_interpolate_vertex_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, ir_interpolate_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, ir_interpolate_texture);
    
    glEnableVertexAttribArray(ir_interpolate_attribute_pos);
    glVertexAttribPointer(ir_interpolate_attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, pos));
    glEnableVertexAttribArray(ir_interpolate_attribute_quad_pos);
    glVertexAttribPointer(ir_interpolate_attribute_quad_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, quad_pos));
    
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(IR_Vertex) * 6, NULL, GL_STREAM_DRAW);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(real32) * IR_BUFFER_LENGTH, NULL, GL_STREAM_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, ir_interpolate_buffer);
    
    
    OpenGL_Context_IR test = {
        
        .integrate_shader_program = ir_integrate_shader_program,
        .integrate_vao = ir_integrate_vao, 
        .integrate_vertex_buffer = ir_integrate_vertex_buffer, 
        .integrate_min_max_buffer = ir_integrate_min_max_buffer,
        .integrate_min_max_texture = ir_integrate_min_max_texture,
        
        .integrate_uniform_mvp = ir_integrate_uniform_mvp, 
        .integrate_attribute_pos = ir_integrate_attribute_pos,
        .integrate_attribute_quad_pos = ir_integrate_attribute_quad_pos,
        
        .integrate_temp_buffer = m_allocate_array(Vec2, IR_MAX_PIXEL_WIDTH, "graphics : ir integration buffer"),
        
        .interpolate_shader_program = ir_interpolate_shader_program,
        .interpolate_vao = ir_interpolate_vao, 
        .interpolate_vertex_buffer = ir_interpolate_vertex_buffer, 
        .interpolate_buffer = ir_interpolate_buffer,
        .interpolate_texture = ir_interpolate_texture,
        
        .interpolate_uniform_mvp = ir_interpolate_uniform_mvp, 
        .interpolate_attribute_pos = ir_interpolate_attribute_pos,
        .interpolate_attribute_quad_pos = ir_interpolate_attribute_quad_pos,
    };
    return test;
}


OpenGL_Context_FFT opengl_initialize_fft()
{
    GLint status = GL_TRUE;
    
    const char *fft_vertex_shader_source = 
        "#version 330 core\n"
        
        "uniform mat4 mvp;\n"
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 quad_pos_in;\n"
        
        "out vec2 quad_pos;\n"
        
        "void main()\n"
        "{\n"
        "   gl_Position =  mvp * vec4(pos.x, pos.y, 1.0, 1.0);\n"
        "   quad_pos = quad_pos_in;\n"
        "}\0";
    
    //printf("%s\n", fft_vertex_shader_source);
    const u32 fft_vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(fft_vertex_shader_handle, 1, &fft_vertex_shader_source, NULL);
    glCompileShader(fft_vertex_shader_handle);
    glGetShaderiv(fft_vertex_shader_handle, GL_COMPILE_STATUS, &status);
	ensure(status == GL_TRUE);
    
    
    
    const char *fft_fragment_shader_source =  
        "#version 330 core\n"
        
        "uniform samplerBuffer IR;\n"
        "in vec2 quad_pos;\n"
        
        "float plot(vec2 st, float pct){\n"
        "    return  smoothstep( pct-0.04, pct, st.y) - \n"
        "            smoothstep( pct, pct+0.04, st.y);\n"
        "}\n"
        
        "void main()\n"
        "{\n"
        "    int fft_buffer_size = textureSize(IR);\n"
        "    float x = quad_pos.x * (fft_buffer_size - 1);\n"
        "    float n = floor(x);\n"  // integer
        "    float f = fract(x);\n"  // fraction
        "    float prev = texelFetch(IR, int(n)).r;\n"
        "    float next = texelFetch(IR, int(n) + 1).r;\n"
        "    float y = mix(prev, next, smoothstep(0.,1.,f));\n"
        "    gl_FragColor  = plot(quad_pos, y) * vec4(1.0);\n"
        "}\n";
    
    
    //printf("%s\n", fft_fragment_shader_source);
    
    const u32 fft_fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fft_fragment_shader_handle, 1, &fft_fragment_shader_source, NULL);
    glCompileShader(fft_fragment_shader_handle);
    glGetShaderiv(fft_fragment_shader_handle, GL_COMPILE_STATUS, &status);
	ensure(status == GL_TRUE);
    
    const u32 fft_shader_program = glCreateProgram();
    glAttachShader(fft_shader_program, fft_vertex_shader_handle);
    glAttachShader(fft_shader_program, fft_fragment_shader_handle);
    glLinkProgram(fft_shader_program);
    glGetProgramiv(fft_shader_program, GL_LINK_STATUS, &status);
    ensure(status == GL_TRUE);
    opengl_check_error();
    
    glDeleteShader(fft_vertex_shader_handle);
    glDeleteShader(fft_fragment_shader_handle);
    
    const GLint fft_uniform_mvp = glGetUniformLocation(fft_shader_program, "mvp");
    const GLint fft_attribute_pos = glGetAttribLocation(fft_shader_program, "pos");
    const GLint fft_attribute_quad_pos = glGetAttribLocation(fft_shader_program, "quad_pos_in");
    opengl_check_error();
    
    ////////////////////////////////
    
    u32 fft_vao; glGenVertexArrays(1, &fft_vao);
    u32 fft_vertex_buffer; glGenBuffers(1, &fft_vertex_buffer);
    
    u32 fft_data_buffer; glGenBuffers(1, &fft_data_buffer);
    u32 fft_data_texture; glGenTextures(1, &fft_data_texture);
    
    glBindVertexArray(fft_vao);
    glBindBuffer(GL_ARRAY_BUFFER, fft_vertex_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, fft_data_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, fft_data_texture);
    
    glEnableVertexAttribArray(fft_attribute_pos);
    glVertexAttribPointer(fft_attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, pos));
    glEnableVertexAttribArray(fft_attribute_quad_pos);
    glVertexAttribPointer(fft_attribute_quad_pos, 2, GL_FLOAT, GL_FALSE, sizeof(IR_Vertex), (void*)OffsetOf(IR_Vertex, quad_pos));
    
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(IR_Vertex) * 6, NULL, GL_STREAM_DRAW);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(real32) * IR_BUFFER_LENGTH * 2, NULL, GL_STREAM_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, fft_data_buffer);
    
    return{
        .shader_program = fft_shader_program,
        .vao = fft_vao, 
        .vertex_buffer = fft_vertex_buffer, 
        .data_buffer = fft_data_buffer,
        .data_texture = fft_data_texture,
        
        .uniform_mvp = fft_uniform_mvp, 
        .attribute_pos = fft_attribute_pos,
        .attribute_quad_pos = fft_attribute_quad_pos,
    };
}

OpenGL_Context opengl_initialize(Window_Backend_Context *window, Font* font)
{
    
    OpenGL_Context_Win32 ctx_win32 = opengl_initialize_win32(window->window);
    OpenGL_Context_Atlas ctx_atlas = opengl_initialize_atlas(font);
    OpenGL_Context_IR ctx_ir = opengl_initialize_ir();
    OpenGL_Context_FFT ctx_fft = opengl_initialize_fft();
    
    //TODO comment on fait pour libérer cette mémoire ?
    //m_free(font->atlas_pixels, "opengl : atlas pixels");
    font->atlas_pixels = nullptr;
    
    ReleaseDC(window->window, ctx_win32.window_dc);
    
    return OpenGL_Context{
        .win32 = ctx_win32,
        .atlas = ctx_atlas,
        .ir = ctx_ir,
        .fft = ctx_fft
    };
}


void rect_to_ir_vertices(Rect rect, IR_Vertex *out_vertices)
{
    Vec2 top_left = rect.origin;
    Vec2 top_right = Vec2{ rect.x + rect.w, rect.y };
    Vec2 bottom_right = Vec2{ rect.x + rect.w, rect.y + rect.h};
    Vec2 bottom_left = Vec2{ rect.x, rect.y + rect.h};
    
    Vec2 uv_top_left = Vec2{ 0.0f, 1.0f};
    Vec2 uv_top_right = Vec2{ 1.0f, 1.0f};
    Vec2 uv_bottom_left = Vec2{ 0.0f, 0.0f};
    Vec2 uv_bottom_right = Vec2{ 1.0f, 0.0f};
    
    out_vertices[0] = { bottom_left,  uv_bottom_left};
    out_vertices[1] = { top_right,    uv_top_right};
    out_vertices[2] = { top_left,     uv_top_left};
    out_vertices[3] = { bottom_left,  uv_bottom_left};
    out_vertices[4] = { bottom_right, uv_bottom_right};
    out_vertices[5] = { top_right,    uv_top_right};
}


void opengl_render(OpenGL_Context *opengl_ctx, Graphics_Context *graphics_ctx)
{
    Vec2 window_dim = graphics_ctx->window_dim;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    glViewport(0,0, window_dim.x, window_dim.y);
    glScissor(0,0, window_dim.x, window_dim.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    real32 projection_matrix[16] = 
    {
        2.0f / window_dim.x, 0.0f, 0.0f, 0.0f,
        0.0f,- 2.0f / window_dim.y, 0.0f, 0.0f ,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };
    
    Draw_Command_List *cmd_list = &graphics_ctx->command_list; 
    
    //collapse popups onto the main command list
    {
        Draw_Command_List *popup_cmd_list = &graphics_ctx->popup_command_list; 
        
        //src -> dest
        memcpy(cmd_list->draw_vertices + cmd_list->draw_vertices_count,
               popup_cmd_list->draw_vertices,
               popup_cmd_list->draw_vertices_count * sizeof(Vertex));
        
        for(u32 i = 0; i < popup_cmd_list->draw_indices_count; i++)
        {
            popup_cmd_list->draw_indices[i] += cmd_list->draw_vertices_count;
        }
        
        memcpy(cmd_list->draw_indices + cmd_list->draw_indices_count,
               popup_cmd_list->draw_indices, 
               popup_cmd_list->draw_indices_count * sizeof(u32));
        
        for(u32 i = 0; i < popup_cmd_list->draw_command_count; i++)
        {
            Draw_Command *cmd = &popup_cmd_list->draw_commands[i];
            if(cmd->type == Draw_Command_Type_ATLAS)
            {
                cmd->atlas.idx_offset += cmd_list->draw_indices_count;
                cmd->atlas.vertex_offset += cmd_list->draw_vertices_count;
            }
        }
        
        memcpy(cmd_list->draw_commands + cmd_list->draw_command_count,
               popup_cmd_list->draw_commands, 
               popup_cmd_list->draw_command_count * sizeof(Draw_Command));
        
        
        cmd_list->draw_indices_count += popup_cmd_list->draw_indices_count;
        cmd_list->draw_vertices_count += popup_cmd_list->draw_vertices_count;
        cmd_list->draw_command_count += popup_cmd_list->draw_command_count;
        
        popup_cmd_list->draw_command_count = 0;
        popup_cmd_list->draw_vertices_count = 0;
        popup_cmd_list->draw_indices_count = 0;
    }
    
    for(i32 vtx_idx = 0; vtx_idx < cmd_list->draw_vertices_count; vtx_idx++)
    {
        Vec2 *uv = &cmd_list->draw_vertices[vtx_idx].uv;
        if(uv->x == -1.0f || uv->y == -1.0f)
        {
            *uv = graphics_ctx->font.white_rect_pos;
        }
    }
    
    i32 last_command_type = -1;
    
    for(u32 cmd_idx = 0; cmd_idx < cmd_list->draw_command_count; cmd_idx++)
    {
        Draw_Command *command = &cmd_list->draw_commands[cmd_idx]; 
        Draw_Command_Type type = command->type;
        
        if(type == Draw_Command_Type_ATLAS)
        {
            if(last_command_type != type)
            {
                glUseProgram(opengl_ctx->atlas.shader_program);
                glBindVertexArray(opengl_ctx->atlas.vao);
                glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->atlas.vertex_buffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_ctx->atlas.index_buffer);
                glBindTexture(GL_TEXTURE_2D, graphics_ctx->font.atlas_texture_id);
                
                //TODO est-ce que je peux supprimer ça ?
                //glUniform1i(opengl_ctx->atlas_uniform_texture, 0);
                glUniformMatrix4fv(opengl_ctx->atlas.uniform_mvp, 
                                   1, 
                                   GL_FALSE, 
                                   projection_matrix);
            }
            
            glBufferSubData(GL_ARRAY_BUFFER, 0, 
                            cmd_list->draw_vertices_count * sizeof(Vertex), 
                            cmd_list->draw_vertices);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 
                            cmd_list->draw_indices_count * sizeof(u32), 
                            cmd_list->draw_indices);
            
            Draw_Command_Atlas *atlas_command = &command->atlas;
            Rect clip_rect = atlas_command->clip_rect;
            /*
            if (clip_rect.w <= 0.0f || clip_rect.h <= 0.0f)
                continue;
            */
            glScissor(clip_rect.x, window_dim.y - clip_rect.y - clip_rect.h,  clip_rect.w, clip_rect.h);
            
            glDrawElements(GL_TRIANGLES, (GLsizei)atlas_command->idx_count, GL_UNSIGNED_INT, 
                           (void*)(intptr_t)(atlas_command->idx_offset * sizeof(u32)));
            
        }
        else if(type == Draw_Command_Type_FFT)
        {
            Draw_Command_FFT *fft_command = &command->fft;
            
            glUseProgram(opengl_ctx->fft.shader_program);
            glBindVertexArray(opengl_ctx->fft.vao);
            glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->fft.vertex_buffer);
            glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->fft.data_buffer);
            glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->fft.data_texture);
            
            glUniformMatrix4fv(opengl_ctx->fft.uniform_mvp, 
                               1, 
                               GL_FALSE, 
                               projection_matrix);
            
            Rect bounds = rect_shrinked(fft_command->bounds, 1.0f, 1.0f);
            
            IR_Vertex fft_vertices[6];
            rect_to_ir_vertices(bounds, fft_vertices);
            
            glBufferSubData(GL_ARRAY_BUFFER, 0, 
                            6 * sizeof(IR_Vertex),
                            fft_vertices);
            glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                            fft_command->sample_count * sizeof(real32), 
                            fft_command->buffer);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        else if(type == Draw_Command_Type_IR)
        {
            Draw_Command_IR *ir_command = &command->ir;
            Rect bounds = rect_shrinked(ir_command->bounds, 1.0f, 1.0f);
            
            IR_Vertex ir_vertices[6];
            rect_to_ir_vertices(bounds, ir_vertices);
            
            i32 pixel_count = (i32)bounds.w;
            ensure(pixel_count >= 0);
            i32 sample_count = ir_command->sample_count;
            
            Vec2 *temp = opengl_ctx->ir.integrate_temp_buffer;
            
            if(bounds.w < sample_count)
            {
                
                for(u32 pixel_idx = 0; pixel_idx < pixel_count; pixel_idx++)
                {
                    temp[pixel_idx] = {.a = -1.0f, .b = 1.0f};
                }
                for(u32 sample = 0; sample < sample_count; sample++)
                {
                    real32 val = ir_command->buffer[sample];
                    u32 pixel_idx  = sample * pixel_count / sample_count;
                    temp[pixel_idx].a = octave_max(temp[pixel_idx].a, val);
                    temp[pixel_idx].b = octave_min(temp[pixel_idx].b, val);
                } 
                
                
                
                glUseProgram(opengl_ctx->ir.integrate_shader_program);
                glBindVertexArray(opengl_ctx->ir.integrate_vao);
                glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->ir.integrate_vertex_buffer);
                glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->ir.integrate_min_max_buffer);
                glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->ir.integrate_min_max_texture);
                
                glUniformMatrix4fv(opengl_ctx->ir.integrate_uniform_mvp, 
                                   1, 
                                   GL_FALSE, 
                                   projection_matrix);
                
                glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                6 * sizeof(IR_Vertex),
                                ir_vertices);
                
                glTexBufferRange(GL_TEXTURE_BUFFER, GL_RG32F, opengl_ctx->ir.integrate_min_max_buffer, 0, sizeof(Vec2) * bounds.w);
                glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                                bounds.w * sizeof(Vec2), 
                                temp);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                
            }
            else {
                glUseProgram(opengl_ctx->ir.interpolate_shader_program);
                glBindVertexArray(opengl_ctx->ir.interpolate_vao);
                glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->ir.interpolate_vertex_buffer);
                glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->ir.interpolate_buffer);
                glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->ir.interpolate_texture);
                
                glUniformMatrix4fv(opengl_ctx->ir.interpolate_uniform_mvp, 
                                   1, 
                                   GL_FALSE, 
                                   projection_matrix);
                
                glBufferSubData(GL_ARRAY_BUFFER, 0, 
                                6 * sizeof(IR_Vertex),
                                ir_vertices);
                glTexBufferRange(GL_TEXTURE_BUFFER, GL_R32F, opengl_ctx->ir.interpolate_buffer, 0, sizeof(real32) * sample_count);
                
                glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                                sample_count *  sizeof(real32), 
                                ir_command->buffer);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        last_command_type = type;
    }
    SwapBuffers(opengl_ctx->win32.window_dc);
    //TODO est-ce qu'on gagne vraiment en latence avec ça ?
    //glFinish();
}

void opengl_uninitialize(OpenGL_Context *opengl_ctx)
{
    
    /*
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    */
    
    wglMakeCurrent (NULL, NULL) ; 
    wglDeleteContext (opengl_ctx->win32.opengl_rc);
}
#endif //OPENGL_H
