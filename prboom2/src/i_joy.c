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
 *   Joystick handling for Linux
 *
 *-----------------------------------------------------------------------------
 */

#ifndef lint
#endif /* lint */

#include <stdlib.h>

#include "SDL.h"
#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"
#include "lprintf.h"
#include "i_system.h"

static SDL_GameController *joystick = NULL;

static void I_EndJoystick(void)
{
  lprintf(LO_DEBUG, "I_EndJoystick : closing joystick\n");

  if(joystick)
    SDL_GameControllerClose(joystick);
}

void I_PollJoystick(void)
{
  event_t ev;
  Sint16 axis_value;

  if (!joystick)
    return;

  ev.type = ev_joystick;
  ev.data1 =
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_A)<<0) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_B)<<1) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_X)<<2) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_Y)<<3) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)<<4) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)<<5) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_START)<<6) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_BACK)<<7) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP)<<8) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN)<<9) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT)<<10) |
    (SDL_GameControllerGetButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)<<11);
  axis_value = SDL_GameControllerGetAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX) / 3000;
  if (abs(axis_value)<2) axis_value=0;
  ev.data2 = axis_value;
  axis_value = SDL_GameControllerGetAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY) / 3000;
  if (abs(axis_value)<2) axis_value=0;
  ev.data3 = axis_value;

  D_PostEvent(&ev);
}

void I_InitJoystick(void)
{
  const char* fname = "I_InitJoystick : ";

  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

  joystick = SDL_GameControllerOpen(0);

  if (!joystick) {
    lprintf(LO_ERROR, "%serror opening joystick\n", fname);
  }
  else {
    I_AtExit(I_EndJoystick, true);

    lprintf(LO_INFO, "%sopened %s\n", fname, SDL_GameControllerName(joystick));
  }
}
