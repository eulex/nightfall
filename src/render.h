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
#ifndef RENDER_H
#define RENDER_H

#ifdef DEBUG_DEP
#warning "render.h"
#endif

#include "ogrexmlmodel-pre.h"
#include "scenegraph.h"
#include "render-pre.h"
#include "materialxml-pre.h"
#include "sdlheader.h"

namespace Scene
{
	namespace Render
	{
		struct GLState : public gc_ptr_from_this<GLState>
		{
				// Edit curStateBools below when adding or removing states
				enum GLStateBool
				{
					GLSTATE_BLEND,
					GLSTATE_CULL_FACE,
					GLSTATE_DEPTH_TEST,
					GLSTATE_NUM
				};

				enum StateBoolState
				{
					STATEBOOLSTATE_ENABLED,
					STATEBOOLSTATE_DISABLED,
					STATEBOOLSTATE_UNKNOWN
				};

				std::map<GLStateBool, bool> stateBools;
				gc_ptr<Utilities::Material> material;

				GLState() : material(NULL)
				{
					
				}

				void Apply();

				void shade()
				{
					material.shade();
				}
				
				typedef std::map<GLStateBool, bool> GLStateBoolList;

				static void ResetState();
				
			private:

				static StateBoolState curStateBools[GLSTATE_NUM];
				static gc_ptr<GLState> curSetState;

				void SetState(GLStateBool state, bool b);
		};

		class GLStateNode : public Graph::Node
		{
			protected:
				virtual void PreRender();
				gc_ptr<GLState> myGLState;
			public:
				GLStateNode(gc_ptr<GLState> state);

				virtual void shade()
				{
					Graph::Node::shade();
					myGLState.shade();
				}
		};

		struct GeomState : public gc_ptr_from_this<GeomState>
		{

				struct AttribArray
				{
					std::string name;
					gc_ptr<VBO> array;
					GLenum type;

					AttribArray(std::string name, gc_ptr<VBO> array, GLenum type) : name(name), array(array), type(type)
					{
						
					}
				};

				gc_ptr<VBO> vertexArray;
				gc_ptr<VBO> normalArray;
				gc_ptr<VBO> indicesArray;
				bool indicesAre32Bit;
				std::vector<gc_ptr<VBO> > texCoordArrays;
				std::vector<AttribArray> attribArrays;
				GLenum primitive;
				unsigned numElems;

				GeomState() : vertexArray(NULL), normalArray(NULL), indicesArray(NULL), indicesAre32Bit(false), primitive(GL_TRIANGLES), numElems(0)
				{
					
				}

				void Apply(GLhandleARB program);

				void Draw();

				void shade()
				{
					vertexArray.shade();
					normalArray.shade();
					indicesArray.shade();
					gc_shade_container(texCoordArrays);
				}
				
				static void ResetState();
				
			private:
				static gc_ptr<GeomState> curSetState;
		};

		class GeometryNode : public GLStateNode
		{
			protected:
				virtual void PreRender();
				virtual void Render();
				gc_ptr<GeomState> myGeomState;
			public:
				GeometryNode(gc_ptr<GeomState> geomState, gc_ptr<GLState> glState);

				virtual void shade()
				{
					GLStateNode::shade();
					myGeomState.shade();
				}
		};

		class OgreSubMeshNode : public GeometryNode
		{
			protected:
				virtual void Render();
			public:
				OgreSubMeshNode(const gc_ptr<Utilities::OgreSubMesh>& mesh);
				virtual ~OgreSubMeshNode();
		};
		
		class OgreMeshNode : public Graph::Node
		{
			private:
				gc_ptr<Utilities::OgreMesh> mesh;
			protected:
				virtual void ApplyMatrix();
				virtual void PostRender();
			public:
				OgreMeshNode(const gc_ptr<Utilities::OgreMesh> mesh);

				virtual void shade()
				{
					Graph::Node::shade();
					mesh.shade();
				}
		};


		
		class MeshTransformation
		{
			public:
				virtual void Apply(Utilities::Matrix4x4& matrix) = 0;
				virtual ~MeshTransformation() {};
				void shade() {};
		};

		class MeshTranslation : public MeshTransformation
		{
			private:
				float x, y, z;
			public:
				MeshTranslation(float x, float y, float z) : x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
				virtual ~MeshTranslation() {};
		};

		class MeshRotation : public MeshTransformation
		{
			private:
				float radians, x, y, z;
			public:
				MeshRotation(float radians, float x, float y, float z) : radians(radians), x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
				virtual ~MeshRotation() {};
		};

		class MeshScaling : public MeshTransformation
		{
			private:
				float x, y, z;
			public:
				MeshScaling(float x, float y, float z) : x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
				virtual ~MeshScaling() {};
		};

	}
}

#ifdef DEBUG_DEP
#warning "render.h-end"
#endif

#endif
