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
#ifndef __HTTPREQUEST_H_PRE__
#define __HTTPREQUEST_H_PRE__

#ifdef DEBUG_DEP
#warning "httprequest.h-pre"
#endif

#include <string>
#include <map>

namespace Utilities
{
	class HTTPRequest
	{
		private:
			static int _Thread(void *arg);
		protected:
			std::string request;
			std::string host;
			int port;

			int statusCode;
			std::map<std::string, std::string> header;
			std::string ret;
			
			void Fetch(std::string host, int port, std::string request);
		public:
			void Fetch(std::string method, std::string host, int port, std::string path, std::map<std::string, std::string> params, std::map<std::string, std::string> header, std::string contents);
			void GET(std::string url, std::map<std::string, std::string> params);

			virtual void Handle() = 0;
			virtual void Fail() = 0;
	};
}

#endif
