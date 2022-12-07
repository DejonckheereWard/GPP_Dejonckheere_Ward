/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) 
		{
			return Elite::BehaviorState::Failure;
		}

		//std::cout << "Wandering\n";

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		if (!pBlackboard->GetData("Target", targetPos))
		{
			return Elite::BehaviorState::Failure;
		}

		//std::cout << "Seeking\n";


		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		AgarioAgent* pFleeTarget = nullptr;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		if (!pBlackboard->GetData("AgentFleeTarget", pFleeTarget) || pFleeTarget == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		
		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawCircle(pFleeTarget->GetPosition(), pFleeTarget->GetRadius(), Elite::Color(1.0f, 0, 0), DEBUGRENDERER2D->NextDepthSlice());
		}
		pAgent->SetToFlee(pFleeTarget->GetPosition());
		return Elite::BehaviorState::Success;

	}
}


namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioFood*>* pFoodVec = nullptr;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		{
			return false;
		}

		const float searchRadius{ 50.0f + pAgent->GetRadius() };
		
		AgarioFood* pClosestFood = nullptr;
		float closestDistSqr{ searchRadius * searchRadius };

		// TODO: Debug Rendering

		Elite::Vector2 agentPos{ pAgent->GetPosition() };
		for (auto& pFood : *pFoodVec)
		{
			float distSq = pFood->GetPosition().DistanceSquared(agentPos);
			if (distSq < closestDistSqr)
			{
				pClosestFood = pFood;
				closestDistSqr = distSq;
			}
		}

		if (pClosestFood)
		{			
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}
		
		return false;		
	}

	bool IsBiggerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioAgent*>* pAgentsVec = nullptr;  // List of all non player agents
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || pAgentsVec == nullptr)
		{
			return false;
		}


		const float fleeRadius{ 30.0f + pAgent->GetRadius() };
		AgarioAgent* pFleeAgent = nullptr;
		float closestDist{ fleeRadius };

		Elite::Vector2 agentPos{ pAgent->GetPosition() };
		for (auto& pEnemyAgent : *pAgentsVec)
		{
			//if (pEnemyAgent == pAgent)
			//	continue;

			if ((pEnemyAgent->GetRadius() - 1.0f) > pAgent->GetRadius())
			{
				const float dist = pEnemyAgent->GetPosition().Distance(agentPos) - pEnemyAgent->GetRadius();
				if (dist < closestDist)
				{
					pFleeAgent = pEnemyAgent;
					closestDist = dist;
				}				
			}
		}

		if (pFleeAgent)
		{
			pBlackboard->ChangeData("AgentFleeTarget", pFleeAgent);
			return true;
		}

		return false;
	}

	bool IsSmallerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioAgent*>* pAgentsVec = nullptr;  // List of all non player agents
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || pAgentsVec == nullptr)
		{
			return false;
		}

		const float chaseRadius{ 10.0f + pAgent->GetRadius() };

		AgarioAgent* pTargetAgent = nullptr;
		float closestDistSqr{ chaseRadius * chaseRadius };

		Elite::Vector2 agentPos{ pAgent->GetPosition() };
		for (auto& pEnemyAgent : *pAgentsVec)
		{
			//if (pEnemyAgent == pAgent)
			//	continue;

			if ((pEnemyAgent->GetRadius() + 1) < pAgent->GetRadius())
			{
				const float distSq = pEnemyAgent->GetPosition().DistanceSquared(agentPos);
				if (distSq < closestDistSqr)
				{
					pTargetAgent = pEnemyAgent;
					closestDistSqr = distSq;
				}
			}
		}

		if (pTargetAgent)
		{
			pBlackboard->ChangeData("Target", pTargetAgent->GetPosition());
			return true;
		}

		return false;
	}
}











#endif