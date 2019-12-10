#pragma once

#include "..//CSC8503Common/PushdownMachine.h"

namespace NCL {
	namespace CSC8503 {
		class Menu
		{
		public:
			Menu();
			~Menu();

			void MenuUpdate(float dt) { machine.Update(); }


		protected:
			PushdownMachine machine;
			PushdownState *mainMenu;
		};
	}
}

