#ifndef SCENE_TURN_H
#define SCENE_TURN_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Maze.h"
#include <queue>

class SceneTurn : public SceneBase
{
public:
	SceneTurn();
	~SceneTurn();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject* go);
	GameObject* FetchGO();

	void DFS(MazePt curr);
	bool BFS(MazePt start, MazePt end);
	bool AStar(GameObject* go, MazePt end);

	void DFSOnce(GameObject* go);
	bool BFSLimit(GameObject* go, MazePt end, int limit);

protected:

	std::vector<GameObject*> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	Maze m_maze;
	MazePt m_start;
	MazePt m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector<MazePt> m_previous; //to store previous tile
	std::vector<MazePt> m_shortestPath;  //to store shortest path
	unsigned m_mazeKey;
	float m_wallLoad;

	int m_turn;

	//for a-star
	std::vector<int> m_gScore;
};

class MazeNodeCompare
{
public:
	bool operator()(const MazeNode& lhs, const MazeNode& rhs)
	{
		return lhs.cost > rhs.cost;
	}
};

class MyPriorityQueue : public std::priority_queue<MazeNode, std::vector<MazeNode>, MazeNodeCompare>
{
public:
	void PushNode(const MazeNode& value)
	{
		auto it = std::find(c.begin(), c.end(), value);
		if (it == c.end())
			push(value);
		else
		{
			(*it).cost = value.cost;
			std::make_heap(c.begin(), c.end(), comp);
		}
	}
};

#endif