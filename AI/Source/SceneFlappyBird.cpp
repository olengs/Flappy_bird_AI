#include "SceneFlappyBird.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

constexpr int INPUTSIZE = 4;
constexpr int HIDDENSIZE = 4;

static std::vector<GameObject*> birds;

float operator*(const std::vector<float>& lhs, const std::vector<float>& rhs)
{
	float total = 0;
	size_t size = lhs.size();
	for (size_t i=0; i<size; ++i)
	{
		total += lhs[i] * rhs[i];
	}

	return total;
}

bool AABBvsCIRCLE(AABB aabb, Vector3 pos, float radius)
{
	float rSQ = radius * radius;

	float DeltaX = pos.x - Math::Max(aabb.position.x - aabb.width / 2, Math::Min(pos.x, aabb.position.x + aabb.width / 2));
	float DeltaY = pos.y - Math::Max(aabb.position.y - aabb.height / 2, Math::Min(pos.y, aabb.position.y + aabb.height / 2));
	float result = (DeltaX * DeltaX + DeltaY * DeltaY);
	return result < rSQ;
}

SceneFlappyBird::SceneFlappyBird()
{
}

SceneFlappyBird::~SceneFlappyBird()
{
}

void SceneFlappyBird::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;

	m_characterRadius = 5.0f;
	m_gravity = -57.6f;
	m_gameSpeed = 25.0f;

	m_spawnTime = 2.25f;
	m_spawnTimer = 0;
	m_spawnReduceTime = 4.0f;
	m_spawnReducerTimer = 0.0;

	m_pipeScaleX = 10;
	m_pipeScaleY = 55;

	m_pipeGapMin = 37.5f;
	m_pipeGapMax = 55.0f;

	m_jumpForce = 30;
	m_score = 0;
	m_highScore = 0;
	generation = 1;

	birds.clear();
	birdCount = 50;
	for (int i = 0; i < birdCount; ++i)
	{
		GameObject* go = FetchGO(GameObject::GO_BIRD);
		go->pos.x = m_worldWidth * 0.15f;
		go->pos.y = m_worldHeight * 0.5f;
		go->vel.SetZero();
		go->score = 0;
		go->alive = true;

		//setup nodes & weights
		RandomizeWeight(go);
		birds.push_back(go);
	}
}

void SceneFlappyBird::RandomizeWeight(GameObject* go)
{
	go->hiddenNode.resize(HIDDENSIZE);
	for (size_t i = 0; i < go->hiddenNode.size(); ++i)
	{
		go->hiddenNode[i].weights.resize(HIDDENSIZE + 1); //+1 accounting for bias
		for (size_t j = 0; j < HIDDENSIZE + 1; ++j)
		{
			go->hiddenNode[i].weights[j] = Math::RandFloatMinMax(-1.f, 1.f);
		}

		//go->hiddenNode[i].bias = Math::RandFloatMinMax(-1.f, 1.f);
	}

	go->hiddenNode2.resize(HIDDENSIZE);
	for (size_t i = 0; i < go->hiddenNode2.size(); ++i)
	{
		go->hiddenNode2[i].weights.resize(INPUTSIZE + 1); //+1 accounting for bias
		for (size_t j = 0; j < INPUTSIZE + 1; ++j)
		{
			go->hiddenNode2[i].weights[j] = Math::RandFloatMinMax(-1.f, 1.f);
		}

		//go->hiddenNode[i].bias = Math::RandFloatMinMax(-1.f, 1.f);
	}

	go->outputNode.weights.resize(go->hiddenNode2.size() + 1);
	for (size_t j = 0; j < go->outputNode.weights.size(); ++j)
	{
		go->outputNode.weights[j] = Math::RandFloatMinMax(-1.f, 1.f);
	}
	//go->outputNode.bias = Math::RandFloatMinMax(-1.f, 1.f);
}

GameObject* SceneFlappyBird::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (auto go : m_goList)
	{
		//GameObject* go = (GameObject*)*it;
		if (!go->active && type == go->type)
		{
			go->active = true;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 1; ++i)
	{
		GameObject* go = new GameObject(type);
		go->id = m_goList.size();
		m_goList.push_back(go);
	}
	return FetchGO(type);
}

float SceneFlappyBird::Sigmoid(float x)
{
	return 1.f / (1.f + exp(-x));
}

float SceneFlappyBird::Derivative(float x)
{
	return 0.f;
}

Pipes* SceneFlappyBird::FetchPipe()
{
	for (std::vector<Pipes*>::iterator it = m_pipesList.begin(); it != m_pipesList.end(); ++it)
	{
		Pipes* p = (Pipes*)*it;
		if (!p->m_active)
		{
			p->m_active = true;
			p->m_gap = Math::RandFloatMinMax(m_pipeGapMin, m_pipeGapMax);

			p->m_top.active = true;
			p->m_bottom.active = true;
			p->m_position.x = m_worldWidth * 0.75;
			p->m_position.y = Math::RandFloatMinMax(m_worldHeight * 0.35, m_worldHeight * 0.65f);

			p->m_aabbTop.Set(m_pipeScaleX - 2.0f, m_pipeScaleY - 1.0f, Vector3(p->m_position.x, p->m_position.y + p->m_gap, p->m_position.z));
			p->m_aabbBot.Set(m_pipeScaleX - 2.0f, m_pipeScaleY - 1.0f, Vector3(p->m_position.x, p->m_position.y - p->m_gap, p->m_position.z));
			return p;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		Pipes* pipe = new Pipes();
		pipe->m_active = false;
		pipe->m_top.type = GameObject::GO_PIPE;
		pipe->m_bottom.type = GameObject::GO_PIPE;
		pipe->m_top.active = false;
		pipe->m_bottom.active = false;

		m_pipesList.push_back(pipe);
	}
	return FetchPipe();
}

void SceneFlappyBird::UpdatePipes(double dt)
{
	for (std::vector<Pipes*>::iterator it = m_pipesList.begin(); it != m_pipesList.end(); ++it)
	{
		Pipes* p = (Pipes*)*it;
		if (p->m_active)
		{
			float value = m_gameSpeed * m_speed * dt;
			p->m_position.x -= value;
			p->m_aabbTop.Update(Vector3(-value, 0, 0));
			p->m_aabbBot.Update(Vector3(-value, 0, 0));

			//Check if it should die by the X
			if (p->m_position.x < -m_pipeScaleX / 2.0f)
			{
				p->m_active = false;
			}
		}
	}

	//Collision Check
	for (std::vector<Pipes*>::iterator it = m_pipesList.begin(); it != m_pipesList.end(); ++it)
	{
		Pipes* p = (Pipes*)*it;
		if (p->m_active)
		{
			for (auto go : m_goList)
			{
				if (go->alive)
				{
					if (AABBvsCIRCLE(p->m_aabbTop, go->pos, m_characterRadius))
					{
						go->alive = false;
					}

					if (AABBvsCIRCLE(p->m_aabbBot, go->pos, m_characterRadius))
					{
						go->alive = false;
					}
				}
			}
		}
	}
}

Pipes* SceneFlappyBird::GetNextPipe(float x)
{
	Pipes* nextP = NULL;
	float nearestX = FLT_MAX;
	for (auto p : m_pipesList)
	{
		if (p->m_active && p->m_position.x > x)
		{
			if (p->m_position.x < nearestX)
			{
				nearestX = p->m_position.x;
				nextP = p;
			}
		}
	}
	return nextP;
}

void SceneFlappyBird::UpdateCharacter(GameObject* go, double dt)
{
	if (go->alive)
	{
		go->score = m_score;

		go->vel.y += m_gravity * m_speed * static_cast<float>(dt);
		go->pos.y += go->vel.y * m_speed * static_cast<float>(dt);

		//Check if character hit limit
		if (go->pos.y < 0)
		{
			go->alive = false;
		}
		else if (go->pos.y > m_worldHeight)
		{
			go->alive = false;
		}
	}
}

void SceneFlappyBird::Update(double dt)
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
	//SPACE to JUMP
	static bool bSpaceState = false;
	if ((!bSpaceState && Application::IsKeyPressed(VK_SPACE)))
	{
		bSpaceState = true;
		if (m_goList.size() > 0)
			m_goList[0]->vel.Set(0, m_jumpForce, 0);
	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
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

	//To control the time between each pipe spawn
	m_spawnReducerTimer += dt * m_speed;
	if (m_spawnReducerTimer >= m_spawnReduceTime)
	{
		m_spawnReducerTimer = 0;
		m_spawnTime -= 0.1f;
		m_spawnTime = Math::Max(0.75f, m_spawnTime);
	}

	//To control the spawning of pipes
	m_spawnTimer += dt * m_speed;
	if (m_spawnTimer >= m_spawnTime)
	{
		m_spawnTimer = 0;
		FetchPipe();
	}

	//Update the pipes
	UpdatePipes(dt);

	for (auto go : m_goList)
	{
		if (go->alive)
		{
			Pipes* p = GetNextPipe(go->pos.x);
			float i0 = p != nullptr ? p->m_position.x - go->pos.x : m_worldWidth; //x-offset
			float i1 = p != nullptr ? p->m_position.y - go->pos.y : m_worldHeight*0.5f - go->pos.y;//m_worldHeight; //y-offset
			float i2 = go->vel.y;
			float i3 = p != nullptr ? p->m_gap : m_worldHeight;
			std::vector<float> inputList{ i0, i1, i2, i3, 1.f };
			float output = FeedNN(go, inputList);
			if (output > 0.5f)
			{
				go->vel.Set(0, m_jumpForce, 0);
			}
			UpdateCharacter(go, dt);
		}
	}
	//Update score & hiscore
	m_score += 100.0f * dt * m_speed;
	if (m_highScore < m_score)
	{
		m_highScore = m_score;
	}
	bool allDead = true;
	for (auto go : m_goList)
	{
		if (go->alive)
		{
			allDead = false;
			break;
		}
	}
	if (allDead)
		Restart();
}

float SceneFlappyBird::FeedNN(GameObject* go, std::vector<float>& inputList)
{
	//layer 1
	for (NNode& node : go->hiddenNode)
	{
		node.z = node.weights * inputList;
		node.output = Sigmoid(node.z);
	}

	std::vector<float> input1;
	for (int i = 0; i < HIDDENSIZE; ++i) {
		input1.push_back(go->hiddenNode[i].output);
	}
	input1.push_back(1.f);

	for (NNode& L2Node : go->hiddenNode2) {
		L2Node.z = L2Node.weights * input1;
		L2Node.output = Sigmoid(L2Node.z);
	}

	//layer 2
	std::vector<float> input2;
	for (int i = 0; i < go->hiddenNode2.size(); ++i)
		input2.push_back(go->hiddenNode2[i].output);
	input2.push_back(1.f);
	go->outputNode.z = go->outputNode.weights * input2;
	go->outputNode.output = Sigmoid(go->outputNode.z);

	return go->outputNode.output;
}

void SceneFlappyBird::Restart()
{
	m_score = 0;
	m_pipesList.clear();

	//Reset Spawn Timings
	m_spawnTime = 2.25f;
	m_spawnTimer = 0;
	m_spawnReduceTime = 4.0f;
	m_spawnReducerTimer = 0.0;

	//Mutate 50% bottom birds - reroll dice
	//Copy from strongest bird to weakest bird
	//Mutate rest a little
	int halfBirds = birdCount / 2;
	std::sort(birds.begin(), birds.end(), [](GameObject* lhs, GameObject* rhs) { return lhs->score < rhs->score; });
	GameObject* strongest = *(birds.end() - 1);

	float threshold = 0.05f; //0.025f
	//handle bottom 50%
	for (int i=0; i<halfBirds/2; ++i)
	{
		birds[i]->hiddenNode = strongest->hiddenNode;
		birds[i]->hiddenNode2 = strongest->hiddenNode2;
		birds[i]->outputNode = strongest->outputNode;

		for (NNode& node : birds[i]->hiddenNode)
		{
			for (float& f : node.weights)
				f += Math::RandFloatMinMax(-threshold, threshold);
			//node.bias += Math::RandFloatMinMax(-threshold, threshold);
		}

		for (NNode& node : birds[i]->hiddenNode2)
		{
			for (float& f : node.weights)
				f += Math::RandFloatMinMax(-threshold, threshold);
			//node.bias += Math::RandFloatMinMax(-threshold, threshold);
		}

		for (float& f : birds[i]->outputNode.weights)
			f += Math::RandFloatMinMax(-0.01f, 0.01f);
		//birds[i]->outputNode.bias += Math::RandFloatMinMax(-threshold, threshold);
	}
	for (int i = halfBirds/2; i < halfBirds; ++i)
	{
		RandomizeWeight(birds[i]);
	}

	//mutate the rest
	for (int i = halfBirds; i < birdCount-1; ++i)
	{
		for (NNode& node : birds[i]->hiddenNode)
		{
			for (float& f : node.weights)
				f += Math::RandFloatMinMax(-threshold, threshold);
			//node.bias += Math::RandFloatMinMax(-threshold, threshold);
		}

		for (NNode& node : birds[i]->hiddenNode2)
		{
			for (float& f : node.weights)
				f += Math::RandFloatMinMax(-threshold, threshold);
			//node.bias += Math::RandFloatMinMax(-threshold, threshold);
		}
	
		for (float& f : birds[i]->outputNode.weights)
			f += Math::RandFloatMinMax(-threshold, threshold);
		//birds[i]->outputNode.bias += Math::RandFloatMinMax(-threshold, threshold);
	}

	//Reset the birds 
	++generation;
	for (auto go : m_goList)
	{
		go->pos.y = m_worldHeight * 0.5f + Math::RandFloatMinMax(-10.f, 10.f);
		go->vel.SetZero();
		go->alive = true;
		go->score = 0;
	}
}

void SceneFlappyBird::RenderGO(GameObject* go)
{
	switch (go->type)
	{
		case GameObject::GO_BIRD:
		{
			modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, 0.01f * go->id);
			modelStack.Scale(m_characterRadius, m_characterRadius, 1);
			RenderMesh(meshList[GEO_CHARACTER], false);

			std::ostringstream ss;
			ss << go->id;
			RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
			modelStack.PopMatrix();
		}
		break;
	}
}

void SceneFlappyBird::RenderPipes()
{
	for (int i = 0; i < m_pipesList.size(); ++i)
	{
		Pipes* p = m_pipesList[i];
		if (p->m_active)
		{
			modelStack.PushMatrix();
			modelStack.Translate(p->m_position.x, p->m_position.y + p->m_gap, 0.1f);
			modelStack.Rotate(180, 0, 0, 1);
			modelStack.Scale(m_pipeScaleX, m_pipeScaleY, 1);
			RenderMesh(meshList[GEO_PIPE], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(p->m_position.x, p->m_position.y - p->m_gap, 0.1f);
			modelStack.Scale(m_pipeScaleX, m_pipeScaleY, 1);
			RenderMesh(meshList[GEO_PIPE], false);
			modelStack.PopMatrix();

			modelStack.PushMatrix();
			modelStack.Translate(p->m_position.x, p->m_position.y, 0.1f);
			modelStack.Scale(10, 10, 10);
			std::ostringstream ss;
			ss << i;
			RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
			modelStack.PopMatrix();
		}
	}
}

void SceneFlappyBird::Render()
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
	modelStack.Translate(m_worldWidth * 0.725 * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldWidth * 0.725, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_BG2], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth * 0.725 + m_worldWidth * 0.375 / 2.0f, m_worldHeight * 0.5f, 10.f);
	modelStack.Scale(m_worldWidth * 0.375, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_SIDEBAR], false);
	modelStack.PopMatrix();

	RenderPipes();

	for (auto go : m_goList)
	{
		if (go->alive)
		{
			RenderGO(go);
		}
	}

	//On screen text
	std::ostringstream ss;

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 0);

	ss.str("");
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 3);

	ss.str("");
	ss.precision(5);
	ss << "High Score:" << m_highScore;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 6);


	ss.str("");
	ss << "Score:" << m_score;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 9);


	ss.str("");
	ss << "Spawn Time:" << m_spawnTime;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 12);

	ss.str("");
	ss << "Generation:" << generation;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 15);
	ss.str("");

	//Render score of each bird
}

void SceneFlappyBird::Exit()
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
