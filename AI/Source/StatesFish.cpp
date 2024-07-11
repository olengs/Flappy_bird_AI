#include "StatesFish.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

//alias
static SceneData& sceneData = *SceneData::GetInstance();

static void RunToTarget(GameObject* subject, GameObject* target, float dt);
static void RunFromTarget(GameObject* subject, GameObject* target, float dt);

StateTooFull::StateTooFull(const std::string& stateID)
	: State(stateID)
{
}

StateTooFull::~StateTooFull()
{
}

void StateTooFull::Enter(GameObject* go)
{
	go->moveSpeed = 0.f;
}

void StateTooFull::Update(double dt, GameObject* go)
{
	go->energy -= dt;
	if (go->energy < 10)
		go->SetNextState("fish_full");
}

void StateTooFull::Exit(GameObject* go)
{
}

StateFull::StateFull(const std::string& stateID)
	: State(stateID)
{
}

StateFull::~StateFull()
{
}

void StateFull::Enter(GameObject* go)
{
	go->moveSpeed = 30.f;
	go->countDown = 1;
	go->nearest = nullptr;
}

void StateFull::Update(double dt, GameObject* go)
{
	go->countDown += dt;
	if (go->countDown >= 1)
	{
		go->countDown -= 1;
		PostOffice::GetInstance()->Send("Scene", new MessageWRU(go, MessageWRU::NEAREST_SHARK, sceneData.GetGridSize()*5));
	}

	GameObject* nearest = go->nearest;
	if (nearest != nullptr && nearest->active)
	{
		RunFromTarget(go, nearest, dt);
	}

	go->energy -= dt;
	if (go->energy < 5)
	{
		go->SetNextState("fish_hungry");
	}
	else if (go->energy >= 10)
	{
		go->SetNextState("fish_tooFull");
	}
}

void StateFull::Exit(GameObject* go)
{
}

StateHungry::StateHungry(const std::string& stateID)
	: State(stateID)
{
}

StateHungry::~StateHungry()
{
}

void StateHungry::Enter(GameObject* go)
{
	go->moveSpeed = 15.f;
	go->nearest = nullptr;

	int range[] = {1, 3};
	PostOffice::GetInstance()->Send("Scene", new MessageSpawn(go, GameObject::GO_FISHFOOD, 2, range));
}

void StateHungry::Update(double dt, GameObject* go)
{
	if (go->nearest == nullptr || !go->nearest->active)
		PostOffice::GetInstance()->Send("Scene", new MessageWRU(go, MessageWRU::NEAREST_FISHFOOD, sceneData.GetGridSize() * 10));

	GameObject* nearest = go->nearest;
	if (nearest != nullptr && nearest->active)
		RunToTarget(go, nearest, dt);

	go->energy -= dt;
	if (go->energy <= 0)
	{
		go->SetNextState("fish_dead");
	}
	else if (go->energy >= 5)
	{
		go->SetNextState("fish_full");
	}
}

void StateHungry::Exit(GameObject* go)
{
}

StateDead::StateDead(const std::string& stateID)
	: State(stateID)
{
}

StateDead::~StateDead()
{
}

void StateDead::Enter(GameObject* go)
{
	go->countDown = 3;
	go->moveSpeed = 0.f;
}

void StateDead::Update(double dt, GameObject* go)
{
	go->countDown -= dt;
	if (go->countDown < 0.f)
	{
		go->active = false;
	}
}

void StateDead::Exit(GameObject* go)
{
}

static void RunToTarget(GameObject* subject, GameObject* target, float dt)
{
	Vector3 dir = subject->target - subject->pos;
	if (dir.Length() > subject->moveSpeed * dt) return; //not reach target cell yet
	subject->pos = subject->target;

	if (abs(target->pos.x - subject->pos.x) > abs(target->pos.y - subject->pos.y)) //target closer in x axis
	{
		if (target->pos.x > subject->pos.x)
		{
			subject->target.x += sceneData.GetGridSize();
		}
		else
		{
			subject->target.x -= sceneData.GetGridSize();
		}
	}
	else //target closer in y axis
	{
		if (target->pos.y > subject->pos.y)
		{
			subject->target.y += sceneData.GetGridSize();
		}
		else
		{
			subject->target.y -= sceneData.GetGridSize();
		}
	}
}

static void RunFromTarget(GameObject* subject, GameObject* target, float dt)
{
	Vector3 dir = subject->target - subject->pos;
	if (dir.Length() > subject->moveSpeed * dt) return; //not reach target cell yet
	subject->pos = subject->target;

	if (abs(target->pos.x - subject->pos.x) > abs(target->pos.y - subject->pos.y)) //target closer in x axis
	{
		if (target->pos.x > subject->pos.x)
		{
			subject->target.x -= sceneData.GetGridSize();
		}
		else
		{
			subject->target.x += sceneData.GetGridSize();
		}
	}
	else //target closer in y axis
	{
		if (target->pos.y > subject->pos.y)
		{
			subject->target.y -= sceneData.GetGridSize();
		}
		else
		{
			subject->target.y += sceneData.GetGridSize();
		}
	}

	//Clamp target to boundary
	subject->target.x = Math::Clamp(subject->target.x, sceneData.GetGridOffset(), sceneData.GetGridOffset() + (sceneData.GetNoGrid() - 1) * sceneData.GetGridSize());
	subject->target.y = Math::Clamp(subject->target.y, sceneData.GetGridOffset(), sceneData.GetGridOffset() + (sceneData.GetNoGrid() - 1) * sceneData.GetGridSize());
}