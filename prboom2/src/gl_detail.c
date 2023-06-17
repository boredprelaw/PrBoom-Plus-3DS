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
 *
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gl_opengl.h"

#include "z_zone.h"
#include <SDL.h>

#include <math.h>

#include "v_video.h"
#include "r_main.h"
#include "gl_intern.h"
#include "w_wad.h"
#include "lprintf.h"
#include "p_spec.h"
#include "m_misc.h"
#include "sc_man.h"
#include "e6y.h"
#include "i_system.h"

int render_usedetail;
int gl_allow_detail_textures;
int gl_detail_maxdist;
float gl_detail_maxdist_sqrt;

detail_t *details;
int details_count;
int details_size;
int scene_has_details;
int scene_has_wall_details;
int scene_has_flat_details;

typedef enum
{
  TAG_DETAIL_WALL,
  TAG_DETAIL_FLAT,
  TAG_DETAIL_MAX,
} tag_detail_e;

static const char *DetailItem_Keywords[TAG_DETAIL_MAX + 1] =
{
  "walls",
  "flats",
  NULL
};

static GLuint last_detail_texid = -1;

float xCamera,yCamera,zCamera;
TAnimItemParam *anim_flats = NULL;
TAnimItemParam *anim_textures = NULL;

void gld_ShutdownDetail(void);

void M_ChangeUseDetail(void)
{
  render_usedetail = false;

  if (V_GetMode() == VID_MODEGL)
  {
    render_usedetail = gl_allow_detail_textures;
    gld_EnableDetail(true);
    gld_EnableDetail(false);
    gld_FlushTextures();
  }
}

float distance2piece(float x0, float y0, float x1, float y1, float x2, float y2)
{
  float t, w;
  float x01 = x0 - x1;
  float x02 = x0 - x2;
  float x21 = x2 - x1;
  float y01 = y0 - y1;
  float y02 = y0 - y2;
  float y21 = y2 - y1;

  if((x01 * x21 + y01 * y21) * (x02 * x21 + y02 * y21) > 0.0001f)
  {
    t = x01 * x01 + y01 * y01;
    w = x02 * x02 + y02 * y02;
    if (w < t) t = w;
  }
  else
  {
    float i1 = x01 * y21 - y01 * x21;
    float i2 = x21 * x21 + y21 * y21;
    t = (i1 * i1) / i2;
  }

  return t;
}

int gld_IsDetailVisible(float x0, float y0, float x1, float y1, float x2, float y2)
{
  if (gl_detail_maxdist_sqrt == 0)
  {
    return true;
  }
  else
  {
    return (distance2piece(x0, y0, x1, y1, x2, y2) < gl_detail_maxdist_sqrt);
  }
}

void gld_InitDetail(void)
{
  gl_detail_maxdist_sqrt = (float)sqrt((float)gl_detail_maxdist);

  I_AtExit(gld_ShutdownDetail, true);
  M_ChangeUseDetail();
}

void gld_ShutdownDetail(void)
{
  int i;

  if (details)
  {
    for (i = 0; i < details_count; i++)
    {
      glDeleteTextures(1, &details[i].texid);
    }

    free(details);
    details = NULL;
    details_count = 0;
    details_size = 0;
  }
}

void gld_PreprocessDetail(void)
{
}


void gld_EnableDetail(int enable)
{
}

void gld_DrawWallDetail_NoARB(GLWall *wall)
{
  if (!wall->gltexture->detail)
    return;

  if (wall->flag >= GLDWF_SKY)
    return;

  if (gld_IsDetailVisible(xCamera, yCamera, 
      wall->glseg->x1, wall->glseg->z1,
      wall->glseg->x2, wall->glseg->z2))
  {
    float w, h, dx, dy;
    detail_t *detail = wall->gltexture->detail;

    w = wall->gltexture->detail_width;
    h = wall->gltexture->detail_height;
    dx = detail->offsetx; 
    dy = detail->offsety;

    gld_BindDetail(wall->gltexture, detail->texid);

    gld_StaticLightAlpha(wall->light, wall->alpha);
    glBegin(GL_TRIANGLE_FAN);

    // lower left corner
    glTexCoord2f(wall->ul*w+dx,wall->vb*h+dy);
    glVertex3f(wall->glseg->x1,wall->ybottom,wall->glseg->z1);

    // split left edge of wall
    if (!wall->glseg->fracleft)
      gld_SplitLeftEdge(wall, true);

    // upper left corner
    glTexCoord2f(wall->ul*w+dx,wall->vt*h+dy);
    glVertex3f(wall->glseg->x1,wall->ytop,wall->glseg->z1);

    // upper right corner
    glTexCoord2f(wall->ur*w+dx,wall->vt*h+dy);
    glVertex3f(wall->glseg->x2,wall->ytop,wall->glseg->z2);

    // split right edge of wall
    if (!wall->glseg->fracright)
      gld_SplitRightEdge(wall, true);

    // lower right corner
    glTexCoord2f(wall->ur*w+dx,wall->vb*h+dy);
    glVertex3f(wall->glseg->x2,wall->ybottom,wall->glseg->z2);

    glEnd();
  }
}

void gld_DrawFlatDetail_NoARB(GLFlat *flat)
{
  float w, h, dx, dy;
  int loopnum;
  GLLoopDef *currentloop;
  detail_t *detail;

  if (!flat->gltexture->detail)
    return;

  detail = flat->gltexture->detail;
  gld_BindDetail(flat->gltexture, detail->texid);

  gld_StaticLightAlpha(flat->light, flat->alpha);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(0.0f,flat->z,0.0f);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();

  w = flat->gltexture->detail_width;
  h = flat->gltexture->detail_height;
  dx = detail->offsetx;
  dy = detail->offsety;

  if ((flat->flags & GLFLAT_HAVE_OFFSET) || dx || dy)
  {
    glTranslatef(flat->uoffs * w + dx, flat->voffs * h + dy, 0.0f);
  }

  glScalef(w, h, 1.0f);

  if (flat->sectornum>=0)
  {
    // go through all loops of this sector
#if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO)
    for (loopnum=0; loopnum<sectorloops[flat->sectornum].loopcount; loopnum++)
    {
      currentloop=&sectorloops[flat->sectornum].loops[loopnum];
      glDrawArrays(currentloop->mode,currentloop->vertexindex,currentloop->vertexcount);
    }
#else
    for (loopnum=0; loopnum<sectorloops[flat->sectornum].loopcount; loopnum++)
    {
      int vertexnum;
      // set the current loop
      currentloop=&sectorloops[flat->sectornum].loops[loopnum];
      if (!currentloop)
        continue;
      // set the mode (GL_TRIANGLES, GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN)
      glBegin(currentloop->mode);
      // go through all vertexes of this loop
      for (vertexnum=currentloop->vertexindex; vertexnum<(currentloop->vertexindex+currentloop->vertexcount); vertexnum++)
      {
        // set texture coordinate of this vertex
        glTexCoord2fv((GLfloat*)&flats_vbo[vertexnum].u);
        // set vertex coordinate
        glVertex3fv((GLfloat*)&flats_vbo[vertexnum].x);
      }
      // end of loop
      glEnd();
    }
#endif
  }
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

static int C_DECL dicmp_wall_detail(const void *a, const void *b)
{
  detail_t *d1 = ((const GLDrawItem *)a)->item.wall->gltexture->detail;
  detail_t *d2 = ((const GLDrawItem *)b)->item.wall->gltexture->detail;
  return d1 - d2;
}

static int C_DECL dicmp_flat_detail(const void *a, const void *b)
{
  detail_t *d1 = ((const GLDrawItem *)a)->item.flat->gltexture->detail;
  detail_t *d2 = ((const GLDrawItem *)b)->item.flat->gltexture->detail;
  return d1 - d2;
}

void gld_DrawItemsSortByDetail(GLDrawItemType itemtype)
{
  typedef int(C_DECL *DICMP_ITEM)(const void *a, const void *b);

  static DICMP_ITEM itemfuncs[GLDIT_TYPES] = {
    0,
    dicmp_wall_detail, dicmp_wall_detail, dicmp_wall_detail, dicmp_wall_detail, dicmp_wall_detail,
    dicmp_wall_detail, dicmp_wall_detail,
    dicmp_flat_detail, dicmp_flat_detail,
    dicmp_flat_detail, dicmp_flat_detail,
    0, 0, 0,
    0,
  };

  if (itemfuncs[itemtype] && gld_drawinfo.num_items[itemtype] > 1)
  {
    qsort(gld_drawinfo.items[itemtype], gld_drawinfo.num_items[itemtype],
      sizeof(gld_drawinfo.items[itemtype]), itemfuncs[itemtype]);
  }
}

void gld_DrawDetail_NoARB(void)
{
  int i;

  if (!scene_has_wall_details && !scene_has_flat_details)
    return;

  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);

  last_detail_texid = -1;

  // detail flats

  if (scene_has_flat_details)
  {
    // enable backside removing
    glEnable(GL_CULL_FACE);

    // floors
    glCullFace(GL_FRONT);
    gld_DrawItemsSortByDetail(GLDIT_FLOOR);
    for (i = gld_drawinfo.num_items[GLDIT_FLOOR] - 1; i >= 0; i--)
    {
      gld_SetFog(gld_drawinfo.items[GLDIT_FLOOR][i].item.flat->fogdensity);
      gld_DrawFlatDetail_NoARB(gld_drawinfo.items[GLDIT_FLOOR][i].item.flat);
    }
    // ceilings
    glCullFace(GL_BACK);
    gld_DrawItemsSortByDetail(GLDIT_CEILING);
    for (i = gld_drawinfo.num_items[GLDIT_CEILING] - 1; i >= 0; i--)
    {
      gld_SetFog(gld_drawinfo.items[GLDIT_CEILING][i].item.flat->fogdensity);
      gld_DrawFlatDetail_NoARB(gld_drawinfo.items[GLDIT_CEILING][i].item.flat);
    }
    glDisable(GL_CULL_FACE);
  }

  // detail walls
  if (scene_has_wall_details)
  {
    gld_DrawItemsSortByDetail(GLDIT_WALL);
    for (i = gld_drawinfo.num_items[GLDIT_WALL] - 1; i >= 0; i--)
    {
      gld_SetFog(gld_drawinfo.items[GLDIT_WALL][i].item.wall->fogdensity);
      gld_DrawWallDetail_NoARB(gld_drawinfo.items[GLDIT_WALL][i].item.wall);
    }

    gld_DrawItemsSortByDetail(GLDIT_MWALL);

    for (i = gld_drawinfo.num_items[GLDIT_MWALL] - 1; i >= 0; i--)
    {
      GLWall *wall = gld_drawinfo.items[GLDIT_MWALL][i].item.wall;
      gld_SetFog(wall->fogdensity);
      gld_DrawWallDetail_NoARB(wall);
    }

    if (gld_drawinfo.num_items[GLDIT_FWALL] > 0)
    {
      glPolygonOffset(1.0f, 128.0f);
      glEnable(GL_POLYGON_OFFSET_FILL);

      gld_DrawItemsSortByDetail(GLDIT_FWALL);
      for (i = gld_drawinfo.num_items[GLDIT_FWALL] - 1; i >= 0; i--)
      {
        gld_DrawWallDetail_NoARB(gld_drawinfo.items[GLDIT_FWALL][i].item.wall);
      }

      glPolygonOffset(0.0f, 0.0f);
      glDisable(GL_POLYGON_OFFSET_FILL);
    }

    gld_DrawItemsSortByDetail(GLDIT_TWALL);
    for (i = gld_drawinfo.num_items[GLDIT_TWALL] - 1; i >= 0; i--)
    {
      gld_SetFog(gld_drawinfo.items[GLDIT_TWALL][i].item.wall->fogdensity);
      gld_DrawWallDetail_NoARB(gld_drawinfo.items[GLDIT_TWALL][i].item.wall);
    }
  }

  // restore
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void gld_InitFrameDetails(void)
{
  last_detail_texid = -1;

  scene_has_details =
    (render_usedetail) &&
    (scene_has_wall_details || scene_has_flat_details);
}

void gld_BindDetail(GLTexture *gltexture, int enable)
{
  if (scene_has_details)
  {
    if (enable &&
        gltexture->detail &&
        gltexture->detail->texid != last_detail_texid)
    {
      last_detail_texid = gltexture->detail->texid;
      glBindTexture(GL_TEXTURE_2D, gltexture->detail->texid);
    }
  }
}

void gld_SetTexDetail(GLTexture *gltexture)
{
  int i;

  gltexture->detail = NULL;

  if (details_count > 0)
  {
    // linear search
    for (i = 0; i < details_count; i++)
    {
      if (gltexture->index == details[i].texture_num)
      {
        gltexture->detail = &details[i];
        break;
      }
    }

    if (!gltexture->detail)
    {
      switch (gltexture->textype)
      {
      case GLDT_TEXTURE:
        if (details[TAG_DETAIL_WALL].texid > 0)
          gltexture->detail = &details[TAG_DETAIL_WALL];
        break;
      case GLDT_FLAT:
        if (details[TAG_DETAIL_FLAT].texid > 0)
          gltexture->detail = &details[TAG_DETAIL_FLAT];
        break;
      }
    }

    if (gltexture->detail)
    {
      gltexture->detail_width  = (float)gltexture->realtexwidth  / gltexture->detail->width;
      gltexture->detail_height = (float)gltexture->realtexheight / gltexture->detail->height;
    }
  }
}

GLuint gld_LoadDetailName(const char *name)
{
  GLuint texid = 0;
  int lump;

  lump = (W_CheckNumForName)(name, ns_hires);

  if (lump != -1)
  {
    SDL_PixelFormat fmt;
    SDL_Surface *surf = NULL;
    SDL_Surface *surf_raw;

    surf_raw = SDL_LoadBMP_RW(SDL_RWFromConstMem(W_CacheLumpNum(lump), W_LumpLength(lump)), 1);

    W_UnlockLumpNum(lump);

    if (surf_raw)
    {
      fmt = *surf_raw->format;
      fmt.BitsPerPixel = 24;
      fmt.BytesPerPixel = 3;
      surf = SDL_ConvertSurface(surf_raw, &fmt, surf_raw->flags);
      SDL_FreeSurface(surf_raw);
      if (surf)
      {
        glGenTextures(1, &texid);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, texid);

        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, gl_tex_format,
          surf->w, surf->h, 0,
          imageformats[surf->format->BytesPerPixel],
          GL_UNSIGNED_BYTE, surf->pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        SDL_FreeSurface(surf);
      }
    }
  }

  return texid;
}

int gld_ReadDetailParams(tag_detail_e item, detail_t *detail)
{
  int result = false;
  if (SC_Check())
  {
    // get detail texture name
    SC_GetString();

    if (strlen(sc_String) < 9)
    {
      detail->texid = gld_LoadDetailName(sc_String);

      if (detail->texid > 0)
      {
        float f;

        if (SC_Check() && SC_GetString() && M_StrToFloat(sc_String, &f))
          detail->width = f;
        if (SC_Check() && SC_GetString() && M_StrToFloat(sc_String, &f))
          detail->height = f;

        if (SC_Check() && SC_GetString() && M_StrToFloat(sc_String, &f))
          detail->offsetx = f / detail->width;
        if (SC_Check() && SC_GetString() && M_StrToFloat(sc_String, &f))
          detail->offsety = f / detail->height;

        result = true;
      }
    }
    // skip the rest of unknown params
    while (SC_Check())
      SC_GetString();
  }

  return result;
}

void gld_ParseDetailItem(tag_detail_e item)
{
  // item's default values
  details[item].width = 16.0f;
  details[item].height = 16.0f;
  details[item].offsetx = 0.0f;
  details[item].offsety = 0.0f;
  if (SC_Check() && !SC_Compare("{"))
  {
    gld_ReadDetailParams(item, &details[item]);
  }

  if (SC_GetString() && SC_Compare("{"))
  {
    while (SC_GetString() && !SC_Compare("}"))
    {
      int result;
      detail_t detail;

      // reset fields for next iteration
      detail.texid   = 0;
      detail.width   = 16.0f;
      detail.height  = 16.0f;
      detail.offsetx = 0.0f;
      detail.offsety = 0.0f;

      if (strlen(sc_String) < 9)
      {
        switch (item)
        {
        case TAG_DETAIL_WALL:
          detail.texture_num = R_CheckTextureNumForName(sc_String);
          break;
        case TAG_DETAIL_FLAT:
          detail.texture_num = (W_CheckNumForName)(sc_String, ns_flats);
          break;
        }

        result = gld_ReadDetailParams(item, &detail);

        if (result || details[item].texid > 0)
        {
          if (details_count + 1 > details_size)
          {
            details_size = (details_size == 0 ? 128 : details_size * 2);
            details = realloc(details, details_size * sizeof(details[0]));
          }
          details[details_count] = detail;
          details_count++;
        }
      }
    }
  }
}

void gld_ParseDetail(void)
{
  gld_ShutdownDetail();

  details_count = 2; // reserved for default wall and flat
  details_size = 128;
  details = calloc(details_size, sizeof(details[0]));

  // skip "Detail" params
  while (SC_Check() && !SC_Compare("{"))
    SC_GetString();

  if (SC_GetString() && SC_Compare("{"))
  {
    while (SC_GetString() && !SC_Compare("}"))
    {
      switch (SC_MatchString(DetailItem_Keywords))
      {
      case TAG_DETAIL_WALL:
        gld_ParseDetailItem(TAG_DETAIL_WALL);
        break;
      case TAG_DETAIL_FLAT:
        gld_ParseDetailItem(TAG_DETAIL_FLAT);
        break;
      }
    }
  }
}
