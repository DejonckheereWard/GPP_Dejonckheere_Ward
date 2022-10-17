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

	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Calculate inverse proportional magnitude vector to each neighbor
	// Take the average vector of all found neighbors and set that as seek point
	
	// Get the neighbors & amount of neighbors
	const std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	const size_t neighborCount{ size_t(m_pFlock->GetNrOfNeighbors()) };
	
	// Vector to store total inverse proportional magnitude vector
	Elite::Vector2 totalInverseProportionalMagnitudeVector{};
	
	
	for (size_t neighborIndex{}; neighborIndex < neighborCount; neighborIndex++)
	{
		// Get the distance to the neighbor
		Elite::Vector2 vectorToNeighbor{ pAgent->GetPosition() - neighbors[neighborIndex]->GetPosition() };
		const float distanceToNeighborSquared{ vectorToNeighbor.MagnitudeSquared() };

		// Get the inverse proportional magnitude vector
		const Elite::Vector2 inverseProportionalMagnitude{ vectorToNeighbor / distanceToNeighborSquared };

		// Add the inverse proportional magnitude vector to the average vector
		totalInverseProportionalMagnitudeVector += inverseProportionalMagnitude;
	}
	totalInverseProportionalMagnitudeVector /= float(neighborCount);

	// Set the target to position + offset
	m_Target.Position = pAgent->GetPosition() + totalInverseProportionalMagnitudeVector;
	
	// Show debug visuals if enabled
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawSegment(pAgent->GetPosition(), m_Target.Position, Elite::Color(1, 0, 0));
	}
	
	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	// Get the average velocity of the neighbors
	const Elite::Vector2 averageVelocity{ m_pFlock->GetAverageNeighborVelocity() };

	// Set our target to a point offset using this average velocity
	m_Target.Position = pAgent->GetPosition() + averageVelocity;


	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };
	return steering;
}
