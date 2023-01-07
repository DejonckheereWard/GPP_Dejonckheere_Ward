#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/HierarchicalSpacePartitioning.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 5.0f }
	, m_NrOfNeighbors{ 0 }
	, m_pQuadCellSpace{ new QuadCellSpace(Boundary({worldSize / 2.0f, worldSize / 2.0f}, worldSize / 2.0f), 1) }
	, m_pCellSpace{ new CellSpace(worldSize, worldSize, 100, 100, flockSize) }
	, m_DrawCellAgentCount{ false }
	, m_DrawNeighborCells{ true }
{
	m_Agents.resize(m_FlockSize);

	// TODO: initialize the flock and the memory pool
	InitializeFlock();
	m_Neighbors.resize(m_FlockSize - 1);  // Don't include self in total amount of neighbours
}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	for(SteeringAgent* pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
	m_Neighbors.clear();
	m_NrOfNeighbors = 0;

	SAFE_DELETE(m_pCellSpace);
	SAFE_DELETE(m_pQuadCellSpace);
}

void Flock::Update(float deltaT)
{
	// Reinit the quadtree every update
	if(m_UseQuadCellSpace)
	{
		m_pQuadCellSpace->Clear();
		for(SteeringAgent* pAgent : m_Agents)
		{
			m_pQuadCellSpace->AddAgent(pAgent);
		}
	}

	TargetData evadeTarget{};
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.AngularVelocity = m_pAgentToEvade->GetAngularVelocity();


	// Loop over every agent


	for(SteeringAgent* pAgent : m_Agents)
	{
		m_pEvadeBehavior->SetTarget(evadeTarget);
		// Register every neighbour

		// Check if agent moved to new cell

		if(!m_UseQuadCellSpace)
		{
			m_pCellSpace->UpdateAgentCell(pAgent);
			if(m_UseSpacePartitioning)
				m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
		}
		RegisterNeighbors(pAgent);
		pAgent->Update(deltaT);
		pAgent->SetOldPosition(pAgent->GetPosition());
		if(m_TrimWorld)
		{
			// Trim the agent to the world
			pAgent->TrimToWorld(m_WorldSize);
		}
	}

}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	if(m_RenderAgents)
	{
		for(SteeringAgent* pAgent : m_Agents)
		{
			pAgent->Render(deltaT);
		}
	}

	if(m_CanDebugRender)
	{
		const Elite::Color boundingBoxColor{ Elite::Color(1.0f, 0.67f, 0.0f) }; // Orange?
		const Elite::Color neighborhoodRadiusColor{ Elite::Color(1.0f, 0.0f, 0.0f) }; // Red?
		const Elite::Color neighborHighlightColor{ Elite::Color(0.0f, 0.67f, 1.0f) }; // Blue?


		// Get the neigbors of agent 0
		// Render neighbors of agent 0
		// Show all neighbors in the flock
		// Get the neighbors & amount of neighbors
		SteeringAgent* agentToDebug{ m_Agents[0] };

		// Update the neighbors to get the correct ones for the AgentToDebug (latest value will be of a (random) agent)
		m_pCellSpace->RegisterNeighbors(agentToDebug, m_NeighborhoodRadius);
		RegisterNeighbors(agentToDebug);
		const std::vector<SteeringAgent*> neighbors{ GetNeighbors() };
		const size_t neighborCount{ size_t(GetNrOfNeighbors()) };

		// Draw the bounding box
		if(m_UseSpacePartitioning)
		{
			// Draw the cell grid
			if(m_UseQuadCellSpace)
			{
				m_pQuadCellSpace->Render();
				if(m_DrawNeighborCells)
					m_pQuadCellSpace->RenderActiveCells(Boundary(agentToDebug->GetPosition(), m_NeighborhoodRadius));
			}
			else
			{

				m_pCellSpace->RenderCells();

				//Draw active neighbor cells for the given agent
				m_pCellSpace->SetDrawCellAgentCount(m_DrawCellAgentCount);

				if(m_DrawNeighborCells)
					m_pCellSpace->RenderActiveCells();
			}


			std::vector<Elite::Vector2> boundingBoxPoints{};
			const Elite::Vector2 agentPos{ agentToDebug->GetPosition() };
			boundingBoxPoints.push_back({ agentPos.x - m_NeighborhoodRadius, agentPos.y - m_NeighborhoodRadius });
			boundingBoxPoints.push_back({ agentPos.x - m_NeighborhoodRadius, agentPos.y + m_NeighborhoodRadius });
			boundingBoxPoints.push_back({ agentPos.x + m_NeighborhoodRadius, agentPos.y + m_NeighborhoodRadius });
			boundingBoxPoints.push_back({ agentPos.x + m_NeighborhoodRadius, agentPos.y - m_NeighborhoodRadius });
			DEBUGRENDERER2D->DrawPolygon(boundingBoxPoints.data(), boundingBoxPoints.size(), boundingBoxColor, 0.8f);

		}

		// Draw neighborhoodradius
		DEBUGRENDERER2D->DrawCircle(agentToDebug->GetPosition(), GetNeighborhoodRadius(), neighborhoodRadiusColor, 0.0f);
		for(size_t neighborIndex{}; neighborIndex < neighborCount; neighborIndex++)
		{
			// Highlight the neighbors 
			if(m_HighlightNeighbors)
				DEBUGRENDERER2D->DrawCircle(neighbors[neighborIndex]->GetPosition(), agentToDebug->GetRadius(), neighborHighlightColor, 0.0f);

		}
	}


}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
	if(m_CanDebugRender)
	{
		ImGui::Checkbox("Draw Neighbor Cells", &m_DrawNeighborCells);
		ImGui::Checkbox("Highlight Neighbor Agents", &m_HighlightNeighbors);
		ImGui::Checkbox("Draw Cell Agent Count", &m_DrawCellAgentCount);
	}
	ImGui::Spacing();
	ImGui::Checkbox("Spatial Partitioning", &m_UseSpacePartitioning);

	if(m_UseSpacePartitioning)
	{
		ImGui::Checkbox("Use Quadtree cells", &m_UseQuadCellSpace);
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SliderFloat("Neighborhood Radius", &m_NeighborhoodRadius, 3.f, 20.f, "%.1f");
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Velocity Match", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2f");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// Reset neighbours to 0 before adding new ones
	m_NrOfNeighbors = 0;


	std::vector<SteeringAgent*> agentList{};
	size_t agentAmount{};

	if(m_UseSpacePartitioning)
	{
		if(m_UseQuadCellSpace)
		{
			agentList = m_pQuadCellSpace->QueryRange(Boundary(pAgent->GetPosition(), m_NeighborhoodRadius));
			agentAmount = agentList.size();
		}
		else
		{
			agentList = m_pCellSpace->GetNeighbors();
			agentAmount = m_pCellSpace->GetNrOfNeighbors();
		}
	}
	else
	{
		agentList = m_Agents;
		agentAmount = m_Agents.size();
	}

	for(size_t agentIndex{}; agentIndex < agentAmount; ++agentIndex)
	{
		// Check if not self
		if(pAgent == agentList[agentIndex])
			continue;

		// Check distance from pAgent & other agent
		// Keep it squared for performance
		const Vector2 vectorToAgent = agentList[agentIndex]->GetPosition() - pAgent->GetPosition();
		const float distanceSquared = vectorToAgent.MagnitudeSquared();

		// Check if distance within squared neighbour radius
		if(distanceSquared < Square(m_NeighborhoodRadius))
		{
			// Add to memory pool & increase m_NrOfNeighbors afterwards (++ behind the var)
			m_Neighbors[m_NrOfNeighbors++] = agentList[agentIndex];
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	// Loop over every neighbour and add their position to the total
	Elite::Vector2 total{};
	for(int i{}; i < m_NrOfNeighbors; i++)
	{
		total += m_Neighbors[i]->GetPosition();
	}
	total /= float(m_NrOfNeighbors);

	return total;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	// Loop over every neighbour and add the velocities
	Elite::Vector2 total{};
	for(int i{}; i < m_NrOfNeighbors; i++)
	{
		total += m_Neighbors[i]->GetLinearVelocity();
	}
	total /= float(m_NrOfNeighbors);
	return total;
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
	m_pSeekBehavior->SetTarget(target);
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if(m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::InitializeFlock()
{
	// Initializes the steering behaviours for the flock	
	// Initializes the all flock agents and gives them a position evenly on the grid

	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(50.0f);

	m_pBlendedSteering = new BlendedSteering({
		{m_pCohesionBehavior, 0.45f},
		{m_pSeparationBehavior, 0.52f},
		{m_pVelMatchBehavior, 0.23f},
		{m_pSeekBehavior, 0.00f},
		{m_pWanderBehavior, 0.60f}
		});
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });


	// Split up the view into grid and randomly assign an agent to that spot
	// Calc gridsize
	const int gridSize = int(std::ceilf(sqrtf(float(m_FlockSize))));
	for(int rowIndex{}; rowIndex < gridSize; rowIndex++)
	{
		for(int columnIndex{}; columnIndex < gridSize; columnIndex++)
		{
			// Calc agent count / index
			const int agentIndex{ rowIndex * gridSize + columnIndex };
			if(agentIndex >= m_FlockSize) break;  // Stop the function when enough are created
			std::cout << agentIndex << "\n";



			// Calc position (+ random to have some randomizations)
			const float x = ((float(columnIndex) + Elite::randomFloat(0.1f, 0.9f)) / float(gridSize)) * m_WorldSize;
			const float y = ((float(rowIndex) + Elite::randomFloat(0.1f, 0.9f)) / float(gridSize)) * m_WorldSize;

			// Create agent
			SteeringAgent* pAgent = new SteeringAgent();

			// Set position & init other agent variables
			pAgent->SetPosition(Elite::Vector2{ x, y });
			pAgent->SetOldPosition(pAgent->GetPosition());
			pAgent->SetSteeringBehavior(m_pPrioritySteering);
			pAgent->SetMaxLinearSpeed(55.0f);
			pAgent->SetMaxAngularSpeed(25.0f);
			pAgent->SetAutoOrient(true);
			pAgent->SetMass(1.0f);


			// Add to flock pool
			m_Agents[agentIndex] = pAgent;

			// Add agent to the partitioning cells
			m_pCellSpace->AddAgent(pAgent);

		}
	}

}
