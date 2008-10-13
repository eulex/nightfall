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
#include "themeengine.h"
#include "font.h"

namespace GUI
{
	namespace ThemeEngine
	{
		namespace Info
		{
			InfoBase::InfoBase(Component* component) : comp(component)
			{
				
			}

			void InfoBase::notifyChanged()
			{
				comp->scheduleRelayout();
			}

			Component* InfoBase::getComponent() const
			{
				return comp;
			}
			
			Metrics* InfoBase::getMetrics() const
			{
				return comp->getMetrics();
			}
			
			Text::Text(Component* component, std::string text) : InfoBase(component), text(text)
			{
				
			}

			void Text::set(std::string text)
			{
				this->text = text;
			}

			std::string Text::get() const
			{
				return text;
			}
		}

		template <>
		void Drawer<Info::Text>::Draw(const Info::Text& text, float x, float y, float w, float h)
		{
			::Window::GUI::FontCache::RenderedText RenderedInfo;
			::Window::GUI::Fonts.SetFontType(2);
			::Window::GUI::Fonts.RenderText(text.get(), RenderedInfo, text.getMetrics()->getDPI());
			
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, RenderedInfo.Texture);
			glPushMatrix();
			glTranslatef(x, y, 0.0f);
			glBegin(GL_QUADS);
				glColor4f(1.0f,1.0f,1.0f,1.0f);
				glTexCoord2f(RenderedInfo.texCoords[0], RenderedInfo.texCoords[1]);
				glVertex2f(0.0f,0.0f);

				glTexCoord2f(RenderedInfo.texCoords[2], RenderedInfo.texCoords[3]);
				glVertex2f(RenderedInfo.w, 0.0f);

				glTexCoord2f(RenderedInfo.texCoords[4], RenderedInfo.texCoords[5]);
				glVertex2f(RenderedInfo.w, RenderedInfo.h);

				glTexCoord2f(RenderedInfo.texCoords[6], RenderedInfo.texCoords[7]);
				glVertex2f(0.0f, RenderedInfo.h);
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
		}

		template <>
		void Drawer<Info::Text>::GetSize(const Info::Text& text, float cw, float ch, float& w, float& h)
		{
			::Window::GUI::FontCache::TextDimension dims;
			::Window::GUI::Fonts.SetFontType(2);
			dims = ::Window::GUI::Fonts.GetTextSize(text.get(), text.getMetrics()->getDPI());
			w = dims.w;
			h = dims.h;
		}
	}
}
