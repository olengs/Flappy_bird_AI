#include "StatesFishFood.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

StateEvolve::StateEvolve(const std::string& stateID)
	: State(stateID)
{
}

StateEvolve::~StateEvolve()
{
}

void StateEvolve::Enter(GameObject* go)
{
	PostOffice::GetInstance()->Send("Scene", new MessageEvolve(go));
}

void StateEvolve::Update(double dt, GameObject* go)
{
}

void StateEvolve::Exit(GameObject* go)
{
}

StateGrow::StateGrow(const std::string& stateID)
	: State(stateID)
{
}

StateGrow::~StateGrow()
{
}

void StateGrow::Enter(GameObject* go)
{
	go->countDown = 0;
}

void StateGrow::Update(double dt, GameObject* go)
{
	go->countDown += dt;
	if (go->countDown >= 15.f)
	{
		go->SetNextState("food_evolve");
	}
}

void StateGrow::Exit(GameObject* go)
{
}
