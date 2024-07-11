#include "StateMachine.h"
#include "GameObject.h"

StateMachine::StateMachine()
{
}

StateMachine::~StateMachine()
{
	for (std::map<std::string, State*>::iterator it = m_stateMap.begin(); it != m_stateMap.end(); ++it)
	{
		delete it->second;
	}
	m_stateMap.clear();
}

void StateMachine::AddState(State *newState)
{
	if (!newState)
		return;
	if (m_stateMap.find(newState->GetStateID()) != m_stateMap.end())
		return;
	m_stateMap.insert(std::pair<std::string, State*>(newState->GetStateID(), newState));
}

void StateMachine::SetNextState(const std::string &nextStateID, GameObject* go)
{
	std::map<std::string, State*>::iterator it = m_stateMap.find(nextStateID);
	if (it != m_stateMap.end())
	{
		go->m_nextState = (State *)it->second;
	}
}

const std::string & StateMachine::GetCurrentState(GameObject* go)
{
	if (go->m_currState)
		return go->m_currState->GetStateID();
	return "<No states>";
}

void StateMachine::Update(double dt, GameObject* go)
{
	if (go->m_nextState != go->m_currState)
	{
		if (go->m_currState != nullptr)
			go->m_currState->Exit(go);
		go->m_currState = go->m_nextState;
		go->m_currState->Enter(go);
	}

	if (go->m_currState != nullptr)
		go->m_currState->Update(dt, go);
}
