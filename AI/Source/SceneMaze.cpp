#include "SceneMaze.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneMaze::SceneMaze()
{
}

SceneMaze::~SceneMaze()
{
}

void SceneMaze::Init()
{
	SceneBase::Init();
	bLightEnabled = true;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();
	
	m_noGrid = 12;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_start.Set(0, 0);
	m_mazeKey = 0;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, 0.3f);
	m_myGrid.resize(m_noGrid * m_noGrid, Maze::TILE_FOG);
	m_visited.resize(m_noGrid * m_noGrid, false);
	m_previous.resize(m_noGrid * m_noGrid);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
	DFS(m_start);
}

GameObject* SceneMaze::FetchGO()
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_CROSS);
		m_goList.push_back(go);
	}
	return FetchGO();
}

void SceneMaze::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	static bool restartPressed = false;
	if (Application::IsKeyPressed('R') && !restartPressed)
	{
		restartPressed = true;

		//Exercise: Implement Reset button
		m_start.Set(0, 0);
		m_mazeKey = 0;
		m_maze = Maze();
		m_maze.Generate(m_mazeKey, m_noGrid, m_start, 0.3f);
		std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
		std::fill(m_visited.begin(), m_visited.end(), false);
		m_previous.resize(m_noGrid * m_noGrid);
		m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
		m_queue = {};
		m_shortestPath.clear();
		DFS(m_start);
	}
	else if (!Application::IsKeyPressed('R'))
	{
		restartPressed = false;
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		
		//Exercise: turn mouse click into end point and run BFS
		int cellX = posX / m_gridSize;
		int cellY = posY / m_gridSize;
		m_end.Set(cellX, cellY);
		BFS(m_start, m_end);

		std::cout << "Shortest Path: ";
		for (const auto& pt : m_shortestPath)
		{
			std::cout << "(" << pt.x << ", " << pt.y << ") ";
		}
		std::cout << std::endl;
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}
}


void SceneMaze::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_NONE:
		break;
	}
}

void SceneMaze::Render()
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

	//modelStack.PushMatrix();
	//modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	//modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	//RenderMesh(meshList[GEO_WHITEQUAD], false);
	//modelStack.PopMatrix();

	//Render tiles
	size_t size = m_myGrid.size();
	for (size_t i=0; i<size; ++i)
	{
		modelStack.PushMatrix();
		modelStack.Translate(i%m_noGrid * m_gridSize + m_gridOffset, i/m_noGrid * m_gridSize + m_gridOffset, 0.f);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);

		switch (m_myGrid[i])
		{
		case Maze::TILE_CONTENT::TILE_FOG:
			meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f);
			break;
		case Maze::TILE_CONTENT::TILE_WALL:
			meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.5f, 0.5f, 0.5f);
			break;
		default:
			meshList[GEO_WHITEQUAD]->material.kAmbient.Set(1, 1, 1);
			break;
		}

		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();
	}
	//Render curr point
	modelStack.PushMatrix();
	modelStack.Translate(m_maze.GetCurr().x * m_gridSize + m_gridOffset, m_maze.GetCurr().y * m_gridSize + m_gridOffset, 0.4f);
	modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
	RenderMesh(meshList[GEO_FISH_FULL], false);
	modelStack.PopMatrix();

	for (int i=0; i<m_visited.size(); ++i)
	{
		int cellX = i % m_noGrid;
		int cellY = i / m_noGrid;
	
		if (!m_visited[i]) continue;
		modelStack.PushMatrix();
		modelStack.Translate(cellX * m_gridSize + m_gridOffset, cellY * m_gridSize + m_gridOffset, 0.25f);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
		meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.25f, 0.25f, 1.0f);
		RenderMesh(meshList[GEO_WHITEQUAD], true);
		modelStack.PopMatrix();
	}

	//Render shortest path
	for (MazePt& pt : m_shortestPath)
	{
		modelStack.PushMatrix();
		modelStack.Translate(pt.x * m_gridSize + m_gridOffset, pt.y * m_gridSize + m_gridOffset, 0.3f);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
		RenderMesh(meshList[GEO_CROSS], false);
		modelStack.PopMatrix();
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
	ss << "Num Move:" << m_maze.GetNumMove();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Maze " << 0;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);
}

void SceneMaze::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}

void SceneMaze::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1 && !m_visited[(curr.y + 1) * m_noGrid + curr.x])
	{
		MazePt next(curr.x, curr.y + 1);
		if (m_maze.Move(Maze::DIR_UP))
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
			DFS(next);
			m_maze.Move(Maze::DIR_DOWN);
		}
		else
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
		}
	}
	//DOWN
	if (curr.y > 0 && !m_visited[(curr.y - 1) * m_noGrid + curr.x])
	{
		MazePt next(curr.x, curr.y - 1);
		if (m_maze.Move(Maze::DIR_DOWN))
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
			DFS(next);
			m_maze.Move(Maze::DIR_UP);
		}
		else
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
		}
	}
	//LEFT
	if (curr.x > 0 && !m_visited[curr.y * m_noGrid + curr.x - 1])
	{
		MazePt next(curr.x - 1, curr.y);
		if (m_maze.Move(Maze::DIR_LEFT))
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
			DFS(next);
			m_maze.Move(Maze::DIR_RIGHT);
		}
		else
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1 && !m_visited[curr.y * m_noGrid + curr.x + 1])
	{
		MazePt next(curr.x + 1, curr.y);
		if (m_maze.Move(Maze::DIR_RIGHT))
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
			DFS(next);
			m_maze.Move(Maze::DIR_LEFT);
		}
		else
		{
			m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
		}
	}
}

bool SceneMaze::BFS(MazePt start, MazePt end)
{
	m_maze.SetNumMove(0);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_queue = {};
	m_shortestPath.clear();
	
	m_visited[start.y * m_noGrid + start.x] = true;
	m_queue.push(start);
	while (!m_queue.empty())
	{
		MazePt pt = m_queue.front();
		m_queue.pop();
		m_maze.SetCurr(pt);

		//is goal?
		if (pt.x == m_end.x && pt.y == m_end.y)
		{
			//construct path
			while (pt.x != m_start.x || pt.y != m_start.y)
			{
				m_shortestPath.insert(m_shortestPath.begin(), pt);
				pt = m_previous[pt.y * m_noGrid + pt.x];
			}
			m_shortestPath.insert(m_shortestPath.begin(), pt);
			return true;
		}

		//check neighbours
		//UP
		if (pt.y < m_noGrid - 1)
		{
			MazePt nxt(pt.x, pt.y + 1);
			if (!m_visited[nxt.y * m_noGrid + nxt.x] && m_myGrid[nxt.y * m_noGrid + nxt.x] == Maze::TILE_EMPTY)
			{
				m_previous[nxt.y * m_noGrid + nxt.x] = m_maze.GetCurr();
				m_visited[nxt.y * m_noGrid + nxt.x] = true;
				m_queue.push(nxt);
			}
		}
		//DOWN
		if (pt.y > 0)
		{
			MazePt nxt(pt.x, pt.y - 1);
			if (!m_visited[nxt.y * m_noGrid + nxt.x] && m_myGrid[nxt.y * m_noGrid + nxt.x] == Maze::TILE_EMPTY)
			{
				m_previous[nxt.y * m_noGrid + nxt.x] = m_maze.GetCurr();
				m_visited[nxt.y * m_noGrid + nxt.x] = true;
				m_queue.push(nxt);
			}
		}
		//LEFT
		if (pt.x > 0)
		{
			MazePt nxt(pt.x - 1, pt.y);
			if (!m_visited[nxt.y * m_noGrid + nxt.x] && m_myGrid[nxt.y * m_noGrid + nxt.x] == Maze::TILE_EMPTY)
			{
				m_previous[nxt.y * m_noGrid + nxt.x] = m_maze.GetCurr();
				m_visited[nxt.y * m_noGrid + nxt.x] = true;
				m_queue.push(nxt);
			}
		}
		//RIGHT
		if (pt.x < m_noGrid - 1)
		{
			MazePt nxt(pt.x + 1, pt.y);
			if (!m_visited[nxt.y * m_noGrid + nxt.x] && m_myGrid[nxt.y * m_noGrid + nxt.x] == Maze::TILE_EMPTY)
			{
				m_previous[nxt.y * m_noGrid + nxt.x] = m_maze.GetCurr();
				m_visited[nxt.y * m_noGrid + nxt.x] = true;
				m_queue.push(nxt);
			}
		}
	}

	return false;
}
