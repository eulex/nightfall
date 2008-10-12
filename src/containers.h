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
#ifndef __CONTAINERS_H__
#define __CONTAINERS_H__ 

#ifdef DEBUG_DEP
	#warning "containers.h"
#endif

#include "containers-pre.h"

#include "compositor.h"
#include "themeengine.h"

namespace GUI
{
	namespace Containers
	{
		class TablePanel : public Container
		{
			private:
				Component*** subComponents;
				struct Position
				{
					int x, y;
				};
				std::map<componentHandle, Position> positions;
			public:
				TablePanel(int cols, int rows);

				componentHandle add(Component* comp, int col, int row);
			
				virtual void remove(componentHandle handle);
				virtual void clear();
		};

		class DockPanel : public Container
		{
			public:
				enum DockPosition
				{
					DOCK_MIDDLE = 0,
					DOCK_ABOVE,
					DOCK_BELOW,
					DOCK_LEFT,
					DOCK_RIGHT,
					DOCK_NUM
				};

				DockPanel();

				componentHandle add(Component* comp, DockPosition pos);

				virtual void remove(componentHandle handle);
				virtual void clear();
			private:
				Component* subComponents[DOCK_NUM];
				std::map<componentHandle, DockPosition> positions;
		};

		class FlowPanel : public Container
		{
			public:
				enum Direction
				{
					DIRECTION_VERTICAL,
					DIRECTION_HORIZONTAL,
					DIRECTION_NONE
				};
			private:
				Direction primaryDirection, secondaryDirection;
			public:
				FlowPanel(Direction primaryDirection, Direction secondaryDirection);

				componentHandle insert(Component* comp, int position);
				componentHandle add(Component* comp);

				virtual void remove(componentHandle handle);
				virtual void clear();
		};
	}
}

#ifdef DEBUG_DEP
	#warning "containers.h-end"
#endif

#endif

