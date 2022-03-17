OPENGL_PROC( GLuint, glCreateProgram, (void));

OPENGL_PROC( GLuint, glCreateShader, (GLenum));

OPENGL_PROC( void, glDeleteProgram, (GLuint program));

OPENGL_PROC( void, glDeleteShader, (GLuint shader));

OPENGL_PROC( void, glCompileShader, (GLuint shader));

OPENGL_PROC( void, glLinkProgram, (GLuint program));

OPENGL_PROC( void, glGenBuffers, (GLsizei n, GLuint *buffers));

OPENGL_PROC( void, glBindBuffer, (GLenum target, GLuint buffer));

OPENGL_PROC( void, glShaderSource,(GLuint shader, GLsizei count, const char *const* string, const GLint *length));

OPENGL_PROC( void, glBufferData, (GLenum target, i64 size, const void *data, GLenum usage));

OPENGL_PROC( void, glAttachShader, (GLuint program, GLuint shader));

OPENGL_PROC( void, glGenVertexArrays, (GLsizei n, GLuint *arrays));

OPENGL_PROC( void, glBindVertexArray, (GLuint array));

OPENGL_PROC( void, glUseProgram, (GLuint program));

OPENGL_PROC( void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer));

OPENGL_PROC( void, glEnableVertexAttribArray, (GLuint index));

OPENGL_PROC( GLint, glGetUniformLocation,(GLuint program, const char *name));

OPENGL_PROC( void, glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));

OPENGL_PROC( void, glGetShaderiv,(GLuint shader, GLenum pname, GLint *params));

OPENGL_PROC( void, glGetProgramiv,(GLuint program, GLenum pname, GLint *params));

OPENGL_PROC( BOOL, wglSwapIntervalEXT,(int interval));

OPENGL_PROC( int, wglGetSwapIntervalEXT,(void));

OPENGL_PROC( GLint, glGetAttribLocation,(GLuint program, const char *name));

OPENGL_PROC( void, glUniform1i, (GLint location, GLint v0));

OPENGL_PROC( void, glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, char*infoLog));

OPENGL_PROC( void, glTexBuffer,(GLenum target, GLenum internalformat, GLuint buffer));

OPENGL_PROC( void, glBufferSubData,(GLenum target, i32 *offset, i64 size, const void *data));

OPENGL_PROC( void, glActiveTexture,(GLenum texture));

OPENGL_PROC( HGLRC, wglCreateContextAttribsARB, (HDC hDC, HGLRC hShareContext, const int *attribList));

OPENGL_PROC( void, glTexBufferRange,(GLenum target, GLenum internalFormat, GLuint buffer, i64 offset, i64 size));
