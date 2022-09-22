//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

///////////////////////////////////////
//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	// Calculate vector to the target
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();  // Normalize the direction
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Multiple the direction with the speed

	// Draw DEBUG visualization
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color(0.0f, 1.0f, 0.0f) );
	}

	return steering;
}

///////////////////////////////////////
//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Needs a distance of how far we want to flee, what direction to flee in, what speed & what direction.
	// 
	SteeringOutput steering = {};

	// Calculate vector to the target & invert it, IF the target is within the start radius
	Elite::Vector2 fleeDirection{ pAgent->GetPosition() - m_Target.Position };  // Inverting the equation = inverted direction

	// Calculate vector to the target
	steering.LinearVelocity = fleeDirection;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Multiple the direction with the speed

	// Draw DEBUG visualization
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color(0.0f, 1.0f, 0.0f));
	}
	

	return steering;
}

///////////////////////////////////////
//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Similar to seek, but  stops when it's within a certain radius, and slows down within a certain radius.
	SteeringOutput steering = {};

	// Squared for small optimization (reduced usage of "sqrt")
	constexpr float arrivalRadiusSquared{ Elite::Square(1.0f) };
	constexpr float slowRadiusSquared{ Elite::Square(15.0f) };
	
	// Calculate vector to the target
	const Elite::Vector2 targetVector{ m_Target.Position - pAgent->GetPosition() };
	const float targetDistanceSquared{ targetVector.MagnitudeSquared() };


	if (targetDistanceSquared < arrivalRadiusSquared)
	{
		// Stop moving when agent reaches target
		steering.LinearVelocity = Elite::Vector2{ 0.0f,0.0f };
	}
	else
	{
		steering.LinearVelocity = targetVector;
		steering.LinearVelocity.Normalize();  // Normalize the direction

	
		if (targetDistanceSquared < slowRadiusSquared)
		{
			// Slow down when agent gets close to the target
			steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * targetDistanceSquared / slowRadiusSquared;
		}
		else
		{		
			steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // Multiple the direction with the speed
		}
	}

	// Draw DEBUG visualization
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color(0.0f, 1.0f, 0.0f));
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), sqrtf(slowRadiusSquared), Elite::Color(0.67f, 0.67f, 0.0f), 0.0f);
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), sqrtf(arrivalRadiusSquared), Elite::Color(0.67f, 0.67f, 0.0f), 0.0f);
	}

	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering{};
	// Face the agent towards the target (use the rotation speed and don't set the rotation)
	const Elite::Vector2 targetVector{ m_Target.Position - pAgent->GetPosition() };
	const Elite::Vector2 agentDirection{ std::cosf(pAgent->GetRotation()), std::sinf(pAgent->GetRotation()) };

	const float angleBetween{ Elite::AngleBetween(targetVector, agentDirection) };


	if (angleBetween > m_StopRotationAngle)
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();		
	}
	else if (angleBetween < -m_StopRotationAngle)
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}
	else
	{
		steering.AngularVelocity = 0.0f;
	}

	if (angleBetween >= -m_SlowRotationAngle && angleBetween <= m_SlowRotationAngle)
	{
		steering.AngularVelocity *= abs(angleBetween) / m_SlowRotationAngle;
	}

	pAgent->SetAutoOrient(false);


	// Draw DEBUG visualization
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), agentDirection, 5.0f, Elite::Color(1.0f, 1.0f, 0.0f));
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), targetVector, 5.0f, Elite::Color(0.0f, 1.0f, 0.0f));
		DEBUGRENDERER2D->DrawString(pAgent->GetPosition() + Elite::Vector2(1.5f, 1.5f), std::to_string(Elite::ToDegrees(angleBetween)).c_str());
	}
	
	return steering;



}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	// Pick a point at x distance in front of the agent.
	// In a radius around that point, pick a random point/angle on that circle, of which the edge is the target of the agent.
	// Can also use random offset to change that point.

	// Get the point at offsetdistance in front of the agent
	const Elite::Vector2 offsetPoint(pAgent->GetPosition() + (m_OffsetDistance * pAgent->GetDirection()));


	// Get a random wander angle
	m_WanderAngle += (rand() % int(m_MaxAngleChange * 200.0f) / 100.0f) - m_MaxAngleChange;

	// Get the point of the angle on the circle
	// (offSetPoint + (Radius * Angle)
	const Elite::Vector2 wanderTarget{ offsetPoint + Elite::Vector2(m_Radius * cosf(m_WanderAngle), m_Radius * sinf(m_WanderAngle))};



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(offsetPoint, m_Radius, Elite::Color(0.0f, 0.0f, 1.0f), 0.0f);
		DEBUGRENDERER2D->DrawPoint(offsetPoint, 5.0f, Elite::Color(0.0f, 0.0f, 1.0f), 0.0f);
		DEBUGRENDERER2D->DrawPoint(wanderTarget, 5.0f, Elite::Color(0.0f, 1.0f, 1.0f), 0.0f);
	}

	m_Target.Position = wanderTarget;

	return Seek::CalculateSteering(deltaT, pAgent);
}
