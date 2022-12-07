#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;


//------------
//---STATES---
//------------

void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Entered Wander State" << "\n";


	AgarioAgent* pAgent;

	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return;
	}
	assert(pAgent != nullptr);

	pAgent->SetToWander();

}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Entered SeekFood State" << "\n";

	AgarioAgent* pAgent;
	AgarioFood* pFood;
	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return;
	}
	if (pBlackboard->GetData("FoodPtr", pFood) == false || pFood == nullptr)
	{
		return;
	}

	pAgent->SetToSeek(pFood->GetPosition());


}

void FSMStates::FleeState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::cout << "Entered Flee State" << "\n";

	AgarioAgent* pAgent;
	AgarioAgent* pFleeAgent;
	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return;
	}
	if (pBlackboard->GetData("FleeAgentPtr", pFleeAgent) == false || pFleeAgent == nullptr)
	{
		return;
	}

	pAgent->SetToFlee(pFleeAgent->GetPosition());

}


//-----------------
//---TRANSITIONS---
//-----------------


bool FSMConditions::FoodNearBy::Evaluate(Blackboard* pBlackboard) const
{

	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;
	Vector2 agentPos{};


	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return false;
	}

	if (pBlackboard->GetData("FoodVecPtr", pFoodVec) == false || pFoodVec == nullptr)
	{
		return false;
	}

	agentPos = pAgent->GetPosition();
	const float foodRadius{ 50.f + pAgent->GetRadius() };

	DEBUGRENDERER2D->DrawCircle(agentPos, foodRadius, Color{ 1.f, 1.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());

	// Iterator to find the closest food, will be end iterator if not found
	auto elementDist = [agentPos](AgarioFood* pFood, AgarioFood* pFood2)
	{
		float dist1 = agentPos.DistanceSquared(pFood->GetPosition());
		float dist2 = agentPos.DistanceSquared(pFood2->GetPosition());
		return dist1 < dist2;
	};
	auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), elementDist);

	if (closestFoodIt != pFoodVec->end())
	{
		AgarioFood* pFood = *closestFoodIt;

		float distanceToFood = agentPos.DistanceSquared(pFood->GetPosition());

		if (distanceToFood < foodRadius * foodRadius)
		{
			pBlackboard->ChangeData("FoodPtr", pFood);
			return true;
		}
	}
	//std::cout << "\n";


	return false;
}

bool FSMConditions::FoodGone::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioFood* pFood;
	std::vector<AgarioFood*>* pFoodVec;
	if (pBlackboard->GetData("FoodPtr", pFood) == false || pFood == nullptr)
	{
		return true;
	}
	if (pBlackboard->GetData("FoodVecPtr", pFoodVec) == false || pFoodVec == nullptr)
	{
		return true;
	}

	if (std::find(pFoodVec->begin(), pFoodVec->end(), pFood) == pFoodVec->end())
	{
		return true;
	}

	return false;
}

bool FSMConditions::BiggerAgentNearby::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent{ nullptr };
	std::vector<AgarioAgent*>* pAgentVec{ nullptr };

	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return false;
	}
	if (pBlackboard->GetData("AgentVecPtr", pAgentVec) == false || pAgentVec == nullptr)
	{
		return false;
	}

	// Loop through all agents and check if any of them are bigger than me and within flee radius
	const float fleeRadius{ 10.0f + pAgent->GetRadius() };

	AgarioAgent* pFleeAgent{};
	float distanceToFleeAgentSqr = FLT_MAX;

	for (AgarioAgent* enemyAgent : *pAgentVec)
	{
		const float distanceToAgentSqr{ pAgent->GetPosition().DistanceSquared(enemyAgent->GetPosition()) + Square(enemyAgent->GetRadius()) };

		// Check if the agent we are checking is withing flee radius,
		// Also include the enemys radius
		if (distanceToAgentSqr < fleeRadius * fleeRadius)
		{
			// Check if the agent is bigger
			if (enemyAgent->GetRadius() > pAgent->GetRadius())
			{
				// Check if the agent is closer than the previous one
				if (distanceToAgentSqr < distanceToFleeAgentSqr)
				{
					pFleeAgent = enemyAgent;
					distanceToFleeAgentSqr = distanceToAgentSqr;
				}
			}
		}
	}

	if (pFleeAgent != nullptr)
	{
		pBlackboard->ChangeData("FleeAgentPtr", pFleeAgent);
		return true;
	}

	return false;
}

bool FSMConditions::BiggerAgentGone::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	AgarioAgent* pFleeAgent;

	if (pBlackboard->GetData("AgentPtr", pAgent) == false || pAgent == nullptr)
	{
		return false;
	}
	if (pBlackboard->GetData("FleeAgentPtr", pFleeAgent) == false || pFleeAgent == nullptr)
	{
		return false;
	}

	if (pAgent && pFleeAgent)
	{
		const float fleeRadius{ 10.0f + pAgent->GetRadius() };
		const float distanceToAgentSqr{ pAgent->GetPosition().DistanceSquared(pFleeAgent->GetPosition()) + Square(pFleeAgent->GetRadius()) };

		if (distanceToAgentSqr > fleeRadius)
		{
			// The flee agent is gone
			pBlackboard->ChangeData("FleeAgentPtr", nullptr);
			return true;
		}
	}
	return false;

}



