#pragma once
#include <functional>
#include "StateMachine.h"
namespace NCL {
	namespace CSC8503 {
		class State		{
		public:
			State() {}
			virtual ~State() {}
			virtual void Update() = 0; //Pure virtual base class
		};

		//typedef void(*StateFunc)(void*);

		typedef std::function<void()> StateFunc;
		

		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc) {
				func		= someFunc;
				//funcData	= someData;
			}
			virtual void Update() {
				//if (funcData != nullptr) {
					func();
				//}
			}
		protected:
			StateFunc func;
			//void* funcData;
		};

		//typedef std::function<void()> SuperStateFunc;
		
		class SuperState : public State {
		public:
			SuperState(StateFunc f, StateMachine *s)
			{
				func = f;
				stateMachine = s;
			}
			virtual void Update() {
				func(); 
				if(stateMachine)
				stateMachine->Update();
			}

		protected:
			StateFunc func;
			StateMachine *stateMachine;
		};


	}
}