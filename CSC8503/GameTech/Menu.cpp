#include "Menu.h"
#include "GameTechRenderer.h"
#include "../CSC8503Common/PushdownState.h"

NCL::CSC8503::Menu::Menu()
{
	mainMenu = new PushdownState([](PushdownState** pushResult)
		{
			int i = 1;
			int offset = 20;
			Debug::Print("F1 to start a single-player game", Vector2(10, offset + (i++) * 20));
			Debug::Print("F2 to start a multi-player game", Vector2(10, offset + (i++) * 20));
			Debug::Print("F3 to load map", Vector2(10, offset + (i++) * 20));
			Debug::Print("F4 to create a emtpy map", Vector2(10, offset + (i++) * 20));
			Debug::Print("ESC to exit the game", Vector2(10, offset + (i++) * 20));
			return NoChange;
		});
}

NCL::CSC8503::Menu::~Menu()
{
	delete mainMenu;
}
