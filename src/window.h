#ifndef __WINDOW_H_PRE__
#define __WINDOW_H_PRE__

#ifdef DEBUG_DEP
#warning "window.h-pre"
#endif

#define __WINDOW_H_PRE_END__

#include "utilities.h"

#endif

#ifdef __UTILITIES_H_PRE_END__

#ifndef __WINDOW_H__
#define __WINDOW_H__

#ifdef DEBUG_DEP
#warning "window.h"
#endif

#include "sdlheader.h"

namespace Window
{
	// Jag är inte säker på att vi behöver extern
	// i namespace. Åja, ren gissning.
	// Om vi stöter på problem räcker det ju med
	// att helt enkelt ta bort kommentarerna.
	//
	// Notera dock: initialized måste initieras.
	//              blir "unknown symbol exception"
	//              annars när man kallar Init.
	extern SDL_Surface* pWindow;
	extern bool initialized;

	extern int windowWidth, windowHeight;
	extern float guiWidth, guiHeight, guiResolution;
	
	// Initierar SDL
	int   Init(int flags);
	
	// Event called when closing the window
	void  OnClose(void (*fptr)(void));
	
	// Stäng ned programmet - rensa!
	void  Exit(void);	
	
	// Öppna ett fönster genom att läsa innehållet ur en konfigurationsfil. 
	// Kallar InitStatic efter värdena har lästs.
	int   OpenDynamic(Utilities::ConfigurationFile* pConfigInterpreter);
	
	// Öppnar ett fönster via statiska värden.
	int   OpenStatic(int width, int height, int bpp, bool fullscreen);
	
	// Sätter titeln på fönstret vilket namnger därmed programmet
	void  SetTitle(const char* title);

	// Sparar screenshot i ./screenshots
	void  MakeScreenshot(void);
}

#ifdef DEBUG_DEP
#warning "window.h-end"
#endif

#define __WINDOW_H_END__

#endif
#endif
