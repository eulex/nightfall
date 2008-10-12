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
#ifndef __THEMEENGINE_H__
#define __THEMEENGINE_H__ 

#ifdef DEBUG_DEP
	#warning "themeengine.h"
#endif

#include "themeengine-pre.h"

#include "compositor.h"

namespace GUI
{
	namespace ThemeEngine
	{
		namespace Info
		{
			class SubComponent : InfoBase
			{
				protected:
					Component *subComp;
				public:
					void set(std::string text);
					void set(Component *comp);
			};
		}
	}
}

#ifdef DEBUG_DEP
	#warning "themeengine.h-end"
#endif

#endif