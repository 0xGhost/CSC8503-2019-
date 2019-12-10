#include "PushdownState.h"

using namespace NCL::CSC8503;

PushdownState::PushdownState(PDStateFunc stateFunc)
{
	func = stateFunc;
}


PushdownState::~PushdownState()
{
}

PushdownResult PushdownState::PushdownUpdate(PushdownState** pushResult) {

	return func(pushResult);
}