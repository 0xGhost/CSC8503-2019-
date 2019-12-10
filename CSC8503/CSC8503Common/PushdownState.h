#pragma once
#include "State.h"

namespace NCL {
	namespace CSC8503 {
		enum PushdownResult {
			Push, Pop, NoChange
		};
		class PushdownState;
		typedef std::function<PushdownResult(PushdownState**)> PDStateFunc;
		class PushdownState //:public State
		{
		public:

			PushdownState(PDStateFunc stateFunc);
			~PushdownState();
			
			PushdownResult PushdownUpdate(PushdownState** pushResult);

			virtual void OnAwake() { ; } //By default do nothing
			virtual void OnSleep() { ; } //By default do nothing
		protected:
			PDStateFunc func;
			void* funcData;
		};
	}
}

