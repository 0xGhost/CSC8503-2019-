#pragma once
#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine();
			~PushdownMachine();
			void InitState(PushdownState* init);
			void Update();

		protected:
			PushdownState * activeState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

