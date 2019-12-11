#pragma once
#include "State.h"

namespace NCL {
	namespace CSC8503 {
		enum PushdownResult {
			Push, Pop, PopUntilBottom, NoChange
		};
		class PushdownState;
		typedef std::function<PushdownResult(PushdownState**)> PDStateFunc;
		typedef std::function<void(void)> PDAwakeFunc;
		typedef std::function<void(void)> PDSleepFunc;
		class PushdownState 
		{
		public:

			PushdownState(PDStateFunc stateFunc);
			~PushdownState();
			
			PushdownResult PushdownUpdate(PushdownState** pushResult);
			void SetStateFunc(PDStateFunc f) { func = f; }
			void SetAwakeFunc(PDAwakeFunc func) { awakeFunc = func; }
			void SetSleepFunc(PDSleepFunc func) { sleepFunc = func; }

			virtual void OnSleep() { sleepFunc(); }
			virtual void OnAwake() { awakeFunc(); } 
			
		protected:
			PushdownState();
			PDStateFunc func;
			PDAwakeFunc awakeFunc;
			PDSleepFunc sleepFunc;
		};
	}
}

