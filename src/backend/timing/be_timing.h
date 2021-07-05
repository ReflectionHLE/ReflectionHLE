/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
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
