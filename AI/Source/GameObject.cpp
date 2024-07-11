
#include "GameObject.h"
#include "ConcreteMessages.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue) 
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	sm(nullptr),
	nearest(nullptr),
	m_currState(nullptr),
	m_nextState(nullptr),
	score(0),
	alive(false)
{
}

GameObject::~GameObject()
{
	//if (sm != nullptr)
	//{
	//	delete sm;
	//	sm = nullptr;
	//}
}

void GameObject::SetNextState(const std::string state)
{
	sm->SetNextState(state, this);
}

std::string GameObject::GetCurrentState()
{
	if (m_currState == nullptr) return "";
	return m_currState->GetStateID();
}

bool GameObject::Handle(Message* message)
{
	MessageCheckActive* msg = dynamic_cast<MessageCheckActive*>(message);
	if (msg != nullptr)
	{
		delete message;
		return active;
	}

	MessageCheckFish* msg2 = dynamic_cast<MessageCheckFish*>(message);
	if (msg2 != nullptr)
	{
		delete message;
		return active && type == GO_FISH;
	}

	MessageStop* msg3 = dynamic_cast<MessageStop*>(message);
	if (msg3 != nullptr)
	{
		moveSpeed = 0;
		delete message;
		return true;
	}

	delete message;
	return false;
}
