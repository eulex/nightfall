#include "game.h"

#include "model.h"
#include "dimension.h"
#include "font.h"
#include "aibase.h"
#include "vector3d.h"
#include "terrain.h"
#include "audio.h"
#include "unit.h"
#include "environment.h"
#include "console.h"
#include "gamegui.h"
#include "networking.h"
#include "textures.h"
#include "aipathfinding.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <sstream>

#define OVERLAY 1
#define SKYBOX 0

using namespace Window::GUI;

namespace Game
{
	namespace Rules
	{
		GameWindow* GameWindow::pInstance = NULL;

		GameWindow* GameWindow::Instance()
		{
			if (GameWindow::pInstance == NULL)
				GameWindow::pInstance = new GameWindow;

			return GameWindow::pInstance;
		}
		
		bool GameWindow::IsNull()
		{
			if (GameWindow::pInstance == NULL)
				return true;
				
			return false;
		}

		void GameWindow::Destroy()
		{
			if (GameWindow::pInstance == NULL)
				return;

			delete GameWindow::pInstance;
			GameWindow::pInstance = NULL;
		}

		Dimension::Camera* GameWindow::GetCamera() const
		{
			return worldCamera;
		}

		Window::GUI::ConsoleBuffer* GameWindow::GetConsoleBuffer() const
		{
			return pConsole;
		}

		void PrintGLError()
		{
			bool is_first_error = true;
			std::cout << "OpenGL errors:" << std::endl;
			while (1)
			{
				int error = glGetError();
				switch(error)
				{
					case GL_NO_ERROR:
					{
						if (is_first_error)
						{
							std::cout << "No errors" << std::endl;
						}
						return;
					}
					case GL_INVALID_OPERATION:
					{
						std::cout << "Invalid Operation" << std::endl;
						break;
					}
					case GL_STACK_OVERFLOW:
					{
						std::cout << "Stack overflow" << std::endl;
						break;
					}
					case GL_STACK_UNDERFLOW:
					{
						std::cout << "Stack underflow" << std::endl;
						break;
					}
					case GL_INVALID_ENUM:
					{
						std::cout << "Invalid Enum" << std::endl;
						break;
					}
					case GL_INVALID_VALUE:
					{
						std::cout << "Invalid Value" << std::endl;
						break;
					}
					default:
					{	
						std::cout << "Unknown error..." <<  std::endl;
						break;
					}

				}
				is_first_error = false;
			}
		}
		
		float time_since_last_frame;
		float time_passed_since_last_water_pass = 0.0;
		
//////////////////////
//////////////////////
//////////////////////

//New GUI Code

//////////////////////
//////////////////////
//////////////////////

		int GameMain()
		{
			GameMenu *pMenu = new GameMenu();
			GameInGameMenu *pInGameMenu = new GameInGameMenu();
			SwitchState nextState = MENU;
			GameWindow *mainWindow = NULL;
			//GameTest *mainTestin = new GameTest();
			NetworkJoinOrCreate *multiplayer = new NetworkJoinOrCreate();
			NetworkJoin *networkJoin = new NetworkJoin();
			NetworkCreate *networkCreate = new NetworkCreate();

			if (Game::Networking::isDedicatedServer)
				nextState = NETWORKCREATE_DEDICATED;

			while(nextState != QUIT)
			{
				switch(nextState)
				{
					case MENU:
					{
						Audio::PlayList("menu");
						nextState = (SwitchState)pMenu->RunLoop();
						Audio::StopAll();
						break;
					}
					case NEWGAME:
					{
						mainWindow = GameWindow::Instance();
						mainWindow->NewGame();
						nextState = GAME;
						break;
					}
					case ENDGAME:
					{
						mainWindow = GameWindow::Instance();
						mainWindow->EndGame();
						nextState = MENU;
						break;
					}
					case GAME:
					{
						Audio::PlayList("ingame");
						mainWindow = GameWindow::Instance();
						nextState = (SwitchState)mainWindow->RunLoop();
						Audio::StopAll();
						break;
					}
					case INGAMEMENU:
					{
						Audio::PlayList("menu");
						nextState = (SwitchState)pInGameMenu->RunLoop();
						Audio::StopAll();
						break;
					}
					case MULTIPLAYER:
					{
						Networking::isNetworked = true;
						nextState = (SwitchState)multiplayer->RunLoop();
						if(nextState == MENU)
							Networking::isNetworked = false;
						break;
					}
					case NETWORKCREATE:
					{
						mainWindow = GameWindow::Instance();
						mainWindow->NewGame();
						Networking::StartNetwork(Networking::SERVER);
						nextState = (SwitchState)networkCreate->RunLoop();
						if(nextState == MENU)
							mainWindow->EndGame();
						break;
					}
					case NETWORKJOIN:
					{
						mainWindow = GameWindow::Instance();
						mainWindow->NewGame();
						Networking::StartNetwork(Networking::CLIENT);
						nextState = (SwitchState)networkJoin->RunLoop();
						if(nextState == MENU)
							mainWindow->EndGame();
						break;
					}
					default:
					{
						nextState = QUIT;
						break;
					}
				}
			}
			GameWindow::Destroy();
			delete pMenu;
			return SUCCESS;
		}

		void WaitForNetwork()
		{
			Window::GUI::LoadWindow *pLoading = new Window::GUI::LoadWindow(1.0f);
			pLoading->SetMessage("Awaiting network...");
			pLoading->SetProgress(0.0f);
			float counter = 0.0f;
			while(Networking::isReadyToStart == false)
			{
				Networking::PerformPregameNetworking();
				counter += 0.05f;
				if(counter > 1.0f)
					counter = 0.0f;

				pLoading->SetProgress(counter);
				pLoading->Update();
				SDL_Delay(16);
			}
			delete pLoading;
		}

		GameWindow::GameWindow()
		{
			frames = 0;
			time = 0;
			start_drag_x = 0;
			start_drag_y = 0;
			end_drag_x = 0;
			end_drag_y = 0;

			is_pressing_lmb = false;
			last_frame = 0;
			this_frame = SDL_GetTicks();

			time_passed_since_last_ai_pass = 0.0f;
			goto_x = 0.0f;
			goto_y = 0.0f;
			
			build_x = -1;
			build_y = -1;
				
			build_type = NULL;
			goto_time = 0;
			pause = 0;

			buildingUnit = NULL;
			pGameInput = NULL;

			part_x = 160.0f;
			part_y = 70.0f;

			gameRunning = false;
		}

		int GameWindow::InitGUI(SDL_Surface* img)
		{
			float increment = 0.1f / 3.0f; //0.1 (10%) divided on 1 update;

			//Initate Panel and place GameInput
			pMainGame = new Window::GUI::Panel();
			pGameInput = new GameInput(this);

			SetPanel(pMainGame);
			float sizes[2] = {0.04f, 0.2f};
			
			pConsole = new Window::GUI::ConsoleBuffer();
			consoleID = pMainGame->Add(pConsole);

			pMainGame->SetConstraintPercent(consoleID, 0.0f, 0.0f, 1.0f, 1.0f);
			pMainGame->SetVisible(consoleID, false);

			int mainWidget = pMainGame->Add(pGameInput);
			pMainGame->SetConstraintPercent(mainWidget, 0.0f, sizes[0], 1.0f, 1.0f - sizes[0] - sizes[1]);
			pMainGame->SetFocus(mainWidget);
			pMainGame->SetFocusEnabled(false);

			pTopBar = new GameTopBar();
			pPlayBar = new GamePlayBar(this, img);

			int topBar = pMainGame->Add(pTopBar);
			pMainGame->SetConstraintPercent(topBar, 0.0f, 0.0f, 1.0f, sizes[0]);

			p_lblMoney = new Window::GUI::InfoLabel();
			p_lblMoney->AttachHandler(&GetMoney);
			p_lblMoney->SetTooltip("Resource: Money");

			p_lblPower = new Window::GUI::InfoLabel();
			p_lblPower->AttachHandler(&GetPower);
			p_lblPower->SetTooltip("Resource: Power");

			p_lblTime = new Window::GUI::InfoLabel();
			p_lblTime->AttachHandler(&GetTime);
			p_lblTime->SetTooltip("Time");

			sell = new Window::GUI::TextButton();
			sell->SetText("Sell");
			sell->AttachHandler(&Sell);
			sell->SetTag(this);
			sell->SetType(1);

			int lblmoney = pTopBar->Add(p_lblMoney);
			int lblpower = pTopBar->Add(p_lblPower);
			int lbltime = pTopBar->Add(p_lblTime);
			int sellid = pTopBar->Add(sell);

			pTopBar->SetConstraintPercent(lblpower, 0.0f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(lblmoney, 0.25f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(lbltime, 0.5f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(sellid, 0.75f, 0.0f, 0.25f, 0.8f);

			int playBar = pMainGame->Add(pPlayBar);
			pMainGame->SetConstraintPercent(playBar, 0.0f, 1.0f - sizes[1], 1.0f, sizes[1]);

			pLoading->Increment(increment);

			pTopBar->init();
			
			pLoading->Increment(increment);

			pPlayBar->init();
			pPlayBar->SwitchSelected(NULL);

			pConsole->SetType(0);
			pConsole->PrepareBuffer();
			console.WriteLine("Nightfall (Codename Twilight)");
			
			/*
			PrintGLError();

			glViewport(2,2,258,258);
			glClearColor(0.4f,0.2f,0.8f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			
			worldCamera->PointCamera();
			PaintGame();
			
			Utilities::SwitchTo2DViewport(w, h);
			glBegin(GL_QUADS);
				glColor4f(0.5f,0.5f,0.5f, 0.5f);

				glVertex2f(0.0f,0.0f);
				glVertex2f(0.0f,0.5f);
				glVertex2f(1.0f,0.5f);
				glVertex2f(1.0f,0.0f);
			glEnd();
			
			Utilities::RevertViewport();
			//SDL_GL_SwapBuffers();
			
			PrintGLError();
			
			glEnable(GL_TEXTURE_2D);
			
			GLuint textur = 0;
			PrintGLError();	
   			glGenTextures(1, &textur);
   			glBindTexture(GL_TEXTURE_2D, textur);
   			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			*/

			/*
			unsigned int *pixels = new unsigned int[256*256];
			glReadPixels(2,2,256,256,GL_RGB,GL_UNSIGNED_INT, (GLvoid*)pixels);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 256,256, GL_RGB, GL_UNSIGNED_INT, (void*)pixels);
			*/

			// Copies the contents of the frame buffer into the texture
			/*
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 256, 256, 0);
			
			pic = new Window::GUI::Picture();
			pMainGame->SetConstraint(pMainGame->Add(pic), 0.1f, 0.1f, 0.5f, 0.5f);
			pic->SetPicture(textur);
			PrintGLError();	

			glViewport(0,0,Window::windowWidth, Window::windowHeight);

			glClearColor(0.0f,0.0f,0.0f,1.0f);
			*/

			pLoading->Increment(increment);

			return SUCCESS;
		}
		
		void GameWindow::Sell(Window::GUI::EventType evt, void* arg)
		{
			if (Networking::isNetworked)
			{
				Networking::PrepareSell(Dimension::GetCurrentPlayer(), 100.0);
			}
			else
			{
				Dimension::SellPower(Dimension::GetCurrentPlayer(), 100.0);
			}
		}

		void GameWindow::DestroyGUI()
		{
			//Initate Panel and place GameInput
			delete pMainGame;
			delete pGameInput;
			delete pConsole;
			delete pPlayBar;
			delete pTopBar;
			delete p_lblMoney;
			delete p_lblPower;
			delete p_lblTime;
			delete sell;
		}

		GameWindow::~GameWindow()
		{
			//GUI Elements
			if(gameRunning == true)
				DestroyGUI();
		}
		
		void GameWindow::NewGame()
		{
#ifdef USE_MULTITHREADED_CALCULATIONS
			Game::AI::InitPathfindingThreading();
#endif
			pLoading = new Window::GUI::LoadWindow(1.0f); //90% Game, 10% GUI
			pLoading->SetMessage("Loading...");
			pLoading->Update();

			InitGame();
			
			/*
			SDL_Surface* img = Utilities::LoadImage("resources/textures/terrain2.png");
			Dimension::terraintexture = Utilities::CreateGLTexture(img);
			
			InitGUI(img);
			
			SDL_FreeSurface(img);
			*/

			delete pLoading;
			gameRunning = true;
		}

		void GameWindow::EndGame()
		{
#ifdef USE_MULTITHREADED_CALCULATIONS
			AI::QuitPathfindingThreading();
#endif

			//Deletes terrain, water, player and unit.
			Dimension::UnloadWorld();
			Dimension::Environment::FourthDimension::Destroy();
			DestroyGUI();

			delete this->worldCamera;
			delete this->input;
			delete FX::pParticleSystems;
			gameRunning = false;
		}

		int GameWindow::InitGame()
		{
			input = new Dimension::InputController();
			float increment = 0.9f / 8.0f; //0.9 (90%) divided on 6 updates...
			Dimension::pWorld = new Dimension::World;
			
			Utilities::Scripting::LuaVirtualMachine* const pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Utilities::InitTextures(256);
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM->DoFile("resources/levels/default_players.lua");
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Dimension::InitUnits();
			pLoading->Increment(increment);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			worldCamera = new Dimension::Camera();
			/*

			if (Dimension::GetCurrentPlayer()->vUnits.size())
			worldCamera->SetCamera(Dimension::GetCurrentPlayer()->vUnits.at(0), 30.0f, -40.0f);
			else
			worldCamera->SetCamera(Utilities::Vector3D(7.0, 0.0, -13.0), 30.0f, -40.0f);

			*/
			worldCamera->SetYMinimum(0.5f);
			worldCamera->SetYMaximum(15.0f);

			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM->DoFile("resources/levels/default_level.lua");
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (Dimension::pWorld->vPlayers.size() == 0)
				Dimension::InitPlayers();
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			FX::pParticleSystems = new Game::FX::ParticleSystemHandler(100, 100);
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pDimension->SetDefaultSun();
			
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			//  Enable smooth shading 
			glShadeModel( GL_SMOOTH );
			
			//  Depth buffer setup 
			glClearDepth( 1.0f );

			// Enable depth testing
			glEnable(GL_DEPTH_TEST);
		
			//  The Type Of Depth Test To Do 
			glDepthFunc( GL_LEQUAL );

			glEnable(GL_CULL_FACE);

			// Enable 2d texturing
			glEnable(GL_TEXTURE_2D);

			//  Really Nice Perspective Calculations 
			glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

/*
 			// Enable ambient light for Light 1
 			glLightfv(GL_LIGHT1, GL_AMBIENT,  Dimension::lightAmbient);
 
 			// Enable diffuse light for Light 1
 			glLightfv(GL_LIGHT1, GL_DIFFUSE,  Dimension::lightDiffuse);
 
 			// Set the position of Light 1
 			glLightfv(GL_LIGHT1, GL_POSITION, Dimension::sunPosition);
*/
			
			glEnable(GL_ALPHA);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

/*
 			// Enable Light 1
 			glEnable(GL_LIGHT1);
 
 			// Enable lighting
 			glEnable(GL_LIGHTING);
*/
			return SUCCESS;
			
			
			/*
			Dimension::LoadWorld("resources/maps/heightmap.pgm");

			SDL_Surface* img = Utilities::LoadImage("resources/textures/terrain2.png");
			Dimension::terraintexture = Utilities::CreateGLTexture(img);
			
			InitGUI(img);
			
			SDL_FreeSurface(img);
			*/
			
			/*
			{
				Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
				
//				pDimension->AllocSkyboxContainer(2);
				if (!pDimension->LoadSkyboxes())
					exit(ENVIRONMENT_SKYBOX_LOAD_FAILED);

				pDimension->InitSkyBox(30, 9);
				int day   = pDimension->GetSkybox("day01"); //pDimension->LoadSkyBox("resources/textures/day01.png");
				int night = pDimension->GetSkybox("night01"); //pDimension->LoadSkyBox("resources/textures/night01.PNG");

				if (day == -1 || night == -1)
					exit(ENVIRONMENT_SKYBOX_REFERENCE_NOT_FOUND);

				pDimension->SetHourLength(3.0f);
				pDimension->SetDayLength(24);

				Dimension::Environment::EnvironmentalCondition* envDay = new Dimension::Environment::EnvironmentalCondition;
				envDay->hourBegin = 6;
				envDay->hourEnd = 18;
				envDay->isDay = true;
				envDay->musicListTag = "musicDay";
				envDay->skybox = day;
				
				envDay->sunPos[0] = 1024.0f; 
				envDay->sunPos[1] = 1024.0f; 
				envDay->sunPos[2] = 1024.0f; 
				envDay->sunPos[3] = 1.0f;
				
				envDay->diffuse[0] = 1.0f;
				envDay->diffuse[1] = 1.0f;
				envDay->diffuse[2] = 1.0f;
				envDay->diffuse[3] = 1.0f;

				envDay->ambient[0] = 1.0f;
				envDay->ambient[1] = 1.0f;
				envDay->ambient[2] = 1.0f;
				envDay->ambient[3] = 1.0f;

				envDay->fogBegin = 12.0f;
				envDay->fogEnd = 14.0f;
				envDay->fogIntensity = 0.15f;
				envDay->fogColor[0] = -1.0f;
				envDay->fogColor[1] = -1.0f;
				envDay->fogColor[2] = -1.0f;
				envDay->fogColor[3] = -1.0f;
  
				Dimension::Environment::EnvironmentalCondition* envNight = new Dimension::Environment::EnvironmentalCondition;
				envNight->hourBegin = 18;
				envNight->hourEnd = 6;
				envNight->isNight = true;
				envNight->musicListTag = "musicNight";
				envNight->skybox = night;

				envNight->sunPos[0] = 1024.0f; 
				envNight->sunPos[1] = 1024.0f; 
				envNight->sunPos[2] = 1024.0f; 
				envNight->sunPos[3] = 1.0f;
				
				envNight->diffuse[0] = 0.2f;
				envNight->diffuse[1] = 0.2f;
				envNight->diffuse[2] = 0.2f;
				envNight->diffuse[3] = 1.0f;

				envNight->ambient[0] = 0.2f;
				envNight->ambient[1] = 0.2f;
				envNight->ambient[2] = 0.2f;
				envNight->ambient[3] = 1.0f;

				envNight->fogBegin = 8.5f;
				envNight->fogEnd = 12.5f;
				envNight->fogIntensity = 0.4f;
				envNight->fogColor[0] = -1.0f;
				envNight->fogColor[1] = -1.0f;
				envNight->fogColor[2] = -1.0f;
				envNight->fogColor[3] = -1.0f;
				
				pDimension->AddCondition(envDay);
				pDimension->AddCondition(envNight);

				if (!pDimension->ValidateConditions())
				{
					cout << "Failed to initialize conditions, exiting..." << endl;
					exit(ENVIRONMENT_INVALID_CONDITIONS);
				}
				
				pDimension->SetCurrentHour(6);
			}
			*/
		}

		bool GameWindow::ProcessEvents()
		{
			//SDL Events
			SDL_Event *event = new SDL_Event;
			while (SDL_PollEvent(event))
			{
				if(pMainPanel != NULL)
				{
					switch(event->type)
					{
						case SDL_KEYDOWN:
						{
							pMainPanel->HandleEvent(KB_DOWN, event, NULL);
							break;
						}
						case SDL_KEYUP:
						{
							pMainPanel->HandleEvent(KB_UP, event, NULL);
							break;
						}
						case SDL_MOUSEMOTION:
						{
							//Translate position
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_MOVE, event, ptr = TranslateMouseCoords(event->motion.x, event->motion.y));
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONDOWN:
						{
							TranslatedMouse *ptr;
							if((*event).button.button == SDL_BUTTON_WHEELUP || (*event).button.button == SDL_BUTTON_WHEELDOWN)
							{
								pMainPanel->HandleEvent(MOUSE_SCROLL, event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							}
							else
							{
								pMainPanel->HandleEvent(MOUSE_DOWN, event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							}
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONUP:
						{
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_UP,event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							delete ptr;
							break;
						}
					}
				}

				switch(event->type)
				{
#ifdef MAC
					case SDL_EVENT_DUMP_CONSOLE:
					{
						fstream stream("console.txt");
						if (!stream.bad())
						{
							stream << console.GetBuffer();
						}
						stream.close();
						break;
					}
#endif
					case SDL_QUIT:
					{
						go = false;
						break;	
					}
				}
			}

			delete event;
	
			return true;
		}

		void GameWindow::PaintGame()
		{
			worldCamera->PointCamera();
			Dimension::Environment::FourthDimension::Instance()->ApplyLight();
			Dimension::Environment::FourthDimension::Instance()->RenderSkyBox();
			Dimension::DrawTerrain();
			Dimension::RenderUnits();
			Dimension::DrawWater();
			//Particle test
			FX::pParticleSystems->Render();

			Dimension::RenderHealthBars();

			Audio::PlaceSoundNodes(*worldCamera->GetPosVector());

			if (goto_time != 0 && SDL_GetTicks() - goto_time < 1000)
			{
				Utilities::Vector3D ter[2][2]; 

				ter[0][0] = Dimension::GetTerrainCoord(goto_x, goto_y);
				ter[0][1] = Dimension::GetTerrainCoord(goto_x+1, goto_y);
				ter[1][0] = Dimension::GetTerrainCoord(goto_x, goto_y+1);
				ter[1][1] = Dimension::GetTerrainCoord(goto_x+1, goto_y+1);
				
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBegin(GL_QUADS);
					glColor4f(1.0f, 0.0f, 0.0f, 1.0 - (SDL_GetTicks() - goto_time) / 1000.0);
					glNormal3f(0.0f, 1.0f, 0.0f);
					glVertex3f(ter[0][0].x, ter[0][0].y, ter[0][0].z);
					glVertex3f(ter[1][0].x, ter[1][0].y, ter[1][0].z);
					glVertex3f(ter[1][1].x, ter[1][1].y, ter[1][1].z);
					glVertex3f(ter[0][1].x, ter[0][1].y, ter[0][1].z);
					
					/*
					glColor4f(1.0f, 1.0f, 0.0f, 0.45f);
					for (size_t i = 0; i < Dimension::unitsSelected.size(); i++)
					{
						Dimension::Unit* unit = Dimension::unitsSelected.at(i);
						
						if (unit->pMovementData->pStart != NULL &&
							unit->pMovementData->calcState == AI::CALCSTATE_REACHED_GOAL)
						{
							AI::Node* node = unit->pMovementData->pStart;
							
							do
							{
								ter[0][0] = Dimension::GetTerrainCoord(node->x, node->y);
								ter[0][1] = Dimension::GetTerrainCoord(node->x + 1, node->y);
								ter[1][0] = Dimension::GetTerrainCoord(node->x, node->y + 1);
								ter[1][1] = Dimension::GetTerrainCoord(node->x + 1, node->y + 1);
								
								glVertex3f(ter[0][0].x, ter[0][0].y, ter[0][0].z);
								glVertex3f(ter[1][0].x, ter[1][0].y, ter[1][0].z);
								glVertex3f(ter[1][1].x, ter[1][1].y, ter[1][1].z);
								glVertex3f(ter[0][1].x, ter[0][1].y, ter[0][1].z);
								
								node = node->pChild;
							} while (node != NULL);
						}
					}
					*/
				glEnd();
					
				glEnable(GL_LIGHTING);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_TEXTURE_2D);
			}

			if (build_x != -1 && build_y != -1 && build_type)
			{
				Dimension::RenderBuildOutline(build_type, build_x, build_y);
			}	
			
		}

		bool GameWindow::PaintAll()
		{
			// nollställ backbufferten och depthbufferten
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// nollställ vyn
			glLoadIdentity();

			PaintGame();

			if(pMainPanel != NULL)
			{
				Utilities::SwitchTo2DViewport(w, h);
				pMainPanel->Paint();
				pMainPanel->PaintTooltip();
				Utilities::RevertViewport();
			}
			
			SDL_GL_SwapBuffers();
			return true;
		}

		void GameWindow::PerformPreFrame()
		{
			if (last_frame != this_frame)
			{
				last_frame = this_frame;
			}
			this_frame = SDL_GetTicks();
			time_since_last_frame = (this_frame - last_frame) / 1000.0;

			time_passed_since_last_ai_pass += time_since_last_frame;

			if (!Game::Networking::isDedicatedServer)
			{
				time_passed_since_last_water_pass += time_since_last_frame;
				while (time_passed_since_last_water_pass >= (1.0 / 3))
				{
					Dimension::CalculateWater();
					time_passed_since_last_water_pass -= (1.0 / 3);
				}
			}


			while (time_passed_since_last_ai_pass >= (1 / AI::aiFps))
			{

				Uint32 before_ai_pass = SDL_GetTicks();
				AI::PerformAIFrame();
				Uint32 after_ai_pass = SDL_GetTicks();
				float time_ai_pass = (after_ai_pass - before_ai_pass) / 1000.0;

				time_passed_since_last_ai_pass -= (1 / (float) AI::aiFps);
				time_since_last_frame -= time_ai_pass;
			}

			//Key States operations
			if (input->GetKeyState(SDLK_UP))
				worldCamera->Fly(-1.5f * time_since_last_frame);
					
			else if (input->GetKeyState(SDLK_DOWN))
				worldCamera->Fly(1.5f * time_since_last_frame);

				
			if (input->GetKeyState(SDLK_LEFT))
				worldCamera->FlyHorizontally(-Game::Dimension::cameraFlySpeed * time_since_last_frame);					
			else if (input->GetKeyState(SDLK_RIGHT))
				worldCamera->FlyHorizontally(Game::Dimension::cameraFlySpeed * time_since_last_frame);
				
			if (input->GetKeyState(SDLK_PAGEUP))
				worldCamera->Zoom(Game::Dimension::cameraZoomSpeed * time_since_last_frame);				
			else if (input->GetKeyState(SDLK_PAGEDOWN))
				worldCamera->Zoom(-Game::Dimension::cameraZoomSpeed * time_since_last_frame);

			if (input->GetKeyState(SDLK_HOME))
				worldCamera->Rotate(Game::Dimension::cameraRotationSpeed * time_since_last_frame);		
			else if (input->GetKeyState(SDLK_END))
				worldCamera->Rotate(-Game::Dimension::cameraRotationSpeed * time_since_last_frame);

			//Empty current UnitBuild and execute building
			if(Dimension::unitsSelected.size() == 0)
			{
				if(buildingUnit != NULL)
				{
					buildingUnit = NULL;
					//buildingGUI = NULL;
					//Window::GUI::PanelWidget obj;
					//obj.pPanel = pPlayBar;
					pPlayBar->SwitchSelected(NULL);
					//pMainGame->SetElement(selection_panel, obj, typePanel);
				}
			}
			else
			{
				Dimension::Unit* unit = Dimension::unitsSelected.at(0);
				pPlayBar->pSelected->Update();
				pPlayBar->pActions->Update();

				if(buildingUnit != unit)
				{
					pPlayBar->SwitchSelected(unit);
					buildingUnit = unit;
/*
					map<Dimension::UnitType*, UnitBuild*>::iterator match = GUI_Build.find(unit->type);
					if(match == GUI_Build.end())
					{
						buildingUnit = unit;
						buildingGUI = NULL;
					}
					else
					{
						buildingUnit = unit;
						buildingGUI = (*match).second;
						Window::GUI::PanelWidget obj;
						obj.pPanel = buildingGUI;
						pMainGame->SetElement(selection_panel, obj, typePanel);
						buildingGUI->SetUnit(unit);
					}
*/
				}
			}
		}

		int GameWindow::RunLoop()
		{
			if (!Game::Networking::isDedicatedServer)
				glClearColor( 0.2f, 0.2f, 0.2f, 0.7f );

			go = true;
			while(go)
			{
				PerformPreFrame();
				ProcessEvents();

				if (!Game::Networking::isDedicatedServer)
					PaintAll();
			
				frames++;

				if (SDL_GetTicks() - time >= 1000)
				{
					if (Game::Networking::isDedicatedServer)
						cout << "Fps: " << (frames / (((float) (SDL_GetTicks() - time)) / 1000)) << " " << Dimension::pWorld->vUnits.size() << endl;
					else
						console << "Fps: " << (frames / (((float) (SDL_GetTicks() - time)) / 1000)) << " " << Dimension::pWorld->vUnits.size() << Console::nl;

					frames = 0;
					time = SDL_GetTicks();
				}

				if (Game::Networking::isDedicatedServer)
					SDL_Delay(1);
			}
			return returnValue;
		}
	}
}
