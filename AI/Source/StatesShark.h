#pragma once

#include "State.h"
#include "GameObject.h"

class StateCrazy : public State
{
public:
	StateCrazy(const std::string& stateID);
	virtual ~StateCrazy();

	virtual void Enter(GameObject* go);
	virtual void Update(double dt, GameObject* go);
	virtual void Exit(GameObject* go);
};

class StateNaughty : public State
{
public:
	StateNaughty(const std::string& stateID);
	virtual ~StateNaughty();

	virtual void Enter(GameObject* go);
	virtual void Update(double dt, GameObject* go);
	virtual void Exit(GameObject* go);
};

class StateHappy : public State
{
public:
	StateHappy(const std::string& stateID);
	virtual ~StateHappy();

	virtual void Enter(GameObject* go);
	virtual void Update(double dt, GameObject* go);
	virtual void Exit(GameObject* go);
};
