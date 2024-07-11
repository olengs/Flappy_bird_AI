#ifndef SCENE_MOVEMENT_H
#define SCENE_MOVEMENT_H

#include <vector>
#include <queue>
#include "GameObject.h"
#include "SceneBase.h"
#include "ObjectBase.h"

class SceneMovement : public SceneBase, public ObjectBase
{
public:
	SceneMovement();
	~SceneMovement();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	GameObject* FindClosestGO(GameObject* go, GameObject::GAMEOBJECT_TYPE type);

	void RenderLine(const Vector3& start, const Vector3& end, bool isRed);
	void RenderGO(GameObject *go);
	bool Handle(Message* message);
	void ProcessMessages();

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
protected:

	const float m_foodSpeed = 2.f;

	std::queue<Message*> m_messageQueue;
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;

	//typedef std::map<GameObject::GAMEOBJECT_TYPE, StateMachine*> StateMachineMap;
	using StateMachineMap = std::map<GameObject::GAMEOBJECT_TYPE, StateMachine*>;
	StateMachineMap m_stateMachines;

	void InitSM();
	
	//GameObject *m_ghost;
};

#endif