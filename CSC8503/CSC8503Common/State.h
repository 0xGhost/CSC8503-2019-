#pragma once
#include <functional>
namespace NCL {
	namespace CSC8503 {
		class State		{
		public:
			State() {}
			virtual ~State() {}
			virtual void Update() = 0; //Pure virtual base class
		};

		//typedef void(*StateFunc)(void*);

		typedef std::function<void(void*)> StateFunc;
		

		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* someData) {
				func		= someFunc;
				funcData	= someData;
			}
			virtual void Update() {
				if (funcData != nullptr) {
					func(funcData);
				}
			}
		protected:
			StateFunc func;
			void* funcData;
		};

		typedef std::function<void()> StateFunc2;
		class State2 : State {
		public:
			State2(StateFunc2 f)
			{
				func = f;
			}
			virtual void Update() { func(); } //Pure virtual base class
		protected:
			StateFunc2 func;
		};
	}
}