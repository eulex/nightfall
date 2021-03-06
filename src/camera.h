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
#include "unit-pre.h"

#ifndef CAMERA_H
#define CAMERA_H

#ifdef DEBUG_DEP
#warning "camera.h"
#endif

#include "camera-pre.h"

#include "vector3d.h"
#include "sdlheader.h"
#include "scenegraph.h"

namespace Game
{
	namespace Dimension
	{
		class Camera : public Scene::Graph::Node
		{
			private:
				Utilities::Vector3D mPosition;
				Utilities::Vector3D mView;
				Utilities::Vector3D mUp;
				Utilities::Vector3D mFocus;
				GLfloat mYMax, mYMin, mZoom, mRotation;
				SDL_mutex* cameraMutex;
				
				Camera(void)     : mPosition(Utilities::Vector3D(0.0f, 0.0f, 0.0f)),
				                   mView(Utilities::Vector3D(0.0f, 0.0f, 0.0f)),
				                   mUp(Utilities::Vector3D(0.0f, 1.0f, 0.0f)),
						   mFocus(Utilities::Vector3D(0.0, 0.0, 0.0)),
								   mYMax(10.0f),
								   mYMin(0),
								   mZoom(2.0f),
								   mRotation(0.0f),
								   cameraMutex(SDL_CreateMutex())
				{}
				
			protected:
				virtual void ApplyMatrix();
				virtual void PostRender();
				void CheckPosition();
			public:
				void SetCamera(Utilities::Vector3D, GLfloat, GLfloat);
				void SetCamera(const gc_ptr<Unit>& unit, GLfloat zoom, GLfloat rotation);
				void SetFocus(float terrain_x, float terrain_y);
				void Fly(GLfloat);
				void FlyHorizontally(GLfloat);
				void Zoom(GLfloat);
				void Rotate(GLfloat);
		
				Utilities::Vector3D GetFocus();
				GLfloat GetZoom();
				GLfloat GetRotation();
				
				void SetYMaximum(GLfloat value) { mYMax = value; }
				void SetYMinimum(GLfloat value) { mYMin = value; }
				
				GLfloat GetYMaximum(void) const { return mYMax; }
				GLfloat GetYMinimum(void) const { return mYMin; }

				GLfloat GetZoom(void) const { return mZoom; }
				GLfloat GetRotation(void) const { return mRotation; }

				const Utilities::Vector3D GetPosVector(void) { return mPosition; };

				static Camera instance;
		};
		
		extern float cameraRotationSpeed;
		extern float cameraFlySpeed;
		extern float cameraZoomSpeed;
	}
}

#ifdef DEBUG_DEP
#warning "camera.h-end"
#endif

#endif
