/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"


//------------
//---STATES---
//------------
namespace FSMStates 
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeState : public Elite::FSMState
	{
	public:
		FleeState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		//virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
		
	};
}


//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class FoodNearBy : public Elite::FSMCondition
	{
	public:
		FoodNearBy() : FSMCondition() {};


		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class FoodGone : public Elite::FSMCondition
	{
	public:
		FoodGone() : FSMCondition() {};


		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class BiggerAgentNearby: public Elite::FSMCondition
	{
	public:
		BiggerAgentNearby() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class BiggerAgentGone: public Elite::FSMCondition
	{
	public:
		BiggerAgentGone() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};
}

#endif