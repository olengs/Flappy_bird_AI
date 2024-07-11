#pragma once

#include "State.h"
#include "GameObject.h"

class StateEvolve : public State
{
public:
	StateEvolve(const std::string& stateID);
	virtual ~StateEvolve();

	virtual void Enter(GameObject* go);
	virtual void Update(double dt, GameObject* go);
	virtual void Exit(GameObject* go);
};

class StateGrow : public State
{
public:
	StateGrow(const std::string& stateID);
	virtual ~StateGrow();

	virtual void Enter(GameObject* go);
	virtual void Update(double dt, GameObject* go);
	virtual void Exit(GameObject* go);
};
