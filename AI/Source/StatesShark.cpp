#include "StatesShark.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

static SceneData& sceneData = *SceneData::GetInstance();

static void RunToTarget(GameObject* subject, GameObject* target, float dt);
static void RunFromTarget(GameObject* subject, GameObject* target, float dt);

StateCrazy::StateCrazy(const std::string& stateID)
	: State(stateID)
{
}

StateCrazy::~StateCrazy()
{
}

void StateCrazy::Enter(GameObject* go)
{
	go->moveSpeed = 20.f;
}

void StateCrazy::Update(double dt, GameObject* go)
{
	if (go->nearest == nullptr || !go->nearest->active || go->nearest->energy < 0)
		PostOffice::GetInstance()->Send("Scene", new MessageWRU(go, MessageWRU::HIGHEST_ENERGYFISH, 0));

	if (go->nearest != nullptr && go->nearest->active)
		RunToTarget(go, go->nearest, dt);

	if (sceneData.GetFishCount() < 12)
		go->SetNextState("shark_naughty");
}

void StateCrazy::Exit(GameObject* go)
{
}

StateNaughty::StateNaughty(const std::string& stateID)
	: State(stateID)
{
}

StateNaughty::~StateNaughty()
{
}

void StateNaughty::Enter(GameObject* go)
{
	go->moveSpeed = 10.f;
}

void StateNaughty::Update(double dt, GameObject* go)
{
	if (go->nearest == nullptr || !go->nearest->active || go->nearest->energy < 0)
		PostOffice::GetInstance()->Send("Scene", new MessageWRU(go, MessageWRU::NEAREST_FULLFISH, 0));

	if (go->nearest != nullptr && go->nearest->active)
		RunToTarget(go, go->nearest, dt);

	if (sceneData.GetFishCount() >= 12)
	{
		go->SetNextState("shark_crazy");
	}
	else if (sceneData.GetFishCount() < 4)
	{
		go->SetNextState("shark_happy");
	}
}

void StateNaughty::Exit(GameObject* go)
{
}

StateHappy::StateHappy(const std::string& stateID)
	: State(stateID)
{
}

StateHappy::~StateHappy()
{
}

void StateHappy::Enter(GameObject* go)
{
	go->moveSpeed = 5.f;
	int range[] = {10, 15};
	PostOffice::GetInstance()->Send("Scene", new MessageSpawn(go, GameObject::GO_FISH, 1, range));
}

void StateHappy::Update(double dt, GameObject* go)
{
	if (sceneData.GetFishCount() >= 4)
	{
		go->SetNextState("shark_naughty");
	}
}

void StateHappy::Exit(GameObject* go)
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
