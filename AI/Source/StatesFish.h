#ifndef STATES_FISH_H
#define STATES_FISH_H

#include "State.h"
#include "GameObject.h"

class StateTooFull : public State
{
public:
	StateTooFull(const std::string& stateID);
	virtual ~StateTooFull();

	virtual void Enter(GameObject*);
	virtual void Update(double dt, GameObject*);
	virtual void Exit(GameObject*);
};

class StateFull : public State
{
public:
	StateFull(const std::string& stateID);
	virtual ~StateFull();

	virtual void Enter(GameObject*);
	virtual void Update(double dt, GameObject*);
	virtual void Exit(GameObject*);
};

class StateHungry : public State
{
public:
	StateHungry(const std::string& stateID);
	virtual ~StateHungry();

	virtual void Enter(GameObject*);
	virtual void Update(double dt, GameObject*);
	virtual void Exit(GameObject*);
};

class StateDead : public State
{
public:
	StateDead(const std::string& stateID);
	virtual ~StateDead();

	virtual void Enter(GameObject*);
	virtual void Update(double dt, GameObject*);
	virtual void Exit(GameObject*);
};

#endif