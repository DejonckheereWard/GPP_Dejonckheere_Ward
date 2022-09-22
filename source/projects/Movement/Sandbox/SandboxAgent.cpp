#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	const float maxSpeed{ 50.0f };
	constexpr float arrivalRadiusSqr{ Square(1.0f) }; // Margin for target (within these bounds -> target reached)
	constexpr float slowRadiusSqr{ Square(15.0f) };

	Elite::Vector2 toTarget{ m_Target - GetPosition() };  // Get the vector from the start point to the target
	const float distanceSqr{ toTarget.MagnitudeSquared() };  // Distance to the target

	if (distanceSqr < arrivalRadiusSqr)
	{
		SetLinearVelocity({ 0.0f, 0.0f });
		return;
	}

	Elite::Vector2 velocity{ toTarget };
	velocity.Normalize();
	if (distanceSqr < slowRadiusSqr)
	{
		velocity *= maxSpeed * distanceSqr / slowRadiusSqr;
	}
	else
	{
		velocity *= maxSpeed;
	}
	

	SetLinearVelocity(velocity);

	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2(velocity.y, velocity.x) + float(E_PI_2));
	}

	SetRotation(GetRotation() + float(E_PI_2));
}