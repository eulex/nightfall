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
#ifndef MEMDEBUG_H
#define MEMDEBUG_H

#include <set>
#include <exception> // for std::bad_alloc
#include <new>

/*void* operator new(size_t size, std::string file, unsigned line) throw(std::bad_alloc);

void operator delete(void *p) throw();

void* operator new[](size_t size, std::string, unsigned line) throw(std::bad_alloc);

void operator delete[](void *p) throw();*/

void WriteFragmentationReport(std::string filename);

void WriteLeakReport(std::string filename);

//#define new new(__FILE__, __LINE__)

#endif

