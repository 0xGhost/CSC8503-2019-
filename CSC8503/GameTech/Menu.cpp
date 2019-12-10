#include "Menu.h"
#include "../CSC8503Common/PushdownState.h"

NCL::CSC8503::Menu::Menu()
{
	mainMenu = new PushdownState([](PushdownState** pushResult)
		{
			return NoChange;
		});
}

NCL::CSC8503::Menu::~Menu()
{
	delete mainMenu;
}
