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
#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#ifdef DEBUG_DEP
#warning "matrix4x4.h"
#endif

#include "vector3d-pre.h"
#include <iostream>

namespace Utilities
{
	class Matrix4x4
	{
		public:
			float matrix[4][4];

			//! Creates a new 4x4 matrix and loads it with the identity matrix
			Matrix4x4();
			//! Creates a new 4x4 matrix and loads it with the passed matrix
			Matrix4x4(float matrix[4][4]);
			~Matrix4x4();

			Matrix4x4& operator = (const Matrix4x4& a);

			//! Loads the matrix with the identity matrix
			void Identity();

			//! Loads the matrix with the zero matrix
			void Zero();

			//! Sets the matrix to an arbitrary matrix
			void Set(const float a[4][4]);

			static Matrix4x4 TranslationMatrix(float x, float y, float z);

			static Matrix4x4 RotateXMatrix(float radians);
			static Matrix4x4 RotateYMatrix(float radians);
			static Matrix4x4 RotateZMatrix(float radians);
	
			static Matrix4x4 RotateMatrix(float radians, float x, float y, float z);
			
			static Matrix4x4 ScaleMatrix(float x, float y, float z);

			//! Translates the matrix the specified amount
			void Translate(float x, float y, float z);
			
			//! Translates the matrix by the specified vector
			void Translate(Vector3D v);
			
			//! Rotates the matrix the specified amount around the X axis
			void RotateX(float radians);
			
			//! Rotates the matrix the specified amount around the Y axis
			void RotateY(float radians);
			
			//! Rotates the matrix the specified amount around the Z axis
			void RotateZ(float radians);
			
			//! Rotates the matrix the specified amount around the specified axis
			void Rotate(float radians, float x, float y, float z);
			
			//! Rotates the matrix the specified amount around the specified axis
			void Rotate(float radians, Vector3D v);
			
			//! Scales the matrix the specified amount
			void Scale(float x, float y, float z);
			
			//! Scales the matrix the specified amount
			void Scale(float amount);
			
			//! Scales the matrix by the specified vector
			void Scale(Vector3D v);

			void MulBy(const Matrix4x4& a);

			//! Multiplies the matrix by another matrix
			Matrix4x4 operator *(const Matrix4x4& a) const;

			// Multiply the matrix by a vector to get a vector
			Vector3D operator *(const Vector3D& a) const;
			
			void Apply();

	};
	
	std::ostream& operator << (std::ostream& os, const Matrix4x4& a);

}

#ifdef DEBUG_DEP
#warning "matrix4x4.h-end"
#endif

#endif
