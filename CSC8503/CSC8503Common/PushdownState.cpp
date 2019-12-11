#include "PushdownState.h"

using namespace NCL::CSC8503;

PushdownState::PushdownState(PDStateFunc stateFunc)
{
	func = stateFunc;
	awakeFunc = []() {};
	sleepFunc = []() {};
}

NCL::CSC8503::PushdownState::PushdownState()
{
	awakeFunc = []() {};
	sleepFunc = []() {};
}


PushdownState::~PushdownState()
{
}

PushdownResult PushdownState::PushdownUpdate(PushdownState** pushResult) {

	return func(pushResult);
}