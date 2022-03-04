/* date = February 1st 2022 8:38 am */

#ifndef OPENGL_H
#define OPENGL_H

#include <GL/GL.h>

typedef struct{
    u32 atlas_shader_program;
    u32 atlas_vao;
    u32 atlas_vertex_buffer;
    u32 atlas_index_buffer;
    
    GLint atlas_uniform_mvp;
    GLint atlas_uniform_texture;
    GLint atlas_attribute_pos;
    GLint atlas_attribute_uv;
    GLint atlas_attribute_col;
    //u32 texture_handle; //TODO c'est chiant, ça veut dire qu'on l'a à deux endroits séparés ????
} OpenGL_Context_Atlas;

typedef struct{
    u32 ir_integrate_shader_program;
    
    u32 ir_integrate_vao;
    u32 ir_integrate_vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 ir_integrate_min_max_buffer;
    u32 ir_integrate_min_max_texture;
    
    GLint ir_integrate_uniform_mvp;
    GLint ir_integrate_attribute_pos;
    GLint ir_integrate_attribute_quad_pos;
    
    Vec2 *ir_integrate_temp_buffer;
    //u32 previous_pixel_length; TODO cache
    
    u32 ir_interpolate_shader_program;
    
    u32 ir_interpolate_vao;
    u32 ir_interpolate_vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 ir_interpolate_buffer;
    u32 ir_interpolate_texture;
    
    GLint ir_interpolate_uniform_mvp;
    GLint ir_interpolate_attribute_pos;
    GLint ir_interpolate_attribute_quad_pos;
} OpenGL_Context_IR;

typedef struct {
    u32 fft_shader_program;
    
    u32 fft_vao;
    u32 fft_vertex_buffer; //NOTE y a que 6 vertex ptdr
    u32 fft_data_buffer;
    u32 fft_data_texture;
    
    GLint fft_uniform_mvp;
    GLint fft_attribute_pos;
    GLint fft_attribute_quad_pos;
    
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

typedef GLuint(*glCreateProgram_t) (void);
static glCreateProgram_t glCreateProgram = nullptr;

typedef GLuint(*glCreateShader_t)(GLenum);
static glCreateShader_t glCreateShader = nullptr;

typedef void (*glDeleteProgram_t) (GLuint program);
static glDeleteProgram_t  glDeleteProgram = nullptr;

typedef void (*glDeleteShader_t) (GLuint shader);
static glDeleteShader_t  glDeleteShader = nullptr;

typedef void (*glCompileShader_t) (GLuint shader);
static glCompileShader_t  glCompileShader = nullptr;

typedef void (*glLinkProgram_t) (GLuint program);
static glLinkProgram_t  glLinkProgram = nullptr;

typedef void (*glGenBuffers_t) (GLsizei n, GLuint *buffers);
static glGenBuffers_t  glGenBuffers = nullptr;

typedef void (*glBindBuffer_t) (GLenum target, GLuint buffer);
static glBindBuffer_t  glBindBuffer = nullptr;

typedef void (*glShaderSource_t)(GLuint shader, GLsizei count, const char *const* string, const GLint *length);
static glShaderSource_t glShaderSource = nullptr;

typedef void (*glBufferData_t) (GLenum target, i64 size, const void *data, GLenum usage);
static glBufferData_t  glBufferData = nullptr;

typedef void(*glAttachShader_t) (GLuint program, GLuint shader);
static glAttachShader_t glAttachShader = nullptr;

typedef void (*glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
static glGenVertexArrays_t glGenVertexArrays = nullptr;

typedef void (*glBindVertexArray_t) (GLuint array);
static glBindVertexArray_t glBindVertexArray = nullptr;

typedef void (*glUseProgram_t) (GLuint program);
static glUseProgram_t glUseProgram = nullptr;

typedef void (*glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
static glVertexAttribPointer_t glVertexAttribPointer = nullptr;

typedef void (*glEnableVertexAttribArray_t) (GLuint index);
static glEnableVertexAttribArray_t glEnableVertexAttribArray = nullptr;

typedef GLint (*glGetUniformLocation_t)(GLuint program, const char *name);
static glGetUniformLocation_t glGetUniformLocation = nullptr;

typedef void (*glUniformMatrix4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
static glUniformMatrix4fv_t glUniformMatrix4fv = nullptr;


typedef void (*glGetShaderiv_t)(GLuint shader, GLenum pname, GLint *params);
static glGetShaderiv_t glGetShaderiv = nullptr;


typedef void (*glGetProgramiv_t)(GLuint program, GLenum pname, GLint *params);
static glGetProgramiv_t glGetProgramiv = nullptr;

typedef BOOL(*wglSwapIntervalEXT_t)(int interval);
static wglSwapIntervalEXT_t wglSwapIntervalEXT = nullptr;

typedef int (*wglGetSwapIntervalEXT_t)(void);
static wglGetSwapIntervalEXT_t wglGetSwapIntervalEXT = nullptr;

typedef GLint (*glGetAttribLocation_t)(GLuint program, const char *name);
static glGetAttribLocation_t glGetAttribLocation = nullptr;

typedef void (*glUniform1i_t) (GLint location, GLint v0);
static glUniform1i_t glUniform1i = nullptr; 

typedef void (*glGetProgramInfoLog_t) (GLuint program, GLsizei bufSize, GLsizei *length, char*infoLog);
static glGetProgramInfoLog_t glGetProgramInfoLog = nullptr;

typedef void (*glTexBuffer_t)(GLenum target, GLenum internalformat, GLuint buffer);
static glTexBuffer_t glTexBuffer = nullptr;

typedef void (*glBufferSubData_t)(GLenum target, i32 *offset, i64 size, const void *data);
static glBufferSubData_t glBufferSubData = nullptr;

typedef void (*glActiveTexture_t)(GLenum texture);
static glActiveTexture_t glActiveTexture = nullptr;

typedef HGLRC (*wglCreateContextAttribsARB_t) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef  void (*glTexBufferRange_t)(GLenum target​, GLenum internalFormat​, GLuint buffer​, i64 offset​, i64 size​);
static glTexBufferRange_t glTexBufferRange = nullptr;

#define LOAD_GL(function_name) function_name = (function_name##_t) wglGetProcAddress(#function_name); if(!function_name) { printf("failed to load" #function_name "\n"); return false; }

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
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glCreateProgram);
    LOAD_GL(glCreateShader);
    LOAD_GL(glDeleteProgram);
    LOAD_GL(glDeleteShader);
    LOAD_GL(glCompileShader);
    LOAD_GL(glLinkProgram);
    LOAD_GL(glGenBuffers);
    LOAD_GL(glBindBuffer);
    LOAD_GL(glShaderSource);
    LOAD_GL(glBufferData);
    LOAD_GL(glAttachShader);
    LOAD_GL(glGenVertexArrays);
    LOAD_GL(glBindVertexArray);
    LOAD_GL(glUseProgram);
    LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glGetUniformLocation);
    LOAD_GL(glUniformMatrix4fv);
    LOAD_GL(glGetShaderiv );
    LOAD_GL(glGetProgramiv);
    LOAD_GL(wglSwapIntervalEXT);
    LOAD_GL(wglGetSwapIntervalEXT);
    LOAD_GL(glGetAttribLocation);
    LOAD_GL(glUniform1i);
    LOAD_GL(glGetProgramInfoLog);
    LOAD_GL(glBufferSubData);
    LOAD_GL(glTexBuffer);
    LOAD_GL(glActiveTexture);
    LOAD_GL(glTexBufferRange);
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
    assert(status == GL_TRUE);
    
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
    assert(status == GL_TRUE);
    
    const u32 atlas_shader_program = glCreateProgram();
    glAttachShader(atlas_shader_program, atlas_vertex_shader_handle);
    glAttachShader(atlas_shader_program, atlas_fragment_shader_handle);
    glLinkProgram(atlas_shader_program);
    glGetProgramiv(atlas_shader_program, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
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
        .atlas_shader_program = atlas_shader_program, 
        .atlas_vao = atlas_vao, 
        .atlas_vertex_buffer = atlas_vertex_buffer, 
        .atlas_index_buffer= atlas_index_buffer,
        
        .atlas_uniform_mvp = atlas_uniform_mvp, 
        .atlas_uniform_texture = atlas_uniform_texture,
        .atlas_attribute_pos = atlas_attribute_pos,
        .atlas_attribute_uv = atlas_attribute_uv,
        .atlas_attribute_col = atlas_attribute_col,
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
    assert(status == GL_TRUE);
    
    
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
    assert(status == GL_TRUE);
    
    const u32 ir_integrate_shader_program = glCreateProgram();
    glAttachShader(ir_integrate_shader_program, ir_vertex_shader_handle);
    glAttachShader(ir_integrate_shader_program, ir_integrate_fragment_shader_handle);
    glLinkProgram(ir_integrate_shader_program);
    glGetProgramiv(ir_integrate_shader_program, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
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
    assert(status == GL_TRUE);
    
    const u32 ir_interpolate_shader_program = glCreateProgram();
    glAttachShader(ir_interpolate_shader_program, ir_vertex_shader_handle);
    glAttachShader(ir_interpolate_shader_program, ir_interpolate_fragment_shader_handle);
    glLinkProgram(ir_interpolate_shader_program);
    glGetProgramiv(ir_interpolate_shader_program, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
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
        
        .ir_integrate_shader_program = ir_integrate_shader_program,
        .ir_integrate_vao = ir_integrate_vao, 
        .ir_integrate_vertex_buffer = ir_integrate_vertex_buffer, 
        .ir_integrate_min_max_buffer = ir_integrate_min_max_buffer,
        .ir_integrate_min_max_texture = ir_integrate_min_max_texture,
        
        .ir_integrate_uniform_mvp = ir_integrate_uniform_mvp, 
        .ir_integrate_attribute_pos = ir_integrate_attribute_pos,
        .ir_integrate_attribute_quad_pos = ir_integrate_attribute_quad_pos,
        
        .ir_integrate_temp_buffer = m_allocate_array(Vec2, IR_MAX_PIXEL_WIDTH),
        
        .ir_interpolate_shader_program = ir_interpolate_shader_program,
        .ir_interpolate_vao = ir_interpolate_vao, 
        .ir_interpolate_vertex_buffer = ir_interpolate_vertex_buffer, 
        .ir_interpolate_buffer = ir_interpolate_buffer,
        .ir_interpolate_texture = ir_interpolate_texture,
        
        .ir_interpolate_uniform_mvp = ir_interpolate_uniform_mvp, 
        .ir_interpolate_attribute_pos = ir_interpolate_attribute_pos,
        .ir_interpolate_attribute_quad_pos = ir_interpolate_attribute_quad_pos,
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
	assert(status == GL_TRUE);
    
    
    
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
	assert(status == GL_TRUE);
    
    const u32 fft_shader_program = glCreateProgram();
    glAttachShader(fft_shader_program, fft_vertex_shader_handle);
    glAttachShader(fft_shader_program, fft_fragment_shader_handle);
    glLinkProgram(fft_shader_program);
    glGetProgramiv(fft_shader_program, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
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
        .fft_shader_program = fft_shader_program,
        .fft_vao = fft_vao, 
        .fft_vertex_buffer = fft_vertex_buffer, 
        .fft_data_buffer = fft_data_buffer,
        .fft_data_texture = fft_data_texture,
        
        .fft_uniform_mvp = fft_uniform_mvp, 
        .fft_attribute_pos = fft_attribute_pos,
        .fft_attribute_quad_pos = fft_attribute_quad_pos,
    };
}

OpenGL_Context opengl_initialize(Window_Context *window, Font* font)
{
    
    OpenGL_Context_Win32 ctx_win32 = opengl_initialize_win32(window->window);
    OpenGL_Context_Atlas ctx_atlas = opengl_initialize_atlas(font);
    OpenGL_Context_IR ctx_ir = opengl_initialize_ir();
    OpenGL_Context_FFT ctx_fft = opengl_initialize_fft();
    
    
    m_free(font->atlas_pixels);
    font->atlas_pixels = nullptr;
    
    ReleaseDC(window->window, ctx_win32.window_dc);
    
    return OpenGL_Context{
        .win32 = ctx_win32,
        .atlas = ctx_atlas,
        .ir = ctx_ir,
        .fft = ctx_fft
    };
}

void opengl_render_atlas(OpenGL_Context_Atlas* opengl_ctx,
                         Graphics_Context_Atlas* graphics_ctx, real32 *projection_matrix)
{
    glUseProgram(opengl_ctx->atlas_shader_program);
    glBindVertexArray(opengl_ctx->atlas_vao);
    glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->atlas_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_ctx->atlas_index_buffer);
    glBindTexture(GL_TEXTURE_2D, graphics_ctx->font.atlas_texture_id);
    
    //TODO est-ce que je peux supprimer ça ?
    //glUniform1i(opengl_ctx->atlas_uniform_texture, 0);
    glUniformMatrix4fv(opengl_ctx->atlas_uniform_mvp, 
                       1, 
                       GL_FALSE, 
                       projection_matrix);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    graphics_ctx->draw_vertices_count * sizeof(Vertex), 
                    graphics_ctx->draw_vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 
                    graphics_ctx->draw_indices_count * sizeof(u32), 
                    graphics_ctx->draw_indices);
    
    glDrawElements(GL_TRIANGLES, (GLsizei)graphics_ctx->draw_indices_count, GL_UNSIGNED_INT, 0);
}


void opengl_render_ir(OpenGL_Context_IR* opengl_ctx, Graphics_Context_IR* graphics_ctx, real32 *projection_matrix)
{
    Rect bounds = rect_remove_padding(graphics_ctx->bounds, 1.0f, 1.0f);
    
    Vec2 top_left = bounds.origin;
    Vec2 top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    Vec2 bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    Vec2 bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 uv_top_left = Vec2{ 0.0f, 1.0f};
    Vec2 uv_top_right = Vec2{ 1.0f, 1.0f};
    Vec2 uv_bottom_left = Vec2{ 0.0f, 0.0f};
    Vec2 uv_bottom_right = Vec2{ 1.0f, 0.0f};
    
    IR_Vertex ir_vertices[6];
    
    ir_vertices[0] = { bottom_left,  uv_bottom_left};
    ir_vertices[1] = { top_right,    uv_top_right};
    ir_vertices[2] = { top_left,     uv_top_left};
    ir_vertices[3] = { bottom_left,  uv_bottom_left};
    ir_vertices[4] = { bottom_right, uv_bottom_right};
    ir_vertices[5] = { top_right,    uv_top_right};
    
    
    i32 pixel_count = bounds.dim.x;
    i32 sample_count = IR_BUFFER_LENGTH;
    assert(pixel_count >= 0);
    
    Vec2 *temp = opengl_ctx->ir_integrate_temp_buffer;
    
    if(bounds.dim.x < graphics_ctx->IR_sample_count)
    {
        
        //memset(temp, 0, pixel_count * sizeof(Vec2));
        for(u32 pixel_idx = 0; pixel_idx < pixel_count; pixel_idx++)
        {
            temp[pixel_idx] = {.a = -1.0f, .b = 1.0f};
        }
        for(u32 sample = 0; sample < sample_count; sample++)
        {
            real32 val = graphics_ctx->IR_buffer/*[channel]*/[sample];
            u32 pixel_idx  = sample * pixel_count / sample_count;
            temp[pixel_idx].a = octave_max(temp[pixel_idx].a, val);
            temp[pixel_idx].b = octave_min(temp[pixel_idx].b, val);
        } 
        
        
        
        glUseProgram(opengl_ctx->ir_integrate_shader_program);
        glBindVertexArray(opengl_ctx->ir_integrate_vao);
        glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->ir_integrate_vertex_buffer);
        glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->ir_integrate_min_max_buffer);
        glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->ir_integrate_min_max_texture);
        
        glUniformMatrix4fv(opengl_ctx->ir_integrate_uniform_mvp, 
                           1, 
                           GL_FALSE, 
                           projection_matrix);
        
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        6 * sizeof(IR_Vertex),
                        ir_vertices);
        
        glTexBufferRange(GL_TEXTURE_BUFFER, GL_RG32F, opengl_ctx->ir_integrate_min_max_buffer, 0, sizeof(Vec2) * bounds.dim.x);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                        bounds.dim.x * sizeof(Vec2), 
                        temp);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    }
    else {
        glUseProgram(opengl_ctx->ir_interpolate_shader_program);
        glBindVertexArray(opengl_ctx->ir_interpolate_vao);
        glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->ir_interpolate_vertex_buffer);
        glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->ir_interpolate_buffer);
        glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->ir_interpolate_texture);
        
        glUniformMatrix4fv(opengl_ctx->ir_interpolate_uniform_mvp, 
                           1, 
                           GL_FALSE, 
                           projection_matrix);
        
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                        6 * sizeof(IR_Vertex),
                        ir_vertices);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                        graphics_ctx->IR_sample_count * sizeof(real32), 
                        graphics_ctx->IR_buffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    }
}

void opengl_render_fft(OpenGL_Context_FFT* opengl_ctx, Graphics_Context_FFT* graphics_ctx, real32 *projection_matrix)
{
    Rect bounds = rect_remove_padding(graphics_ctx->bounds, 1.0f, 1.0f);
    
    Vec2 top_left = bounds.origin;
    Vec2 top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    Vec2 bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    Vec2 bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 uv_top_left = Vec2{ 0.0f, 1.0f};
    Vec2 uv_top_right = Vec2{ 1.0f, 1.0f};
    Vec2 uv_bottom_left = Vec2{ 0.0f, 0.0f};
    Vec2 uv_bottom_right = Vec2{ 1.0f, 0.0f};
    
    IR_Vertex ir_vertices[6];
    
    ir_vertices[0] = { bottom_left,  uv_bottom_left};
    ir_vertices[1] = { top_right,    uv_top_right};
    ir_vertices[2] = { top_left,     uv_top_left};
    ir_vertices[3] = { bottom_left,  uv_bottom_left};
    ir_vertices[4] = { bottom_right, uv_bottom_right};
    ir_vertices[5] = { top_right,    uv_top_right};
    
    
    glUseProgram(opengl_ctx->fft_shader_program);
    glBindVertexArray(opengl_ctx->fft_vao);
    glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->fft_vertex_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, opengl_ctx->fft_data_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, opengl_ctx->fft_data_texture);
    
    
    glUniformMatrix4fv(opengl_ctx->fft_uniform_mvp, 
                       1, 
                       GL_FALSE, 
                       projection_matrix);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    6 * sizeof(IR_Vertex),
                    ir_vertices);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, 
                    graphics_ctx->fft_sample_count * sizeof(real32), 
                    graphics_ctx->fft_buffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void opengl_render_generic(OpenGL_Context *opengl_ctx, Graphics_Context *graphics_ctx)
{
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    glViewport(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    glScissor(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    real32 projection_matrix[16] = 
    {
        2.0f / graphics_ctx->window_dim.x, 0.0f, 0.0f, 0.0f,
        0.0f,- 2.0f / graphics_ctx->window_dim.y, 0.0f, 0.0f ,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };
    
    opengl_render_atlas(&opengl_ctx->atlas, &graphics_ctx->atlas, projection_matrix);
    
    SwapBuffers(opengl_ctx->win32.window_dc);
    //glFinish();
}

void opengl_render_ui(OpenGL_Context *opengl_ctx, Graphics_Context *graphics_ctx)
{
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    glViewport(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    glScissor(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    real32 projection_matrix[16] = 
    {
        2.0f / graphics_ctx->window_dim.x, 0.0f, 0.0f, 0.0f,
        0.0f,- 2.0f / graphics_ctx->window_dim.y, 0.0f, 0.0f ,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };
    
    opengl_render_atlas(&opengl_ctx->atlas, &graphics_ctx->atlas, projection_matrix);
    opengl_render_ir(&opengl_ctx->ir, &graphics_ctx->ir, projection_matrix);
    opengl_render_fft(&opengl_ctx->fft, &graphics_ctx->fft, projection_matrix);
    
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
