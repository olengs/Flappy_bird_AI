#include "SceneQueen.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneQueen::SceneQueen()
{
}

SceneQueen::~SceneQueen()
{
}

void SceneQueen::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	m_noGrid = 8;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_queen.resize(m_noGrid, -1);
	m_numSolutions = 0;
	DFS(0);
	std::cout << "Num solutions: " << m_numSolutions << std::endl;
}

void SceneQueen::Update(double dt)
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

void SceneQueen::Render()
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

	//Print queens
	int size = m_queen.size();
	for (int i=0; i<size; ++i)
	{
		int cellX = m_queen[i] % m_noGrid;
		int cellY = m_queen[i] / m_noGrid;

		modelStack.PushMatrix();
		modelStack.Translate(cellX * m_gridSize + m_gridOffset, cellY * m_gridSize + m_gridOffset, 0.f);
		modelStack.Scale(m_gridSize, m_gridSize, 1.f);
		RenderMesh(meshList[GEO_QUEEN], false);
		modelStack.PopMatrix();
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

	RenderTextOnScreen(meshList[GEO_TEXT], "8 Queens", Color(0, 1, 0), 3, 50, 0);
}

void SceneQueen::Exit()
{
	SceneBase::Exit();
}

void SceneQueen::PrintSolution()
{
	std::cout << "Solution: ";
	for (int i : m_queen)
		std::cout << i << " ";
	std::cout << std::endl;
}

bool SceneQueen::CheckQueen()
{
	for (int i = 0; i < m_queen.size()-1; ++i)
	{
		for (int j = i+1; j < m_queen.size(); ++j)
		{
			if (m_queen[j] == -1) break;
			int cellX = m_queen[i] % m_noGrid;
			int cellY = m_queen[i] / m_noGrid;
			int cell2X = m_queen[j] % m_noGrid;
			int cell2Y = m_queen[j] / m_noGrid;
			int dx = cellX - cell2X;
			int dy = cellY - cell2Y;

			if (dx == 0 || dy == 0 || abs(dx) == abs(dy))
				return false;
		}
	}
	return true;
}

void SceneQueen::DFS(int row)
{
	std::vector<int>::iterator it = std::find(m_queen.begin(), m_queen.end(), -1);
	int index = it - m_queen.begin();
	
	for (int i = 0; i < m_noGrid; ++i)
	{
		m_queen[index] = row * m_noGrid + i;
		Application::GetInstance().Iterate();
		Sleep(10);
		if (CheckQueen())
		{
			if (row != m_noGrid - 1) //last row
				DFS(row + 1);
			else
			{
				m_numSolutions++;
				PrintSolution();
			}
		}
		m_queen[index] = -1;
	}
}
