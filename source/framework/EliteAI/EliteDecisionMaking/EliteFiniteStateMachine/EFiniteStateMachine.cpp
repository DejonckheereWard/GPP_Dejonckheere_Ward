//=== General Includes ===
#include "stdafx.h"
#include "EFiniteStateMachine.h"
using namespace Elite;

FiniteStateMachine::FiniteStateMachine(FSMState* startState, Blackboard* pBlackboard)
    : m_pCurrentState(nullptr),
    m_pBlackboard(pBlackboard)
{
    ChangeState(startState);
}

FiniteStateMachine::~FiniteStateMachine()
{
    SAFE_DELETE(m_pBlackboard);
}

void FiniteStateMachine::AddTransition(FSMState* startState, FSMState* toState, FSMCondition* condition)
{
    auto it = m_Transitions.find(startState);
    if (it == m_Transitions.end())
    {
        m_Transitions[startState] = Transitions();
    }
   
    m_Transitions[startState].push_back(std::make_pair(condition, toState));
}

void FiniteStateMachine::Update(float deltaTime)
{
    //TODO 4: Look if 1 or more transition exists for the current state that we are in
    //Tip: Check the transitions map for a TransitionState pair
    auto transitionIt = m_Transitions.find(m_pCurrentState);
	
    //TODO 5: if a TransitionState exists
    if (transitionIt != m_Transitions.end())
    {
        //TODO 6: Loop over all the TransitionState pairs 
        for (auto& transition : transitionIt->second)
        {
            FSMCondition* cond = transition.first;
            FSMState* state = transition.second;
			
			assert(cond && state);

			if (cond->Evaluate(m_pBlackboard))
			{
                //TODO 8: Update the current state (if one exists)
				ChangeState(transition.second);
                return;
			}
        }
    }
    
}

Blackboard* FiniteStateMachine::GetBlackboard() const
{
    return m_pBlackboard;
}

void FiniteStateMachine::ChangeState(FSMState* newState)
{
    //TODO 1. If currently in a state => make sure the OnExit of that state gets called
    if (m_pCurrentState != nullptr)
    {
		m_pCurrentState->OnExit(m_pBlackboard);
    }

    //TODO 2. Change the current state to the new state
    m_pCurrentState = newState;

    assert(m_pCurrentState != nullptr);

    //TODO 3. Call the OnEnter of the new state
	m_pCurrentState->OnEnter(m_pBlackboard);
        
}
