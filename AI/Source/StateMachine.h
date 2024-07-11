#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <string>
#include <map>
#include "State.h"

class GameObject;
class StateMachine
{
	std::map<std::string, State*> m_stateMap;
public:
	StateMachine();
	~StateMachine();
	void AddState(State *newState);
	void SetNextState(const std::string &nextStateID, GameObject* go);
	const std::string& GetCurrentState(GameObject* go);
	void Update(double dt, GameObject* go);
};

#endif