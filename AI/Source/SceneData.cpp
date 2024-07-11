#include "SceneData.h"

SceneData::SceneData()
	: m_objectCount(0), m_noGrid(0), m_gridSize(0), m_gridOffset(0), m_fishCount(0)
{
}

SceneData::~SceneData()
{
}

int SceneData::GetFishCount() const
{
	return m_fishCount;
}

int SceneData::GetObjectCount() const
{
	return m_objectCount;
}

int SceneData::GetNoGrid() const
{
	return m_noGrid;
}

float SceneData::GetGridSize() const
{
	return m_gridSize;
}

float SceneData::GetGridOffset() const
{
	return m_gridOffset;
}

void SceneData::SetFishCount(int val)
{
	m_fishCount = val;
}

void SceneData::SetObjectCount(int val)
{
	m_objectCount = val;
}

void SceneData::SetNoGrid(int val)
{
	m_noGrid = val;
}

void SceneData::SetGridSize(float val)
{
	m_gridSize = val;
}

void SceneData::SetGridOffset(float val)
{
	m_gridOffset = val;
}
