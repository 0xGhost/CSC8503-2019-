#pragma once
#include "Human.h"
namespace NCL {
	namespace CSC8503 {
		class Watcher :
			public Human
		{
		public:
			Watcher(string name = "", string tag = "");

			virtual void Update(float dt) override;


		protected:
			StateMachine *stateMachine;

			GameObject *playerObject;
		};
	}
}