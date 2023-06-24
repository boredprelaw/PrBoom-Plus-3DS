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

#include "doomtype.h"
#include "lprintf.h"

#ifdef GL_DOOM

#include <SDL/SDL.h>
#include "gl_opengl.h"

dboolean gl_ext_arb_vertex_buffer_object = false;

// VBO
PFNGLGENBUFFERSARBPROC GLEXT_glGenBuffersARB = NULL;
PFNGLDELETEBUFFERSARBPROC GLEXT_glDeleteBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC GLEXT_glBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC GLEXT_glBufferDataARB = NULL;

void gld_InitOpenGL()
{
  // VBO
#ifdef USE_VBO
  gl_ext_arb_vertex_buffer_object = true;

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

  gld_EnableTexture2D(true);
  gld_EnableTexture2D(false);
}

void gld_EnableTexture2D(int enable)
{
  if (enable) {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
  }
  else {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE); 
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  }
}

#endif
