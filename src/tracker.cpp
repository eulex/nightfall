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
#include "tracker.h"
#include "game.h"
#include "utilities.h"
#include "httprequest.h"

#include "sdlheader.h"

namespace Utilities
{
	class BeginRequest : public Utilities::HTTPRequest
	{
		void Fail()
		{
			std::cout << "Failed to connect to tracker; retrying in 5s" << std::endl;
			SDL_Delay(5000);
			(new BeginRequest)->Fetch(host, port, request);
		}

		void Handle()
		{
			if (statusCode == 200)
			{
				if (ret.find("OK", 0) == 0 && ret.find("\n") != std::string::npos)
				{
					gameTracker.SetSecretGameID(ret.substr(ret.find("\n")+1));
				}
			}
			else
			{
				std::cout << "Failed to connect to tracker; retrying in 5s" << std::endl;
				SDL_Delay(5000);
				(new BeginRequest)->Fetch(host, port, request);
			}
		}
	};

	class GenericRequest : public Utilities::HTTPRequest
	{
		void Fail()
		{
		}

		void Handle()
		{
		}
	};

	void Tracker::BeginGame(int maxplayers, int freeplayerslots, int freespectatorslots)
	{
		std::map<std::string, std::string> params;

		secretgameid = "";
		needShutdown = false;
		unsentUpdate.clear();

		params["cmd"] = "begin";
		params["name"] = "This is a test";
		params["level"] = Game::Rules::CurrentLevel;
		params["levelhash"] = Game::Rules::CurGame::Instance()->GetLevelHash();
		params["port"] = Utilities::ToString(Game::Networking::netPort);
		params["maxplayers"] = Utilities::ToString(maxplayers);
		params["freeplayerslots"] = Utilities::ToString(freeplayerslots);
		params["freespectatorslots"] = Utilities::ToString(freespectatorslots);

		(new BeginRequest)->GET("http://nightfall-rts.org/tracker/", params);
	}
	
	void Tracker::UpdateGame(int maxplayers, int freeplayerslots, int freespectatorslots, bool started)
	{
		std::map<std::string, std::string> params;

		params["cmd"] = "update";
		params["secretgameid"] = secretgameid;
		params["name"] = "This is an update";
		params["maxplayers"] = Utilities::ToString(maxplayers);
		params["started"] = Utilities::ToString(int(started));
		params["freeplayerslots"] = Utilities::ToString(freeplayerslots);
		params["freespectatorslots"] = Utilities::ToString(freespectatorslots);
		params["updateid"] = Utilities::ToString(++lastUpdateID);

		if (secretgameid.length())
		{
			(new GenericRequest)->GET("http://nightfall-rts.org/tracker/", params);
		}
		else
		{
			unsentUpdate = params;
		}
	}
	
	void Tracker::KeepAlive()
	{
		std::map<std::string, std::string> params;

		params["cmd"] = "keepalive";
		params["secretgameid"] = secretgameid;

		if (secretgameid.length())
		{
			(new GenericRequest)->GET("http://nightfall-rts.org/tracker/", params);
		}
		else
		{
			unsentUpdate = params;
		}
	}
	
	void Tracker::EndGame()
	{
		if (secretgameid.length())
		{
			std::map<std::string, std::string> params;

			params["cmd"] = "end";
			params["secretgameid"] = secretgameid;

			(new GenericRequest)->GET("http://nightfall-rts.org/tracker/", params);

			secretgameid = "";
		}
		else
		{
			needShutdown = true;
		}
	}
	
	void Tracker::SetSecretGameID(std::string gameid)
	{
		std::cout << "game id: " << gameid << std::endl;
		if (gameid.length() != 40)
		{
			std::cout << "invalid." << std::endl;
			return;
		}
		secretgameid = gameid;
		if (needShutdown)
		{
			EndGame();
		}
		else
		{
			if (!unsentUpdate.empty())
			{
				unsentUpdate["secretgameid"] = secretgameid;
				(new GenericRequest)->GET("http://nightfall-rts.org/tracker/", unsentUpdate);
			}
		}
	}

	bool Tracker::IsPrefixOfSecretGameID(std::string prefix)
	{
		return prefix.length() <= secretgameid.length() && prefix == secretgameid.substr(0, prefix.length());
	}

	std::string Tracker::GetLastPartOfSecretGameId()
	{
		return secretgameid.length() == 40 ? secretgameid.substr(20, 20) : std::string(' ', 20);
	}
	
	Tracker gameTracker;
}

