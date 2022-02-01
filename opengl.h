/* date = February 1st 2022 8:38 am */

#ifndef OPENGL_H
#define OPENGL_H

#include <GL/GL.h>

typedef struct {
    u32 shader_program;
    GLint uniform_mvp;
    u32 VBO;
    u32 VAO;
    HDC window_dc;
    HGLRC opengl_rc;
    float mvp[4][4];
} OpenGL_Context;

//~ OpenGL defines
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

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

#define LOAD_GL(function_name) function_name = (function_name##_t) wglGetProcAddress(#function_name); if(!function_name) { printf("failed to load" #function_name "\n"); return false; }

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
    
    return true;
}

OpenGL_Context opengl_initialize(HWND window)
{
    
    //we don't use the pixel buffer, we've moved to triangle/shader/core profile rendering
    /*
    u32 pixel_buffer_size = window_height * window_width * sizeof(u8) * 4;
    u8* pixel_buffer = (u8*) malloc(pixel_buffer_size);
    memset(pixel_buffer, 1, pixel_buffer_size);
    */
    
    HDC window_dc = GetDC(window);
    
    
    PIXELFORMATDESCRIPTOR pixel_format =
    {
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
    
    int pixel_format_idx = ChoosePixelFormat(window_dc, &pixel_format);
    if(pixel_format_idx == 0)
    {
        printf("couldn't find a valid pixel format\n");
        exit(1);
    }
    
    SetPixelFormat(window_dc, pixel_format_idx, &pixel_format);
    
    HGLRC opengl_rc = wglCreateContext(window_dc);
    if(wglMakeCurrent(window_dc, opengl_rc) == FALSE)
    {
        printf("couldn't open opengl context\n");
        exit(1);
    }
    
    
    if(!load_opengl_functions())
    {
        printf("failed to load opengl functions\n");
        exit(1);
    }
    
    wglSwapIntervalEXT(1);
    printf("swap interval : %d\n", wglGetSwapIntervalEXT());
	GLint status = GL_TRUE;
    
    const char *vertexShaderSource = 
        "#version 330 core\n"
        "uniform mat4 MVP;"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position =  MVP * vec4(aPos.x, aPos.y, 1.0, 1.0);\n"
        "}\0";
    
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);
    
    const char *fragmentShaderSource =  
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "} \n";
    
    
	
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);
    
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragmentShader);
    glLinkProgram(shader_program);
    
	glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
	assert(status == GL_TRUE);
    
    GLint uniform_mvp;
	uniform_mvp = glGetUniformLocation(shader_program, "MVP");
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    
    
    
    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    assert(glGetError() == GL_NO_ERROR);
    
    ReleaseDC(window, window_dc);
    
    //TODO hack
    real32 window_width = 600.0f;
    real32 window_height = 400.0f;
    
    OpenGL_Context new_opengl_ctx = 
    {
        .shader_program = shader_program, 
        .uniform_mvp = uniform_mvp, 
        .VBO = VBO, 
        .VAO = VAO, 
        .window_dc = window_dc, 
        .opengl_rc = opengl_rc,
        .mvp = {
            { 2.0f/window_width,	0.0f,			0.0f,		0.0f },
            { 0.0f,			-2.0f/window_height,		0.0f,		0.0f },
            { 0.0f,			0.0f,			1.0f,		0.0f },
            { -1.0f, 1.0f,	0.0f,		1.0f },
        }
    };
    
    return new_opengl_ctx;
}


void opengl_render_ui(OpenGL_Context *opengl_ctx, GraphicsContext *graphics_ctx)
{
    //TODO hack
    real32 window_width = 600.0f;
    real32 window_height = 400.0f;
    
    glViewport(0,0, window_width, window_height);
    
    //NOTE texture stuff, we don't care anymore, unless it's going to be used in the core profile pipeline ????
    /*
    unsigned int texture = 0;
    static bool init = false;
    if(!init)
    {
        glGenTextures(1, &texture);
        init = true;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA4,
                 window_width,
                 window_height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixel_buffer
                 );
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    
    glEnable(GL_TEXTURE_2D);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    float projection_matrix[] = 
    {
        -1.0f / window_width, 0.0f, 0.0f, 0.0f,
        0.0f,- 1.0f / window_height, 0.0f, 0.0f ,
        0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
    };
    glLoadMatrixf(projection_matrix);
    */
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(opengl_ctx->shader_program);
    
    glUniformMatrix4fv(opengl_ctx->uniform_mvp, 1, GL_FALSE, &opengl_ctx->mvp[0][0]);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->VBO);
    glBindVertexArray(opengl_ctx->VAO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 4096 * 4, graphics_ctx->vertex_buffer, GL_STATIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, graphics_ctx->used_triangles * 3);
    
    SwapBuffers(opengl_ctx->window_dc);
    
    graphics_ctx->used_triangles = 0;
    
}

void opengl_uninitialize(OpenGL_Context *opengl_ctx)
{
    wglMakeCurrent (NULL, NULL) ; 
    wglDeleteContext (opengl_ctx->opengl_rc);
}
#endif //OPENGL_H
