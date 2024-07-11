#ifndef SCENE_KNIGHT_H
#define SCENE_KNIGHT_H

#include <vector>
#include "SceneBase.h"

class SceneKnight : public SceneBase
{
public:
	SceneKnight();
	~SceneKnight();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void PrintTour();
	void DFS(int index);

protected:
	std::vector<int> m_grid;
	int m_numTours;
	int m_closedTours;
	int m_move;
	int m_call;

	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;
};

#endif