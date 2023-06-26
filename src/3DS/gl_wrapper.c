#include "gl_wrapper.h"

#include <citro3d.h>

//========== GRAPHICS FUNCTIONS ==========

void glEnable(GLenum cap) {

}

void glDisable(GLenum cap) {

}

void glCullFace(GLenum mode) {

}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {

}

void glDepthFunc(GLenum func) {

}

void glDepthMask(GLboolean flag) {

}

void glAlphaFunc(GLenum func, GLclampf ref) {

}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {

}

void glClearDepth(GLclampd depth) {

}

void glClear(GLbitfield mask) {

}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {

}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {

}

void glBegin(GLenum mode) {

}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {

}

void glColor4ubv(const GLubyte *v) {

}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {

}

void glTexCoord2f(GLfloat s, GLfloat t) {

}

void glTexCoord2fv(const GLfloat *v) {

}

void glVertex2i(GLint x, GLint y) {

}

void glVertex2f(GLfloat x, GLfloat y) {

}

void glVertex3f(GLfloat x,GLfloat y,GLfloat z) {

}

void glVertex3fv(const GLfloat *v) {

}

void glEnd(void) {

}

void glGenTextures(GLsizei n, GLuint *textures) {

}

void glBindTexture(GLenum target, GLuint texture) {

}

void glTexParameteri(GLenum target, GLenum pname, GLint param) {

}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {

}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {

}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {

}

void glDeleteTextures(GLsizei n, const GLuint *textures) {

}

void glFogi(GLenum pname, GLint param) {

}

void glFogf(GLenum pname, GLfloat param) {

}

void glFogfv(GLenum pname, const GLfloat *params) {

}

void glTexEnvi(GLenum target, GLenum pname, GLint param) {

}

void glGetIntegerv(GLenum pname, GLint *params) {

}

void glGetFloatv(GLenum pname, GLfloat *params) {

}

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params) {

}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {

}

void glFlush(void) {

}

void glFinish(void) {

}


//========== MATRIX FUNCTIONS ==========

void glMatrixMode(GLenum mode) {

}

void glLoadIdentity(void) {

}

void glLoadMatrixf(const GLfloat *m) {

}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {

}

void glPushMatrix(void) {

}

void glPopMatrix(void) {

}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {

}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {

}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {

}
