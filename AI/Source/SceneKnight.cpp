#include "SceneKnight.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneKnight::SceneKnight()
{
}

SceneKnight::~SceneKnight()
{
}

void SceneKnight::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 6;
	m_gridSize = m_worldHeight / 8;// m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_grid.resize(m_noGrid * m_noGrid, -1);
	m_numTours = m_move = m_call = m_closedTours = 0;
	DFS(0);
	std::cout << "Num tours: " << m_numTours << std::endl;
	std::cout << "Closed tours: " << m_closedTours << std::endl;
	std::cout << "Calls (DFS): " << m_call << std::endl;
}

void SceneKnight::PrintTour()
{
	std::cout << "Tour: ";
	for (int i : m_grid)
	{
		std::cout << i << " ";
	}
	std::cout << std::endl;
}

void SceneKnight::DFS(int index)
{
	m_call++;
	//std::cout << m_call << " ";
	if (m_grid[index] != -1)
		return;
	m_move++;
	m_grid[index] = m_move;
	Application::GetInstance().Iterate();

	if (m_move == m_noGrid * m_noGrid)
	{
		m_numTours++;
		PrintTour();

		//int cellX = index % m_noGrid;
		//int cellY = index / m_noGrid;
		//int startX = (m_grid[0] - 1) % m_noGrid;
		//int startY = (m_grid[0] - 1) / m_noGrid;
		//
		//int dx = cellX - startX;
		//int dy = cellY - startY;
		//if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2) ||
		//	(dx == -1 && dy == 2) || (dx == -2 && dy == 1) ||
		//	(dx == -2 && dy == -1) || (dx == -1 && dy == -2) ||
		//	(dx == 1 && dy == -2) || (dx == 2 && dy == -1))
		//	m_closedTours++;
	}
	else
	{
		int cellX = index % m_noGrid;
		int cellY = index / m_noGrid;

		int newX = cellX + 2;
		int newY = cellY + 1;
		if (newX < m_noGrid && newY < m_noGrid)
			DFS(newY*m_noGrid + newX);

		newX = cellX + 1;
		newY = cellY + 2;
		if (newX < m_noGrid && newY < m_noGrid)
			DFS(newY * m_noGrid + newX);

		newX = cellX - 1;
		newY = cellY + 2;
		if (newX >= 0 && newY < m_noGrid)
			DFS(newY * m_noGrid + newX);

		newX = cellX - 2;
		newY = cellY + 1;
		if (newX >= 0 && newY < m_noGrid)
			DFS(newY * m_noGrid + newX);

		newX = cellX - 2;
		newY = cellY - 1;
		if (newX >= 0 && newY >= 0)
			DFS(newY * m_noGrid + newX);

		newX = cellX - 1;
		newY = cellY - 2;
		if (newX >= 0 && newY >= 0)
			DFS(newY * m_noGrid + newX);

		newX = cellX + 1;
		newY = cellY - 2;
		if (newX < m_noGrid && newY >= 0)
			DFS(newY * m_noGrid + newX);

		newX = cellX + 2;
		newY = cellY - 1;
		if (newX < m_noGrid && newY >= 0)
			DFS(newY * m_noGrid + newX);
	}

	m_grid[index] = -1;
	m_move--;
}

void SceneKnight::Update(double dt)
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

void SceneKnight::Render()
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
	RenderMesh(meshList[GEO_CHESSBOARD], false);
	modelStack.PopMatrix();

	//Print knights
	//Print queens
	for (int i=0; i<m_noGrid* m_noGrid; ++i)
	{
		if (m_grid[i] != -1)
		{
			int cellX = i % m_noGrid;
			int cellY = i / m_noGrid;

			modelStack.PushMatrix();
			modelStack.Translate(cellX * m_gridSize + m_gridOffset, cellY * m_gridSize + m_gridOffset, 0.f);
			modelStack.Scale(m_gridSize, m_gridSize, 1);
			RenderMesh(meshList[GEO_KNIGHT], false);
			modelStack.PopMatrix();
		}
	}

	//On screen text
	std::ostringstream ss;
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	//ss << "Something" << something;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	RenderTextOnScreen(meshList[GEO_TEXT], "Knight's Tour", Color(0, 1, 0), 3, 50, 0);
}

void SceneKnight::Exit()
{
	SceneBase::Exit();
}
