#include <sstream>
#include "GL\glew.h"
#include "SceneMovement.h"
#include "Application.h"
#include "StatesFish.h"
#include "StatesShark.h"
#include "StatesFishFood.h"
#include "SceneData.h"
#include "PostOffice.h"
#include "ConcreteMessages.h"

//alias
static SceneData& sceneData = *SceneData::GetInstance();

SceneMovement::SceneMovement()
{
}

SceneMovement::~SceneMovement()
{
}

void SceneMovement::Init()
{
	SceneBase::Init();
	PostOffice::GetInstance()->Register("Scene", this);

	InitSM();
	
	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	Math::InitRNG();

	sceneData.SetObjectCount(0);
	sceneData.SetNoGrid(20);
	sceneData.SetGridSize(m_worldHeight / sceneData.GetNoGrid());
	sceneData.SetGridOffset(sceneData.GetGridSize() / 2);

	//Make sharkkkkkk
	GameObject* shark = FetchGO(GameObject::GO_SHARK);
	shark->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
	shark->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(),
				   sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(), 0);
	shark->target = shark->pos;
	shark->SetNextState("shark_happy");
}

void SceneMovement::InitSM()
{
	m_stateMachines.insert(std::make_pair<GameObject::GAMEOBJECT_TYPE, StateMachine*>(GameObject::GO_FISH, new StateMachine()));
	m_stateMachines.insert(std::make_pair<GameObject::GAMEOBJECT_TYPE, StateMachine*>(GameObject::GO_FISHFOOD, new StateMachine()));
	m_stateMachines.insert(std::make_pair<GameObject::GAMEOBJECT_TYPE, StateMachine*>(GameObject::GO_SHARK, new StateMachine()));

	m_stateMachines[GameObject::GO_FISH]->AddState(new StateDead("fish_dead")); //DEAD
	m_stateMachines[GameObject::GO_FISH]->AddState(new StateTooFull("fish_tooFull")); //TOOFULL
	m_stateMachines[GameObject::GO_FISH]->AddState(new StateFull("fish_full")); //FULL
	m_stateMachines[GameObject::GO_FISH]->AddState(new StateHungry("fish_hungry")); //HUNGRY

	m_stateMachines[GameObject::GO_SHARK]->AddState(new StateNaughty("shark_naughty"));
	m_stateMachines[GameObject::GO_SHARK]->AddState(new StateHappy("shark_happy"));
	m_stateMachines[GameObject::GO_SHARK]->AddState(new StateCrazy("shark_crazy"));

	m_stateMachines[GameObject::GO_FISHFOOD]->AddState(new StateGrow("food_grow"));
	m_stateMachines[GameObject::GO_FISHFOOD]->AddState(new StateEvolve("food_evolve"));
}

GameObject* SceneMovement::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active && type == go->type)
		{
			go->active = true;
			sceneData.SetObjectCount(sceneData.GetObjectCount() + 1);
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject* go = new GameObject(type);
		m_goList.push_back(go);
		go->id = m_goList.size();
		go->sm = m_stateMachines[type];
	}
	return FetchGO(type);
}

GameObject* SceneMovement::FindClosestGO(GameObject* go, GameObject::GAMEOBJECT_TYPE type)
{
	GameObject* closestGO = nullptr;
	float closestDist = FLT_MAX;
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go1 = (GameObject*)*it;
		if (go1->active)
		{
			if (go1->type == type && (go1->pos - go->pos).LengthSquared() < closestDist)
			{
				closestGO = go1;
				closestDist = (go1->pos - go->pos).LengthSquared();
			}
		}
	}
	return closestGO;
}

void SceneMovement::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	
	if(Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}

	//Input Section
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
		GameObject* go = FetchGO(GameObject::GO_FISH);
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(),
					sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(), 0);
		go->target = go->pos;
		go->steps = 0;
		go->energy = 8.0f;
		go->SetNextState("fish_full");
		go->countDown = 3.f;
		//go->moveSpeed = m_fishSpeed;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	static bool bJState = false;
	if (!bJState && Application::IsKeyPressed('J'))
	{
		bJState = true;

		GameObject* go = FetchGO(GameObject::GO_FISHFOOD);
		go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
		go->pos.Set(sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(), 
					sceneData.GetGridOffset() + Math::RandIntMinMax(0, sceneData.GetNoGrid() - 1) * sceneData.GetGridSize(), 0);
		go->moveSpeed = m_foodSpeed;
		go->SetNextState("food_grow");
		go->target = go->pos;
	}
	else if (bJState && !Application::IsKeyPressed('J'))
	{
		bJState = false;
	}

	//FSM
	for (GameObject* go : m_goList)
	{
		if (go->active && go->sm != nullptr)
		{
			go->sm->Update(dt * m_speed, go);
		}
	}

	//counting
	int objCount = 0;
	int fishCount = 0;
	for (GameObject* go : m_goList)
	{
		objCount += go->Handle(new MessageCheckActive()) ? 1 : 0;
		fishCount += go->Handle(new MessageCheckFish()) ? 1 : 0;
	}
	SceneData::GetInstance()->SetObjectCount(objCount);
	SceneData::GetInstance()->SetFishCount(fishCount);

	//Movement Section
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			Vector3 dir = go->target - go->pos;
			if (dir.Length() < go->moveSpeed * dt * m_speed)
			{
				//GO->pos reach target
				go->pos = go->target;
				int random = Math::RandIntMinMax(0, 3);

				// set next target pos
				switch (random)
				{
				case 0:
					go->target.x += sceneData.GetGridSize();
					break;
				case 1:
					go->target.x -= sceneData.GetGridSize();
					break;
				case 2:
					go->target.y += sceneData.GetGridSize();
					break;
				case 3:
					go->target.y -= sceneData.GetGridSize();
					break;
				}

				//Clamp target to boundary
				go->target.x = Math::Clamp(go->target.x, sceneData.GetGridOffset(), sceneData.GetGridOffset() + (sceneData.GetNoGrid() - 1) * sceneData.GetGridSize());
				go->target.y = Math::Clamp(go->target.y, sceneData.GetGridOffset(), sceneData.GetGridOffset() + (sceneData.GetNoGrid() - 1) * sceneData.GetGridSize());
			}
			else
			{
				try
				{
					dir.Normalize();
				}
				catch (DivideByZero& e)
				{
					dir.SetZero();
				}
				go->pos += dir * go->moveSpeed * static_cast<float>(dt) * m_speed;
			}
		}
	}

	//Collision - we check for overlapping after all entities' positions are updated
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active) continue;
		for (std::vector<GameObject*>::iterator it2 = it+1; it2 != m_goList.end(); ++it2) 
		{
			GameObject* go2 = (GameObject*)*it2;
			if ((go->pos - go2->pos).Length() < ((go->scale.x + go2->scale.x) / 2.f) && go2->active)
			{
				if (go->type == GameObject::GO_FISH) 
				{
					if (go2->type == GameObject::GO_FISHFOOD)
					{
						go->energy += 2;
						go2->active = false;
					}
					else if (go2->type == GameObject::GO_SHARK)
					{
						go->energy = -1;
					}
				}
				else if (go2->type == GameObject::GO_FISH)
				{
					if (go->type == GameObject::GO_FISHFOOD) 
					{
						go2->energy += 2;
						go->active = false;
						break;
					}
					else if (go->type == GameObject::GO_SHARK)
					{
						go2->energy = -1;
					}
				}
			}
		}
	}

	ProcessMessages();
}

void SceneMovement::ProcessMessages()
{
	while (!m_messageQueue.empty())
	{
		Message* message = m_messageQueue.front();
		m_messageQueue.pop();
		MessageWRU* messageWRU = dynamic_cast<MessageWRU*>(message);
		if (messageWRU != nullptr)
		{
			if (messageWRU->type == MessageWRU::NEAREST_FISHFOOD)
			{
				GameObject* obj = FindClosestGO(messageWRU->go, GameObject::GAMEOBJECT_TYPE::GO_FISHFOOD);
				if (obj != nullptr && (obj->pos - messageWRU->go->pos).LengthSquared() <= messageWRU->threshold * messageWRU->threshold)
				{
					messageWRU->go->nearest = obj;
					obj->Handle(new MessageStop());
				}
			}
			else if (messageWRU->type == MessageWRU::NEAREST_SHARK)
			{
				GameObject* obj = FindClosestGO(messageWRU->go, GameObject::GAMEOBJECT_TYPE::GO_SHARK);
				if (obj != nullptr && (obj->pos - messageWRU->go->pos).LengthSquared() <= messageWRU->threshold * messageWRU->threshold)
					messageWRU->go->nearest = obj;
			}
			else if (messageWRU->type == MessageWRU::HIGHEST_ENERGYFISH)
			{
				float highestEnergy = FLT_MIN;
				GameObject* highest = nullptr;
				for (GameObject* go : m_goList)
				{
					if (go->type != GameObject::GO_FISH) continue;
					if (go->energy > highestEnergy)
					{
						highestEnergy = go->energy;
						highest = go;
					}
				}

				messageWRU->go->nearest = highest;
			}
			else if (messageWRU->type == MessageWRU::NEAREST_FULLFISH)
			{
				GameObject* nearest = nullptr;
				float smallestDist = FLT_MAX;
				for (GameObject* go : m_goList)
				{
					if (go->type != GameObject::GO_FISH) continue;

					const std::string& state = go->GetCurrentState();
					float dist = (messageWRU->go->pos - go->pos).LengthSquared();
					if ((state == "fish_toofull" || state == "fish_full") && dist < smallestDist)
					{
						smallestDist = dist;
						nearest = go;
					}
				}

				messageWRU->go->nearest = nearest;
			}
		}

		MessageSpawn* messageSpawn = dynamic_cast<MessageSpawn*>(message);
		if (messageSpawn != nullptr)
		{
			for (int i = 0; i < messageSpawn->count; ++i)
			{
				float sign = Math::RandFloatMinMax(0.f, 1.f) < 0.5 ? -1 : 1;
				GameObject* go = FetchGO((GameObject::GAMEOBJECT_TYPE)messageSpawn->type);
				int tileX = (int)messageSpawn->go->pos.x / sceneData.GetGridSize() + sign * Math::RandIntMinMax(messageSpawn->distRange[0], messageSpawn->distRange[1]);
				int tileY = (int)messageSpawn->go->pos.y / sceneData.GetGridSize() + sign * Math::RandIntMinMax(messageSpawn->distRange[0], messageSpawn->distRange[1]);
				tileX = Math::Clamp(tileX, 0, sceneData.GetNoGrid() - 1);
				tileY = Math::Clamp(tileY, 0, sceneData.GetNoGrid() - 1);

				go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
				go->pos.Set(sceneData.GetGridOffset() + tileX * sceneData.GetGridSize(),
					sceneData.GetGridOffset() + tileY * sceneData.GetGridSize(), 0);
				go->target = go->pos;
				go->moveSpeed = m_foodSpeed;

				if (go->type == GameObject::GO_FISH)
				{
					go->energy = 8.0f;
					go->SetNextState("fish_full");
					go->countDown = 3.f;
				}
			}
		}

		MessageEvolve* messageEvolve = dynamic_cast<MessageEvolve*>(message);
		if (messageEvolve != nullptr)
		{
			messageEvolve->go->active = false;
			GameObject* go = FetchGO(GameObject::GO_FISH);
			go->scale.Set(sceneData.GetGridSize(), sceneData.GetGridSize(), sceneData.GetGridSize());
			go->pos = messageEvolve->go->pos;
			go->target = messageEvolve->go->target;
			go->energy = 8.0f;
			go->SetNextState("fish_full");
			go->countDown = 3.f;

			go->SetNextState("food_grow");
		}

		delete message;
	}
}

bool SceneMovement::Handle(Message* message)
{
	m_messageQueue.push(message);
	return true;
}

void SceneMovement::RenderLine(const Vector3& start, const Vector3& end, bool isRed)
{
	Vector3 disp = end - start;
	modelStack.PushMatrix();
	modelStack.Translate(start.x, start.y, 1);
	modelStack.Rotate(Math::RadianToDegree(atan2(disp.y, disp.x)), 0, 0, 1);
	modelStack.Scale(disp.Length(), 1, 1);

	RenderMesh(meshList[isRed ? GEO_LINE_RED : GEO_LINE_GREEN], false);

	modelStack.PopMatrix();
}

void SceneMovement::RenderGO(GameObject *go)
{
	std::stringstream ss;
	switch(go->type)
	{
	case GameObject::GO_FISH:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + (float)go->id * 0.01f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->GetCurrentState() == "fish_tooFull") RenderMesh(meshList[GEO_FISH_TOOFULL], false);
		else if (go->GetCurrentState() == "fish_full") RenderMesh(meshList[GEO_FISH_FULL], false);
		else if (go->GetCurrentState() == "fish_hungry") RenderMesh(meshList[GEO_FISH_HUNGRY], false);
		else if (go->GetCurrentState() == "fish_dead") RenderMesh(meshList[GEO_FISH_DEAD], false);

		ss.str("");
		ss.precision(3);
		ss << go->energy;
		modelStack.Translate(0, -.5f, 0);
		modelStack.Scale(.5f, .5f, .5f);
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHARK:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + (float)go->id * 0.01f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);

		if (go->GetCurrentState() == "shark_happy") RenderMesh(meshList[GEO_SHARK_HAPPY], false);
		else if (go->GetCurrentState() == "shark_naughty") RenderMesh(meshList[GEO_SHARK], false);
		else if (go->GetCurrentState() == "shark_crazy") RenderMesh(meshList[GEO_SHARK_CRAZY], false);

		modelStack.PopMatrix();
		break;
	case GameObject::GO_FISHFOOD:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + (float)go->id * 0.01f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_FISHFOOD], false);
		modelStack.PopMatrix();
		break;
	}

	if (go->nearest != nullptr && go->nearest->active)
		RenderLine(go->pos, go->nearest->pos, go->nearest->type != GameObject::GO_FISHFOOD);
}

void SceneMovement::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);
	
	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
						camera.position.x, camera.position.y, camera.position.z,
						camera.target.x, camera.target.y, camera.target.z,
						camera.up.x, camera.up.y, camera.up.z
					);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();
	
	RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_BG], false);
	modelStack.PopMatrix();

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}

	//On screen text

	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);
	
	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Count:" << sceneData.GetObjectCount();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Movement", Color(0, 1, 0), 3, 50, 0);
}

void SceneMovement::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}

	for (StateMachineMap::iterator it = m_stateMachines.begin(); it != m_stateMachines.end(); ++it)
	{
		delete (*it).second;
	}
	
	while (!m_messageQueue.empty())
	{
		delete m_messageQueue.front();
		m_messageQueue.pop();
	}
}
