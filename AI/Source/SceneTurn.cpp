#include "SceneTurn.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneTurn::SceneTurn()
{
}

SceneTurn::~SceneTurn()
{
}

void SceneTurn::Init()
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
	m_wallLoad = 0.3f;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	//m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
	//DFS(m_start);

	m_turn = 0;

	m_gScore.resize(m_noGrid * m_noGrid);
}

GameObject* SceneTurn::FetchGO()
{
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (!go->active)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject* go = new GameObject(GameObject::GO_NONE);
		m_goList.push_back(go);
	}
	return FetchGO();
}

void SceneTurn::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if (curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_UP) == true)
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
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_DOWN) == true)
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
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_LEFT) == true)
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
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_RIGHT) == true)
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
}

bool SceneTurn::BFS(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_queue.empty())
		m_queue.pop();
	m_shortestPath.clear();
	m_queue.push(start);
	m_maze.SetNumMove(0);
	while (!m_queue.empty())
	{
		MazePt curr = m_queue.front();
		m_maze.SetCurr(curr);
		m_queue.pop();
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = m_previous[curr.y * m_noGrid + curr.x];
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//RIGHT
		if (curr.x < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
	}
	return false;
}

void SceneTurn::DFSOnce(GameObject* go)
{
	if (go->stack.empty() || go->curr.x!=go->stack.back().x || go->curr.y != go->stack.back().y)
		go->stack.push_back(go->curr);
	go->visited[go->curr.y * m_noGrid + go->curr.x] = true;

	int offsets[][2] = { {0, 1}, {0, -1}, {-1, 0}, {1, 0} };

	for (int i = 0; i < 4; ++i)
	{
		MazePt nxt(go->curr.x + offsets[i][0], go->curr.y + offsets[i][1]);
		if (nxt.x < m_noGrid && nxt.x >= 0 &&
			nxt.y < m_noGrid && nxt.y >= 0 &&
			!go->visited[nxt.y*m_noGrid + nxt.x])
		{
			int index = nxt.y * m_noGrid + nxt.x;
			go->grid[index] = m_maze.See(nxt);
			if (go->grid[index] == Maze::TILE_EMPTY)
			{
				go->curr = nxt;
				return;
			}
		}
	}

	go->stack.pop_back();
	if (!go->stack.empty())
		go->curr = go->stack.back();
}

bool SceneTurn::BFSLimit(GameObject* go, MazePt end, int limit)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_queue = {};
	go->path.clear();

	m_queue.push(go->curr);
	float nearestDistance = FLT_MAX;
	MazePt nearestTile = go->curr;
	int loop = 0;

	//do i need to do this?
	m_visited[go->curr.y * m_noGrid + go->curr.x] = true;

	while (!m_queue.empty() && loop < limit)
	{
		loop++;
		m_maze.SetCurr(m_queue.front());
		m_queue.pop();
		MazePt curr = m_maze.GetCurr();
		float distance = abs(m_end.x - curr.x) + abs(m_end.y - curr.y);
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestTile = curr;
		}
		//construct shortest path
		if (curr.x == m_end.x && curr.y == m_end.y)
		{
			while (curr.x != go->curr.x || curr.y != go->curr.y)
			{
				go->path.insert(go->path.begin(), curr);
				curr = m_previous[curr.y * m_noGrid + curr.x];
			}
			go->path.insert(go->path.begin(), curr);
			return true;
		}

		//UP
		MazePt next;
		next.Set(curr.x, curr.y + 1);
		int nextIndex = next.y * m_noGrid + next.x;
		if (next.y < m_noGrid && !m_visited[nextIndex] && go->grid[nextIndex] == Maze::TILE_EMPTY)
		{
			m_previous[nextIndex] = m_maze.GetCurr();
			m_queue.push(next);
			m_visited[nextIndex] = true;
		}
		//DOWN
		next.Set(curr.x, curr.y - 1);
		nextIndex = next.y * m_noGrid + next.x;
		if (next.y >= 0 && !m_visited[nextIndex] && go->grid[nextIndex] == Maze::TILE_EMPTY)
		{
			m_previous[nextIndex] = m_maze.GetCurr();
			m_queue.push(next);
			m_visited[nextIndex] = true;
		}
		//LEFT
		next.Set(curr.x - 1, curr.y);
		nextIndex = next.y * m_noGrid + next.x;
		if (next.x >= 0 && !m_visited[nextIndex] && go->grid[nextIndex] == Maze::TILE_EMPTY)
		{
			m_previous[nextIndex] = m_maze.GetCurr();
			m_queue.push(next);
			m_visited[nextIndex] = true;
		}
		//RIGHT
		next.Set(curr.x + 1, curr.y);
		nextIndex = next.y * m_noGrid + next.x;
		if (next.x < m_noGrid && !m_visited[nextIndex] && go->grid[nextIndex] == Maze::TILE_EMPTY)
		{
			m_previous[nextIndex] = m_maze.GetCurr();
			m_queue.push(next);
			m_visited[nextIndex] = true;
		}
	}

	//shortest path to nearest instead of targeted end tile
	m_maze.SetCurr(nearestTile);
	while (m_maze.GetCurr().x != go->curr.x || m_maze.GetCurr().y != go->curr.y)
	{
		auto curr = m_maze.GetCurr();
		go->path.insert(go->path.begin(), curr);
		m_maze.SetCurr(m_previous[curr.y * m_noGrid + curr.x]);
	}

	return false;
}

static int HScore(MazePt pt, MazePt end)
{
	return abs(pt.x - end.x) + abs(pt.y - end.y);
}

bool SceneTurn::AStar(GameObject* go, MazePt end)
{
	MyPriorityQueue pQueue;
	std::fill(m_visited.begin(), m_visited.end(), false);
	std::fill(m_gScore.begin(), m_gScore.end(), INT_MAX);
	go->path.clear();

	m_visited[go->curr.y * m_noGrid + go->curr.x] = true;
	m_gScore[go->curr.y * m_noGrid + go->curr.x] = 0;
	pQueue.PushNode(MazeNode {go->curr, HScore(go->curr, end) });

	while (!pQueue.empty())
	{
		MazeNode currNode = pQueue.top(); //get cheapest node
		//has path been found?
		if (currNode.pt.x == end.x && currNode.pt.y == end.y)
		{
			//construct path
			MazePt pt = currNode.pt;
			while (pt.x != go->curr.x || pt.y != go->curr.y)
			{
				go->path.insert(go->path.begin(), pt);
				pt = m_previous[pt.y * m_noGrid + pt.x];
				//std::cout << pt.x << ", " << pt.y << std::endl;
			}
			go->path.insert(go->path.begin(), go->curr);

			return true;
		}
		pQueue.pop();

		MazePt neighbours[] = { MazePt(currNode.pt.x, currNode.pt.y + 1),
								MazePt(currNode.pt.x, currNode.pt.y - 1),
								MazePt(currNode.pt.x - 1, currNode.pt.y),
								MazePt(currNode.pt.x + 1, currNode.pt.y) };
		//neighbours
		int currIndex = currNode.pt.y * m_noGrid + currNode.pt.x;
		for (int i=0; i<4; ++i)
		{
			if (neighbours[i].x < 0 || neighbours[i].x >= m_noGrid || neighbours[i].y < 0 || neighbours[i].y >= m_noGrid)
				continue;
			if (m_maze.See(neighbours[i]) == Maze::TILE_CONTENT::TILE_WALL)
				continue;

			int nIndex = neighbours[i].y * m_noGrid + neighbours[i].x;
			int tempScore = m_gScore[currIndex] + (int)m_maze.See(neighbours[i]); //dist travelled + edge's weight
			if (tempScore < m_gScore[nIndex])
			{
				pQueue.PushNode(MazeNode{ neighbours[i], tempScore + HScore(neighbours[i], end) });
				m_gScore[nIndex] = tempScore;
				m_previous[nIndex] = currNode.pt;
				m_visited[nIndex] = true;
			}
		}
	}

	return false;
}

void SceneTurn::Update(double dt)
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
	if (Application::IsKeyPressed('N'))
	{
	}
	if (Application::IsKeyPressed('M'))
	{
	}
	if (Application::IsKeyPressed(VK_RETURN))
	{
	}
	if (Application::IsKeyPressed('R'))
	{
		//Exercise: Implement Reset button
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
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
		{
			m_end.Set((int)(posX / m_gridSize), (int)(posY / m_gridSize));
			for (GameObject* go : m_goList)
				if (go->active && go->type == GameObject::GO_NPC)
					AStar(go, m_end);
					//BFSLimit(go, m_end, 20);
		}
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

		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
		{
			m_maze.m_grid[int(posY/m_gridSize) * m_noGrid + int(posX/m_gridSize)] = Maze::TILE_CONTENT::TILE_MUD;
		}
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
		GameObject* go = FetchGO();
		go->type = GameObject::GAMEOBJECT_TYPE::GO_NPC;
		go->grid.resize(m_noGrid * m_noGrid);
		go->visited.resize(m_noGrid * m_noGrid);
		std::fill(go->grid.begin(), go->grid.end(), Maze::TILE_FOG);
		std::fill(go->visited.begin(), go->visited.end(), false);
		int index = 0;
		do
		{
			index = Math::RandIntMinMax(0, m_noGrid*m_noGrid - 1);
		} while (m_maze.m_grid[index] != Maze::TILE_CONTENT::TILE_EMPTY);
		go->curr.Set(index % m_noGrid, index / m_noGrid);

		go->stack.push_back(go->curr);
		go->grid[go->curr.y * m_noGrid + go->curr.x] = Maze::TILE_EMPTY;
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	static const float TURN_TIME = 0.5f;
	static float timer = 0.f;
	timer += static_cast<float>(dt) * m_speed;
	if (timer >= TURN_TIME)
	{
		do {
			timer -= TURN_TIME;
		} while (timer >= TURN_TIME);

		m_turn++;
		for (GameObject* go : m_goList)
		{
			if (go->active && go->type == GameObject::GAMEOBJECT_TYPE::GO_NPC)
			{
				if (!go->stack.empty())
					while (!go->stack.empty())
						DFSOnce(go);
				else if (!go->path.empty())
				{
					MazePt node = go->path.front();
					go->path.erase(go->path.begin());
					go->curr = node;
				}
			}
		}
	}
}

void SceneTurn::RenderGO(GameObject* go)
{
	switch (go->type)
	{
	case GameObject::GO_NPC: //Render GO_NPC
		modelStack.PushMatrix();
		modelStack.Translate(go->curr.x * m_gridSize + m_gridOffset, go->curr.y * m_gridSize + m_gridOffset, 0.1f);
		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);

		RenderMesh(meshList[GEO_KNIGHT], false);

		modelStack.PopMatrix();
		break;
	}
}

void SceneTurn::Render()
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
	RenderMesh(meshList[GEO_WHITEQUAD], false);
	modelStack.PopMatrix();

	//Render tiles 
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * col, m_gridOffset + m_gridSize * row, 0);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			switch (m_maze.m_grid[row * m_noGrid + col])
			{
			case Maze::TILE_WALL:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.5f, 0.5f, 0.5f);
				break;
			case Maze::TILE_FOG:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0, 0, 0);
				break;
			case Maze::TILE_EMPTY:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(1, 1, 1);
				break;
			case Maze::TILE_MUD:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.75f, 0.75f, 0.2f);
				break;
			}
			RenderMesh(meshList[GEO_WHITEQUAD], true);
			modelStack.PopMatrix();
		}
	}

	int i = 0;
	for (bool b : m_visited)
	{
		if (b)
		{
			int cellX = i % m_noGrid;
			int cellY = i / m_noGrid;

			if (m_maze.m_grid[cellY * m_noGrid + cellX] == Maze::TILE_CONTENT::TILE_EMPTY)
			{
				modelStack.PushMatrix();
				modelStack.Translate(m_gridOffset + m_gridSize * cellX, m_gridOffset + m_gridSize * cellY, 0.05f);
				modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.25f, 0.25f, 1);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				modelStack.PopMatrix();
			}
		}
		i++;
	}
	
	//Render m_goList
	for (GameObject* go : m_goList)
	{
		if (!go->active) continue;
		RenderGO(go);
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
	ss << "Turn:" << m_turn;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn Maze " << m_mazeKey;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);
}

void SceneTurn::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}
