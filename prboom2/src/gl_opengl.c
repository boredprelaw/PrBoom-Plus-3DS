/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Thanks Roman "Vortex" Marchenko
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <SDL.h>
#include "gl_opengl.h"

#include "doomtype.h"
#include "lprintf.h"

#define isExtensionSupported(ext) strstr(extensions, ext)

int gl_version;

static dboolean gl_compatibility_mode;

int GLEXT_CLAMP_TO_EDGE = GL_CLAMP;
int gl_max_texture_size = 0;

dboolean gl_ext_arb_vertex_buffer_object = false;

// cfg values
int gl_ext_arb_vertex_buffer_object_default;

/* VBO */
PFNGLGENBUFFERSARBPROC              GLEXT_glGenBuffersARB              = NULL;
PFNGLDELETEBUFFERSARBPROC           GLEXT_glDeleteBuffersARB           = NULL;
PFNGLBINDBUFFERARBPROC              GLEXT_glBindBufferARB              = NULL;
PFNGLBUFFERDATAARBPROC              GLEXT_glBufferDataARB              = NULL;

void gld_InitOpenGLVersion(void)
{
  int MajorVersion, MinorVersion;
  gl_version = OPENGL_VERSION_1_0;
  if (sscanf((const char*)glGetString(GL_VERSION), "%d.%d", &MajorVersion, &MinorVersion) == 2)
  {
    if (MajorVersion > 1)
    {
      gl_version = OPENGL_VERSION_2_0;
      if (MinorVersion > 0) gl_version = OPENGL_VERSION_2_1;
    }
    else
    {
      gl_version = OPENGL_VERSION_1_0;
      if (MinorVersion > 0) gl_version = OPENGL_VERSION_1_1;
      if (MinorVersion > 1) gl_version = OPENGL_VERSION_1_2;
      if (MinorVersion > 2) gl_version = OPENGL_VERSION_1_3;
      if (MinorVersion > 3) gl_version = OPENGL_VERSION_1_4;
      if (MinorVersion > 4) gl_version = OPENGL_VERSION_1_5;
    }
  }
}

void gld_InitOpenGL(dboolean compatibility_mode)
{
  GLenum texture;
  const char *extensions = (const char*)glGetString(GL_EXTENSIONS);

  gl_compatibility_mode = compatibility_mode;

  gld_InitOpenGLVersion();

  // VBO
#ifdef USE_VBO
  gl_ext_arb_vertex_buffer_object = gl_ext_arb_vertex_buffer_object_default &&
    isExtensionSupported("GL_ARB_vertex_buffer_object") != NULL;
  if (gl_ext_arb_vertex_buffer_object)
  {
    GLEXT_glGenBuffersARB = SDL_GL_GetProcAddress("glGenBuffersARB");
    GLEXT_glDeleteBuffersARB = SDL_GL_GetProcAddress("glDeleteBuffersARB");
    GLEXT_glBindBufferARB = SDL_GL_GetProcAddress("glBindBufferARB");
    GLEXT_glBufferDataARB = SDL_GL_GetProcAddress("glBufferDataARB");

    if (!GLEXT_glGenBuffersARB || !GLEXT_glDeleteBuffersARB ||
        !GLEXT_glBindBufferARB || !GLEXT_glBufferDataARB)
      gl_ext_arb_vertex_buffer_object = false;
  }
  if (gl_ext_arb_vertex_buffer_object)
    lprintf(LO_INFO,"using GL_ARB_vertex_buffer_object\n");
#else
  gl_ext_arb_vertex_buffer_object = false;
#endif

  // GL_CLAMP_TO_EDGE
  GLEXT_CLAMP_TO_EDGE = (gl_version >= OPENGL_VERSION_1_2 ? GL_CLAMP_TO_EDGE : GL_CLAMP);

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
  lprintf(LO_INFO,"GL_MAX_TEXTURE_SIZE=%i\n", gl_max_texture_size);

  if ((compatibility_mode) || (gl_version <= OPENGL_VERSION_1_1))
  {
    lprintf(LO_INFO, "gld_InitOpenGL: Compatibility mode is used.\n");
    gl_ext_arb_vertex_buffer_object = false;
    GLEXT_CLAMP_TO_EDGE = GL_CLAMP;
    gl_version = OPENGL_VERSION_1_1;
  }

  gld_EnableTexture2D(true);
  gld_EnableTexture2D(false);
}

// TODO: On 3DS, we'll have to use a blank white
// texture when we need to disable texture usage
void gld_EnableTexture2D(int enable)
{
  if (enable)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
}

void SetTextureMode(tex_mode_e type)
{
  if (gl_compatibility_mode)
  {
    type = TM_MODULATE;
  }

  if (type == TM_MASK)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE); 
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
  }
  else if (type == TM_OPAQUE)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE); 
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  }
  else if (type == TM_INVERT)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE); 
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
  }
  else if (type == TM_INVERTOPAQUE)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE); 
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  }
  else // if (type == TM_MODULATE)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }
}
