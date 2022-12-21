/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */

#pragma once

#include <gl/GL.h>

#ifdef MAINCODE
#define EXTERN(x,y) x y=NULL
#else
#define EXTERN(x,y) extern x y
#endif

 //OpenGL function pointers. Loaded by rGL_Load from the original executable. Welcome to suffering.
typedef void(__stdcall* glAlphaFunc_fp)(GLenum func, GLclampf ref);
typedef void(__stdcall* glBegin_fp)(GLenum mode);
typedef void(__stdcall* glBindTexture_fp)(GLenum target, GLuint texture);
typedef void(__stdcall* glBlendFunc_fp)(GLenum sfactor, GLenum dfactor);
typedef void(__stdcall* glClear_fp)(GLbitfield mask);
typedef void(__stdcall* glClearColor_fp)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void(__stdcall* glClearDepth_fp)(GLclampd depth);
typedef void(__stdcall* glColor3ub_fp)(GLubyte red, GLubyte green, GLubyte blue);
typedef void(__stdcall* glColor4ub_fp)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void(__stdcall* glColor4f_fp)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void(__stdcall* glColorPointer_fp)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void(__stdcall* glDeleteTextures_fp) (GLsizei n, const GLuint* textures);
typedef void(__stdcall* glDepthFunc_fp)(GLenum func);
typedef void(__stdcall* glDepthMask_fp)(GLboolean flag);
typedef void(__stdcall* glDepthRange_fp)(GLclampd zNear, GLclampd zFar);
typedef void(__stdcall* glDisable_fp)(GLenum cap);
typedef void(__stdcall* glDisableClientState_fp)(GLenum array);
typedef void(__stdcall* glDrawArrays_fp)(GLenum mode, GLint first, GLsizei count);
typedef void(__stdcall* glDrawPixels_fp)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void(__stdcall* glEnable_fp)(GLenum cap);
typedef void(__stdcall* glEnableClientState_fp)(GLenum array);
typedef void(__stdcall* glEnd_fp)(void);
typedef void(__stdcall* glFogf_fp)(GLenum pname, GLfloat param);
typedef void(__stdcall* glFogfv_fp)(GLenum pname, const GLfloat* params);
typedef void(__stdcall* glFogi_fp)(GLenum pname, GLint param);
typedef const GLubyte* (__stdcall* glGetString_fp)(GLenum name);
typedef void(__stdcall* glHint_fp)(GLenum target, GLenum mode);
typedef void(__stdcall* glLoadIdentity_fp)(void);
typedef void(__stdcall* glMatrixMode_fp)(GLenum mode);
typedef void(__stdcall* glOrtho_fp)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef void(__stdcall* glPixelStorei_fp)(GLenum pname, GLint param);
typedef void(__stdcall* glPixelTransferi_fp)(GLenum pname, GLint param);
typedef void(__stdcall* glPolygonOffset_fp)(GLfloat factor, GLfloat units);
typedef void(__stdcall* glReadPixels_fp)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
typedef void(__stdcall* glScissor_fp)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void(__stdcall* glShadeModel_fp)(GLenum mode);
typedef void(__stdcall* glTexCoordPointer_fp)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void(__stdcall* glTexEnvf_fp)(GLenum target, GLenum pname, GLfloat param);
typedef void(__stdcall* glTexImage2D_fp)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void(__stdcall* glTexParameteri_fp)(GLenum target, GLenum pname, GLint param);
typedef void(__stdcall* glTexSubImage2D_fp)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void(__stdcall* glVertex2i_fp)(GLint, GLint);
typedef void(__stdcall* glVertex3f_fp)(GLfloat, GLfloat, GLfloat);
typedef void(__stdcall* glVertexPointer_fp)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void(__stdcall* glViewport_fp)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef HGLRC(__stdcall* wglCreateContext_fp)(HDC);
typedef BOOL(__stdcall* wglDeleteContext_fp)(HGLRC);
typedef BOOL(__stdcall* wglMakeCurrent_fp)(HDC, HGLRC);
typedef PROC(__stdcall* wglGetProcAddress_fp)(LPCSTR);
//New functions for windowed mode
typedef void(__stdcall* glFramebufferTexture2d_fp)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void(__stdcall* glGenFramebuffers_fp)(GLsizei n, GLuint* ids);
typedef void(__stdcall* glDeleteFramebuffers_fp)(GLsizei n, GLuint* framebuffers);
typedef void(__stdcall* glBindFramebuffer_fp)(GLenum target, GLuint framebuffer);
typedef GLenum(__stdcall* glCheckFramebufferStatus_fp)(GLenum target);
typedef GLenum(__stdcall* glGetError_fp)();
typedef void(__stdcall* glTexCoord2f_fp)(GLfloat s, GLfloat t);
typedef void (__stdcall* glBlitFramebuffer_fp) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (__stdcall* glTexImage2DMultisample_fp) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (__stdcall* glBlendFuncSeparate_fp) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (__stdcall* glActiveTexture_fp) (GLenum texture);

//Needed constants 
#define GL_MULTISAMPLE                    0x809D
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DEPTH32F_STENCIL8              0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT            0x8218
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#define GL_TEXTURE_RED_TYPE               0x8C10
#define GL_TEXTURE_GREEN_TYPE             0x8C11
#define GL_TEXTURE_BLUE_TYPE              0x8C12
#define GL_TEXTURE_ALPHA_TYPE             0x8C13
#define GL_TEXTURE_DEPTH_TYPE             0x8C16
#define GL_UNSIGNED_NORMALIZED            0x8C17
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
#define GL_RENDERBUFFER_SAMPLES           0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF

#define GL_UNSIGNED_BYTE_3_3_2_EXT        0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT     0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT     0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT       0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT    0x8036

EXTERN(wglCreateContext_fp, dwglCreateContext);
EXTERN(wglDeleteContext_fp, dwglDeleteContext);
EXTERN(wglMakeCurrent_fp, dwglMakeCurrent);
EXTERN(wglGetProcAddress_fp, dwglGetProcAddress);

EXTERN(glAlphaFunc_fp, dglAlphaFunc);
EXTERN(glBegin_fp, dglBegin);
EXTERN(glBindTexture_fp, dglBindTexture);
EXTERN(glBlendFunc_fp, dglBlendFunc);
EXTERN(glClear_fp, dglClear);
EXTERN(glClearColor_fp, dglClearColor);
EXTERN(glClearDepth_fp, dglClearDepth);
EXTERN(glColor3ub_fp, dglColor3ub);
EXTERN(glColor4ub_fp, dglColor4ub);
EXTERN(glColor4f_fp, dglColor4f);
EXTERN(glColorPointer_fp, dglColorPointer);
EXTERN(glDeleteTextures_fp, dglDeleteTextures);
EXTERN(glDepthFunc_fp, dglDepthFunc);
EXTERN(glDepthMask_fp, dglDepthMask);
EXTERN(glDepthRange_fp, dglDepthRange);
EXTERN(glDisable_fp, dglDisable);
EXTERN(glDisableClientState_fp, dglDisableClientState);
EXTERN(glDrawArrays_fp, dglDrawArrays);
EXTERN(glDrawPixels_fp, dglDrawPixels);
EXTERN(glEnable_fp, dglEnable);
EXTERN(glEnableClientState_fp, dglEnableClientState);
EXTERN(glEnd_fp, dglEnd);
EXTERN(glFogf_fp, dglFogf);
EXTERN(glFogfv_fp, dglFogfv);
EXTERN(glFogi_fp, dglFogi);
EXTERN(glGetString_fp, dglGetString);
EXTERN(glHint_fp, dglHint);
EXTERN(glLoadIdentity_fp, dglLoadIdentity);
EXTERN(glMatrixMode_fp, dglMatrixMode);
EXTERN(glOrtho_fp, dglOrtho);
EXTERN(glPixelStorei_fp, dglPixelStorei);
EXTERN(glPixelTransferi_fp, dglPixelTransferi);
EXTERN(glPolygonOffset_fp, dglPolygonOffset);
EXTERN(glReadPixels_fp, dglReadPixels);
EXTERN(glScissor_fp, dglScissor);
EXTERN(glShadeModel_fp, dglShadeModel);
EXTERN(glTexCoordPointer_fp, dglTexCoordPointer);
EXTERN(glTexEnvf_fp, dglTexEnvf);
EXTERN(glTexImage2D_fp, dglTexImage2D);
EXTERN(glTexParameteri_fp, dglTexParameteri);
EXTERN(glTexSubImage2D_fp, dglTexSubImage2D);
EXTERN(glVertex2i_fp, dglVertex2i);
EXTERN(glVertex3f_fp, dglVertex3f);
EXTERN(glVertexPointer_fp, dglVertexPointer);
EXTERN(glViewport_fp, dglViewport);
EXTERN(glFramebufferTexture2d_fp, dglFramebufferTexture2d);
EXTERN(glGenFramebuffers_fp, dglGenFramebuffers);
EXTERN(glDeleteFramebuffers_fp, dglDeleteFramebuffers);
EXTERN(glBindFramebuffer_fp, dglBindFramebuffer);
EXTERN(glCheckFramebufferStatus_fp, dglCheckFramebufferStatus);
EXTERN(glGetError_fp, dglGetError);//glTexCoord2f_fp
EXTERN(glTexCoord2f_fp, dglTexCoord2f);
EXTERN(glBlitFramebuffer_fp, dglBlitFramebuffer);
EXTERN(glTexImage2DMultisample_fp, dglTexImage2DMultisample);
EXTERN(glBlendFuncSeparate_fp, dglBlendFuncSeparate);
EXTERN(glActiveTexture_fp, dglActiveTexture);
