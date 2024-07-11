#ifndef SCENE_MAZE_H
#define SCENE_MAZE_H

#include "GameObject.h"
#include <vector>
#include <queue>
#include "SceneBase.h"
#include "Maze.h"

class SceneMaze : public SceneBase
{
public:
	SceneMaze();
	~SceneMaze();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	GameObject* FetchGO();

protected:
	void DFS(MazePt curr);
	bool BFS(MazePt start, MazePt end);
	Maze m_maze;
	MazePt m_start, m_end;
	std::vector<Maze::TILE_CONTENT> m_myGrid;
	std::vector<bool> m_visited;
	std::queue<MazePt> m_queue;
	std::vector<MazePt> m_previous;
	std::vector<MazePt> m_shortestPath;
	unsigned m_mazeKey;

	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
};

#endif