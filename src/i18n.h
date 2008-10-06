/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __I18N_H__
#define __I18N_H__ 

#ifdef ENABLE_NLS

#include "gettext.h"
#define _(x) gettext(x)
#define N_(s, p, n) ngettext(s, p, n)

#else

#define _(x) (x)
#define N_(s, p, n) ((n) == 1 ? (s) : (p)) // Fallback

#endif // ENABLE_NLS

#define M_(x) (x)

#endif // __I18N_H__