// Set to 1 if you wish to use audio!
#define USE_AUDIO 1
#define USE_FONT 1
//#define DEBUG_DEP // uncomment if you want to enable .h dependency debug output -- note: might only work with g++-ish compilers

#if WIN32
	#include <sstream>
	#include "win32/win32_console.h"
	#include <SDL_syswm.h>
#endif

#include <iostream>
#include "window.h"
#include "game.h"
#include "font.h"
#include "aibase.h"
#include "configuration.h"
#include "unitinterface.h"
#include "gui.h"
#include "networking.h"
#include "paths.h"
#include "camera.h"
#include "aipathfinding.h"
#include "unit.h"

#if USE_FONT == 1
#include "font.h"
#endif

#if USE_AUDIO == 1
#include "audio.h"
#endif
#define USE_LUA
#include "luawrapper.h"

using namespace std;

void ParseArguments(int argc, char** argv);
void KillAll(void);
int InitMainConfig();

std::map<std::string, std::string> mainConfigOverrides;

int main(int argc, char** argv)
{
#ifdef WIN32
	//RedirectIOToConsole();
#endif

	Utilities::InitPaths(argv[0]);

	ParseArguments(argc, argv);

	int errCode = Window::Init();
	
	if (errCode != SUCCESS)
	{
		if (errCode == WINDOW_ERROR_INIT_FAILURE)
		{
#ifdef WIN32
			MessageBoxA( NULL, "Misslyckades med att initiera SDL!", "Fel har uppst�tt", MB_OK | MB_ICONERROR);
#endif
			std::cerr << "Misslyckades med att initiera SDL: " << SDL_GetError() << std::endl;
		}
			
		// Borde och kan aldrig ske - men men..!
		else if (errCode == WINDOW_ERROR_ALREADY_INITIALIZED)
			std::cerr << "En instans av SDL har redan initierats.\n";

		
		return errCode;
	}
	
	Window::OnClose(KillAll);

	{
		errCode = InitMainConfig();

		if (errCode != SUCCESS)
		{
			std::cerr << "config.txt could not be not parsed!" << std::endl;
//			return errCode; // Non-critical now that we have default values
		}

		errCode = Window::OpenDynamic();

		if (errCode != SUCCESS)
		{
#ifdef WIN32
			MessageBoxA( NULL, "Window subsystem could not be initialized!", "Error", MB_OK | MB_ICONERROR);
#endif
			std::cerr << "Window subsystem could not be initialized!" << std::endl;
			return errCode;
		}

		Window::SetTitle(Utilities::mainConfig.GetValue("application header").c_str());

		if(Window::GUI::InitFont(Utilities::mainConfig.GetValue("default font")) != SUCCESS)
		{
#ifdef WIN32
			MessageBoxA( NULL, "Font Init Error.", "Error", MB_OK | MB_ICONERROR);
#endif
			cerr << "Font Init Error." << endl;
			return FONT_ERROR_FILE_LOAD;
		}

		Window::GUI::LoadWindow *loading = new Window::GUI::LoadWindow(5.0f);
		loading->SetMessage("Initializing Soundsytem");
		loading->Update();
	#if USE_AUDIO == 1
		if (!Game::Rules::noSound)
		{
			Audio::Init(Utilities::mainConfig.GetValue("audio config"));
		}
	#endif
		
		if(SDLNet_Init() == -1)
		{
			cout << "SDLNet_Init: " << SDLNet_GetError() << endl;
			return NETWORK_ERROR_INIT;
		}

		loading->Increment(1.0f);
		loading->SetMessage("Loading World");
		loading->Update();

		if (Utilities::mainConfig.GetValue("camera rotation speed").length())
		{
			float value = (float) atof(Utilities::mainConfig.GetValue("camera rotation speed").c_str());

			if (value > 0 && value < 100.0f)
				Game::Dimension::cameraRotationSpeed = value;
		}

		if (Utilities::mainConfig.GetValue("camera fly speed").length())
		{
			float value = (float) atof(Utilities::mainConfig.GetValue("camera fly speed").c_str());

			if (value > 0 && value < 100.0f)
				Game::Dimension::cameraFlySpeed = value;
		}

		if (Utilities::mainConfig.GetValue("camera zoom speed").length())
		{
			float value = (float) atof(Utilities::mainConfig.GetValue("camera zoom speed").c_str());

			if (value > 0 && value < 100.0f)
				Game::Dimension::cameraZoomSpeed = value;
		}

		loading->Increment(1.0f);
		loading->SetMessage("Loading GameMain...");
		loading->Update();
	}

	Game::Rules::GameMain();
	
	return SUCCESS;
}

std::string configFile("config.txt");

int InitMainConfig()
{
	std::cout << "Opening config file " << configFile << std::endl;
	Utilities::mainConfig.SetFile(configFile);
	Utilities::mainConfig.SetRestriction("screen width", new Utilities::ConfigurationFile::TypeRestriction<int>(800));
	Utilities::mainConfig.SetRestriction("screen height", new Utilities::ConfigurationFile::TypeRestriction<int>(600));
	Utilities::mainConfig.SetRestriction("screen bpp", new Utilities::ConfigurationFile::TypeRestriction<int>(32));
	Utilities::mainConfig.SetRestriction("fullscreen", new Utilities::ConfigurationFile::TypeRestriction<int>(0));
	
	// As the following are type restrictions to std::string they don't really restrict anything.
	// Instead, their use is to provide a default value.
	Utilities::mainConfig.SetRestriction("default font", new Utilities::ConfigurationFile::TypeRestriction<std::string>("fonts/vera.ttf"));
	Utilities::mainConfig.SetRestriction("application header", new Utilities::ConfigurationFile::TypeRestriction<std::string>("Nightfall (Codename Twilight)"));
	Utilities::mainConfig.SetRestriction("audio config", new Utilities::ConfigurationFile::TypeRestriction<std::string>("audio.txt"));
	
	Utilities::mainConfig.SetRestriction("camera rotation speed", new Utilities::ConfigurationFile::TypeRestriction<int>(40));
	Utilities::mainConfig.SetRestriction("camera fly speed", new Utilities::ConfigurationFile::TypeRestriction<int>(3));
	Utilities::mainConfig.SetRestriction("camera zoom speed", new Utilities::ConfigurationFile::TypeRestriction<int>(40));
	
	std::set<std::string> qualities;
	qualities.insert("low");
	qualities.insert("medium");
	qualities.insert("high");

	Utilities::mainConfig.SetRestriction("lighting quality", new Utilities::ConfigurationFile::EnumRestriction<std::string>(qualities, "medium"));
	Utilities::mainConfig.SetRestriction("model quality", new Utilities::ConfigurationFile::EnumRestriction<std::string>(qualities, "medium"));
	Utilities::mainConfig.SetRestriction("terrain quality", new Utilities::ConfigurationFile::EnumRestriction<std::string>(qualities, "medium"));
	Utilities::mainConfig.SetRestriction("effect quality", new Utilities::ConfigurationFile::EnumRestriction<std::string>(qualities, "medium"));

	int err = Utilities::mainConfig.Parse();

	for (std::map<std::string, std::string>::iterator it = mainConfigOverrides.begin(); it != mainConfigOverrides.end(); it++)
	{
		Utilities::mainConfig.SetValue(it->first, it->second);
	}

	return err;
}

void ParseArguments(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--dedicated-server"))
		{
			Game::Rules::startState = Game::Rules::NETWORKCREATE;
			Window::noWindow = true;
			Game::Rules::noGraphics = true;
			Game::Rules::noSound = true;
		}
		else if (!strcmp(argv[i], "--dedicated-client"))
		{
			Game::Rules::startState = Game::Rules::NETWORKJOIN;
			Window::noWindow = true;
			Game::Rules::noGraphics = true;
			Game::Rules::noSound = true;
		}
		else if (!strcmp(argv[i], "--server"))
		{
			Game::Rules::startState = Game::Rules::NETWORKCREATE;
		}
		else if (!strcmp(argv[i], "--client"))
		{
			Game::Rules::startState = Game::Rules::NETWORKJOIN;
		}
		else if (!strcmp(argv[i], "--start-game"))
		{
			Game::Rules::startState = Game::Rules::NEWGAME;
		}
		else if (!strcmp(argv[i], "--load-game"))
		{
			Game::Rules::startState = Game::Rules::LOADGAME;
		}
		else if (!strcmp(argv[i], "--no-graphics"))
		{
			Game::Rules::noGraphics = true;
		}
		else if (!strcmp(argv[i], "--no-window"))
		{
			Window::noWindow = true;
			Game::Rules::noGraphics = true;
		}
		else if (!strcmp(argv[i], "--no-sound"))
		{
			Game::Rules::noSound = true;
		}
		else if (!strcmp(argv[i], "--host"))
		{
			if (++i < argc)
			{
				Game::Rules::host = (std::string) argv[i];
			}
		}
		else if (!strcmp(argv[i], "--port"))
		{
			if (++i < argc)
			{
				std::stringstream ss(argv[i]);
				ss >> Game::Networking::netPort;
			}
		}
		else if (!strcmp(argv[i], "--checksum-log"))
		{
			if (++i < argc)
			{
				Game::Rules::checksumLog = (std::string) argv[i];
			}
		}
		else if (!strcmp(argv[i], "--player-goal"))
		{
			if (++i < argc)
			{
				std::stringstream ss(argv[i]);
				ss >> Game::Rules::numPlayersGoal;
			}
		}
		else if (!strcmp(argv[i], "--lua-threads"))
		{
			if (++i < argc)
			{
				std::stringstream ss(argv[i]);
				ss >> Game::AI::numLuaAIThreads;
			}
		}
		else if (!strcmp(argv[i], "--config-file"))
		{
			if (++i < argc)
			{
				configFile = argv[i];
			}
		}
		else if (!strcmp(argv[i], "--fail-safe"))
		{
			configFile = "failsafe-config.txt";
		}
		else if (!strcmp(argv[i], "--override"))
		{
			if (i < argc-2)
			{
				std::string key = argv[++i];
				std::string value = argv[++i];
				mainConfigOverrides[key] = value;
			}
		}
		else if (!strcmp(argv[i], "--level"))
		{
			if (++i < argc)
			{
				Game::Rules::CurrentLevel = argv[i];
			}
		}
	}
}

void KillAll(void)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

#if USE_AUDIO == 1
	Audio::Kill();
#endif

#if USE_FONT == 1
	Window::GUI::KillFontSystem();
#endif

	if (Game::Rules::GameWindow::IsNull() == false)
	{
		if (Game::Dimension::pWorld != NULL)
		{
			while(Game::Dimension::pWorld->vUnits.size() > 0)
				Game::Dimension::DeleteUnit(Game::Dimension::pWorld->vUnits.at(0));

			delete Game::Dimension::pWorld;
		}
	}

	Game::AI::QuitPathfindingThreading();

	SDLNet_Quit();
	SDL_Quit();

	cout << "attempted frames waiting percent: " << ((double) Game::Networking::attempted_frames_waited / Game::Networking::attempted_frame_count) * 100 << endl;
	
	cout << "average bytes sent per aiFrame: " << (double) Game::Networking::bytes_sent / Game::AI::currentFrame << endl;

	std::cout << "Goodbye!" << std::endl;
}
