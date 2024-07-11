#ifndef SCENE_DATA_H
#define SCENE_DATA_H

#include "SingletonTemplate.h"

class SceneData : public Singleton<SceneData>
{
public:
	int GetFishCount() const;
	int GetObjectCount() const;
	int GetNoGrid() const;
	float GetGridSize() const;
	float GetGridOffset() const;

	void SetFishCount(int);
	void SetObjectCount(int);
	void SetNoGrid(int);
	void SetGridSize(float);
	void SetGridOffset(float);

private:
	friend class Singleton<SceneData>;

	int m_fishCount;
	int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	SceneData();
	~SceneData();
	SceneData(const SceneData&) = delete;
};

#endif //SCENE_DATA_H
