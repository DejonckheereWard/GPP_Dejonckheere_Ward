#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

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
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{ 0 }
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

	for (SteeringAgent* pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
	m_Neighbors.clear();
	m_NrOfNeighbors = 0;
}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

	// Loop over every agent
	for (SteeringAgent* pAgent : m_Agents)
	{
		// Register every neighbour
		RegisterNeighbors(pAgent);
		pAgent->Update(deltaT);
		if (m_TrimWorld)
		{
			// Trim the agent to the world
			pAgent->TrimToWorld(m_WorldSize);
		}

	}

}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
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

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// Reset neighbours to 0 before adding new ones
	m_NrOfNeighbors = 0;

	for (SteeringAgent* pOtherAgent : m_Agents)
	{
		// Check if not self
		if (pAgent == pOtherAgent)
			continue;

		// Check distance from pAgent & other agent
		// Keep it squared for performance
		const float distance = DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition());

		// Check if distance within squared neighbour radius
		if (distance < m_NeighborhoodRadius * m_NeighborhoodRadius)
		{
			// Add to memory pool
			m_Neighbors[m_NrOfNeighbors] = pOtherAgent;
			++m_NrOfNeighbors;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	// Loop over every neighbour and add their position to the total
	Elite::Vector2 total{};
	for (int i{}; i < m_NrOfNeighbors; i++)
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
	for (int i{}; i < m_NrOfNeighbors; i++)
	{
		total += m_Neighbors[i]->GetLinearVelocity();
	}
	total /= float(m_NrOfNeighbors);
	return total;
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
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
	
	m_pBlendedSteering = new BlendedSteering({ {m_pSeparationBehavior, 0.5f} });
	m_pPrioritySteering = new PrioritySteering({ m_pBlendedSteering });


	// Split up the view into grid and randomly assign an agent to that spot
	// Calc gridsize
	const int gridSize = int(std::ceilf(sqrtf(float(m_FlockSize))));
	for (int rowIndex{}; rowIndex < gridSize; rowIndex++)
	{
		for (int columnIndex{}; columnIndex < gridSize; columnIndex++)
		{
			// Calc agent count / index
			const int agentIndex{ rowIndex * gridSize + columnIndex };
			if (agentIndex >= m_FlockSize) break;  // Stop the function when enough are created
			std::cout << agentIndex << "\n";

			
			
			// Calc position (+ random to have some randomizations)
			const float x = ((float(columnIndex) + Elite::randomFloat(0.1f, 0.9f)) / float(gridSize)) * m_WorldSize;
			const float y = ((float(rowIndex) + Elite::randomFloat(0.1f, 0.9f)) / float(gridSize)) * m_WorldSize;

			// Create agent
			SteeringAgent* pAgent = new SteeringAgent();

			// Set position & init other agent variables
			pAgent->SetPosition(Elite::Vector2{ x, y });
			pAgent->SetSteeringBehavior(m_pPrioritySteering);
			pAgent->SetMaxLinearSpeed(15.0f);
			pAgent->SetAutoOrient(true);
			pAgent->SetBodyColor({ 1, 0, 0, 0 });
			pAgent->SetMass(0.3f);
			

			// Add to flock pool
			m_Agents[agentIndex] = pAgent;

		}
	}
	
	// Turn on debug for 1 agent
	m_Agents[45]->SetRenderBehavior(true);


}
