#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Vector3.h"
#include "StateMachine.h"
#include "ObjectBase.h"
#include "Maze.h"
#include "NNode.h"

struct GameObject : public ObjectBase
{
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CROSS,
		GO_FISH,
		GO_FISHFOOD,
		GO_SHARK,
		GO_NPC,
		GO_BIRD,
		GO_PIPE,
		GO_TOTAL, //must be last
	};

	float energy;
	float moveSpeed;
	float countDown;

	GAMEOBJECT_TYPE type;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	bool active;
	float mass;
	Vector3 target;
	int id;
	int steps;
	GameObject* nearest;

	StateMachine* sm;
	State* m_currState;
	State* m_nextState;

	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();

	bool Handle(Message* message);
	void SetNextState(const std::string state);
	std::string GetCurrentState();

	std::vector<Maze::TILE_CONTENT> grid;
	std::vector<bool> visited;
	std::vector<MazePt> stack;
	std::vector<MazePt> path;
	MazePt curr;

	int score;
	bool alive;
	std::vector<NNode> hiddenNode;
	std::vector<NNode> hiddenNode2;
	NNode outputNode;
};

#endif