#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	// Move to the center of the neighborhood
	// Inherits from seek, so return Seek behaviour after setting the new target

	// Get the average position of the neighbors
	const Elite::Vector2 averagePos{ m_pFlock->GetAverageNeighborPos() };

	// Set the target to the average position
	m_Target.Position = averagePos;

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	// Show debug visuals if enabled
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5.0f, Elite::Color(0.0f, 0.67f, 1.0f), 0.0f);

		// Show all neighbors in the flock
		// Get the neighbors & amount of neighbors
		const std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
		const size_t neighborCount{ size_t(m_pFlock->GetNrOfNeighbors()) };
		
		for (size_t neighborIndex{}; neighborIndex < neighborCount; neighborIndex++)
		{
			DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 15.0f, Elite::Color(0.1f, 0.9f, 0.1f), 0.0f);
			DEBUGRENDERER2D->DrawCircle(neighbors[neighborIndex]->GetPosition(), pAgent->GetRadius(), Elite::Color(0.0f, 0.67f, 1.0f), 0.0f);
		}
	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)