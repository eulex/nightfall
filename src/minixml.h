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
#ifndef MINIXML_H
#define MINIXML_H

#include "sdlheader.h"
#include "chunkallocator.h"
#include "festring.h"
#include <string>
#include <fstream>
#include <stack>
#include <vector>
#include <map>
#include <sstream>

#ifdef DEBUG_DEP
#warning "minixml.h"
#endif

namespace Utilities
{
	struct XMLElement;
	
	typedef std::map<std::string, std::string> SAttrList;

	typedef void (*TextFunc)(const std::string&);

	typedef void (*TagFunc)(XMLElement*);
	typedef std::map<Utilities::FEString, TagFunc> TagFuncMap;

	typedef std::map<Utilities::FEString, std::string> AttrList;
	typedef void (*AttrFunc)(XMLElement*, const std::string&);
	typedef std::map<Utilities::FEString, AttrFunc> AttrFuncMap;

	class XMLWriter
	{
		private:
		std::ofstream ofile;
		std::stack<std::string> tags;
		bool last_was_push;
		bool deferred_tag_ending;

		void PrepareBeginTag(const std::string& tag);
		void FinishTag();

		public:
		XMLWriter();
		bool Open(std::string filename);
		void Close();
		void BeginTag(const std::string& tag);
		void BeginTag(const std::string& tag, const SAttrList& attributes);
		void Write(const std::string& text);
		void Write(char* text);
		void Write(int i);
		void Write(Uint32 i);
		void Write(float f);
		void Write(double d);
		void EndTag();
	};

	struct XMLElement;

	struct XMLNode
	{
		XMLNode* parent;
		XMLNode() : parent(NULL)
		{
		}
		virtual void Apply(const TagFuncMap& tag_funcs, const TextFunc& text_func);
		virtual void Apply(const Utilities::FEString& tag, const TagFunc& tag_func);
		virtual void Apply(const TextFunc& text_func);
		virtual std::string GetText() = 0;
		virtual ~XMLNode() {};
	};

	struct XMLTextNode : XMLNode
	{
		std::string str;
		virtual void Apply(const TagFuncMap& tag_funcs, const TextFunc& text_func);
		virtual void Apply(const TextFunc& text_func);
		virtual std::string GetText();
		virtual ~XMLTextNode() {};
	};

	struct XMLElement : XMLNode
	{
		unsigned index;
		Utilities::FEString tag;
		AttrList attributes;

		std::vector<XMLNode*> children;
		XMLElement() : index(0)
		{
		}
		
		void Iterate(const TagFuncMap& tag_funcs, const TextFunc& text_func);
		void Iterate(const Utilities::FEString& tag, const TagFunc& tag_func);
		void Iterate(const TextFunc& text_func);
		void Iterate(const AttrFuncMap& attr_funcs);

		const std::string& GetAttribute(const Utilities::FEString& attr, const std::string& def);
		const std::string& GetAttribute(const Utilities::FEString& attr);

		template <typename T>
		T GetAttributeT(const Utilities::FEString& attr, T def)
		{
			std::stringstream ss, ss2;
			T a;
			ss << def;
			ss2 << GetAttribute(attr, ss.str());
			ss2 >> a;
			return a;
		}

		bool HasAttribute(const Utilities::FEString& attr);
		
		virtual void Apply(const TagFuncMap& tag_funcs, const TextFunc& text_func);
		virtual void Apply(const Utilities::FEString& tag, const TagFunc& tag_func);
	
		unsigned Count(const Utilities::FEString& tag);

		virtual std::string GetText();

		virtual ~XMLElement() {};
	};

	class XMLReader
	{
		private:
			std::ifstream ifile;
			std::stack<Utilities::FEString> tags;
			int level;
			ChunkAllocator<XMLElement> *xmlElementAlloc;
			ChunkAllocator<XMLTextNode> *xmlTextNodeAlloc;

			Utilities::FEString ReadTag(bool &open, std::map<Utilities::FEString, std::string> &attributes);
			std::string ReadTagEnd();
			bool IsWhitespace(char c);
			std::string CleanWhitespace(const std::string& s);
			bool ReadDeclaration();
			XMLElement *ReadTagBlock();
			XMLElement *ReadText();

		public:
			XMLReader();
			~XMLReader();
			bool Read(const std::string& filename);
			void Deallocate();
			XMLElement *root;
	};
}

#ifdef DEBUG_DEP
#warning "minixml.h-end"
#endif

#endif

