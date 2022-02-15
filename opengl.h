/* date = February 1st 2022 8:38 am */

#ifndef OPENGL_H
#define OPENGL_H

#include <GL/GL.h>

//TODO définir un point blanc dans la texture !!!!!
#define DRAW_TEX_UV_FOR_WHITE	Vec{0.0f, 0.0f}

typedef struct {
    u32 shader_program;
    
    GLint uniform_mvp;
    GLint uniform_texture;
    GLint attribute_pos;
    GLint attribute_uv;
    GLint attribute_col;
    
    u32 VAO;
    u32 VBO; //TODO rename
    u32 element_handle;
    //u32 texture_handle; //TODO c'est chiant, ça veut dire qu'on l'a à deux endroits séparés ????
    
    HDC window_dc;
    HGLRC opengl_rc;
} OpenGL_Context;

//~ OpenGL defines
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
//TODO est-ce qu'on a vraiment besoin de ça ? on a pas tant de triangles que ça à priori
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

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
    
    return true;
}

OpenGL_Context opengl_initialize(HWND window, Font* font)
{
    
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
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    wglSwapIntervalEXT(1);
    printf("swap interval : %d\n", wglGetSwapIntervalEXT());
	GLint status = GL_TRUE;
    
    const char *vertexShaderSource = 
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
    
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);
    
    const char *fragmentShaderSource =  
        "#version 330 core\n"
        "in vec2 frag_uv;\n"
        "in vec4 frag_col;\n"
        "uniform sampler2D texture;\n"
        "layout (location = 0) out vec4 out_col;\n"
        "void main()\n"
        "{\n"
        "    out_col = frag_col * texture(texture, frag_uv.st);\n"
        "}\n";
    
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
    opengl_check_error();
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    GLint uniform_mvp = glGetUniformLocation(shader_program, "mvp");
    GLint uniform_texture = glGetUniformLocation(shader_program, "texture");
    
    GLint attribute_pos = glGetAttribLocation(shader_program, "pos");
    GLint attribute_uv = glGetAttribLocation(shader_program, "uv");	
    GLint attribute_col = (GLuint)glGetAttribLocation(shader_program, "col");
    opengl_check_error();
    
    u32 VBO;
    u32 VAO;
    u32 element_handle;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &element_handle);
    
    //TODO dans quel sens je dois faire ça ?
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glGenTextures(1, &font->atlas_texture_id);
    glBindTexture(GL_TEXTURE_2D, font->atlas_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 font->atlas_texture_dim.x, font->atlas_texture_dim.y, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, font->atlas_pixels);
    /*
    m_freee(font->atlas_pixels);
    font->atlas_pixels = nullptr;
    */
    ReleaseDC(window, window_dc);
    
    return {
        .shader_program = shader_program, 
        .uniform_mvp = uniform_mvp, 
        .uniform_texture = uniform_texture,
        .attribute_pos = attribute_pos,
        .attribute_uv = attribute_uv,
        .attribute_col = attribute_col,
        .VAO = VAO, 
        .VBO = VBO, 
        .element_handle = element_handle,
        .window_dc = window_dc, 
        .opengl_rc = opengl_rc,
    };
}

void opengl_render_ui(OpenGL_Context *opengl_ctx, GraphicsContext *graphics_ctx)
{
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    glViewport(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    glScissor(0,0, graphics_ctx->window_dim.x, graphics_ctx->window_dim.y);
    
    float projection_matrix[16] = 
    {
        2.0f / graphics_ctx->window_dim.x, 0.0f, 0.0f, 0.0f,
        0.0f,- 2.0f / graphics_ctx->window_dim.y, 0.0f, 0.0f ,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };
    
    
    glUseProgram(opengl_ctx->shader_program);
    glUniform1i(opengl_ctx->uniform_texture, 0);
    glUniformMatrix4fv(opengl_ctx->uniform_mvp, 
                       1, 
                       GL_FALSE, 
                       &projection_matrix[0]);
    
    
    glBindVertexArray(opengl_ctx->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, opengl_ctx->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_ctx->element_handle);
    glBindTexture(GL_TEXTURE_2D, graphics_ctx->font->atlas_texture_id);
    
    
    glEnableVertexAttribArray(opengl_ctx->attribute_pos);
    glEnableVertexAttribArray(opengl_ctx->attribute_uv);
    glEnableVertexAttribArray(opengl_ctx->attribute_col);
    
    glVertexAttribPointer(opengl_ctx->attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OffsetOf(Vertex, pos));
    glVertexAttribPointer(opengl_ctx->attribute_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OffsetOf(Vertex, uv));
    glVertexAttribPointer(opengl_ctx->attribute_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)OffsetOf(Vertex, col));
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawElements(GL_TRIANGLES, (GLsizei)graphics_ctx->draw_indices_count, GL_UNSIGNED_INT, NULL);
    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex2f(-2, -1);
    glVertex2f(1, -1);
    glVertex2f(0, 2);
    
    glVertex2f(0, 0);
    glVertex2f(100, 100);
    glVertex2f(0, 100);
    
    glEnd();
    
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    
    SwapBuffers(opengl_ctx->window_dc);
    
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
    wglDeleteContext (opengl_ctx->opengl_rc);
}
#endif //OPENGL_H
