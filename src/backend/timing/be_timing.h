/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BE_TIMING
#define BE_TIMING

#include "refkeen_config.h"

// Replacement for int 8 timer callback
extern void (*g_sdlTimerIntFuncPtr)(void);

int BE_ST_SET_TIMER_INT_COUNTER_SET(int x);
int BE_ST_SET_TIMER_INT_COUNTER_GET(void);
int BE_ST_SET_TIMER_INT_COUNTER_ADD(int x);
int BE_ST_SET_TIMER_INT_COUNTER_INC(void);

#endif
