#include "stdafx.h"
#include "App_AgarioGame_IM.h"
#include "Behaviors_IM.h"

#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"
#include "projects/Shared/NavigationColliderElement.h"

using namespace Elite;
App_AgarioGame_IM::App_AgarioGame_IM()
{

}

App_AgarioGame_IM::~App_AgarioGame_IM()
{
	for(auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for(auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pSmartAgent);

	for(auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();
	SAFE_DELETE(m_pContactListener);

	SAFE_DELETE(m_pInfluenceGrid);
}

void App_AgarioGame_IM::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize * .5f, m_TrimWorldSize * .5f));
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(m_TrimWorldSize * .55f);

	//Create Boundaries
	const float blockSize{ 2.0f };
	const float hBlockSize{ blockSize / 2.0f };
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(-hBlockSize, m_TrimWorldSize * .5f), blockSize, m_TrimWorldSize + blockSize * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize + hBlockSize, m_TrimWorldSize * .5f), blockSize, m_TrimWorldSize + blockSize * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize * .5f, m_TrimWorldSize + hBlockSize), m_TrimWorldSize, blockSize));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldSize * .5f, -hBlockSize), m_TrimWorldSize, blockSize));

	// Initialize influence grid
	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(m_InfluenceMapCols, m_InfluenceMapRows, (int)m_InfluenceMapCellSize, false, true);
	m_pInfluenceGrid->InitializeBuffer();

	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for(int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0.f, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for(int i = 0; i < m_AmountOfAgents; i++)
	{
		Vector2 randomPos = randomVector2(0.f, m_TrimWorldSize);
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		//1. Create Blackboard
		Blackboard* pBlackboard = CreateBlackboard(newAgent);

		//2. Create BehaviorTree
		//BehaviorTree* pBehaviorTree = new BehaviorTree(pBlackboard, new BehaviorAction(BT_Actions_IM::ChangeToWander));
		BehaviorTree* pBehaviorTree = new BehaviorTree(pBlackboard,
			new BehaviorSelector(
				{
					new BehaviorSequence(
					{
						new BehaviorConditional(BT_Conditions_IM::IsFoodNearby),
						new BehaviorAction(BT_Actions_IM::ChangeToSeek)
					}),
					new BehaviorAction(BT_Actions_IM::ChangeToWander)
				}
		));

		//3. Set the BehaviorTree active on the agent 
		newAgent->SetDecisionMaking(pBehaviorTree);
		m_pAgentVec.push_back(newAgent);
	}


	//-------------------
	//Create The Uber Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0.f, m_TrimWorldSize);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pSmartAgent = new AgarioAgent(randomPos, customColor);

	//Create and add the necessary blackboard data
	//1. Create Blackboard
	Blackboard* pBlackboard = CreateBlackboard(m_pSmartAgent);

	//2. Create BehaviorTree (make more conditions/actions and create a more advanced tree than the simple agents
	BehaviorTree* pBehaviorTree = new BehaviorTree(pBlackboard,
		new BehaviorSelector(
			{
				new BehaviorSequence({
					new BehaviorSelector({
						new BehaviorSequence({
							new BehaviorConditional(BT_Conditions_IM::IsBiggerAgentNearby),
							new BehaviorAction(BT_Actions_IM::ChangeToFlee)
						}),
						new BehaviorSequence({
							new BehaviorConditional(BT_Conditions_IM::IsSmallerAgentNearby),
							new BehaviorAction(BT_Actions_IM::ChangeToSeek)
						})
					})
				}),
				new BehaviorSequence(
				{
					new BehaviorConditional(BT_Conditions_IM::IsFoodNearby),
					new BehaviorAction(BT_Actions_IM::ChangeToSeek)
				}),
				new BehaviorAction(BT_Actions_IM::ChangeToWander)
			}
	));

	//3. Set the BehaviorTree active on the agent 
	m_pSmartAgent->SetDecisionMaking(pBehaviorTree);
	m_pSmartAgent->SetRenderBehavior(true);
	//m_pAgentVec.push_back(m_pSmartAgent);

}

void App_AgarioGame_IM::Update(float deltaTime)
{
	UpdateImGui();

	m_GridEditor.UpdateGraph(m_pInfluenceGrid);

	//Check if agent is still alive
	if(m_pSmartAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}
	//Update the custom agent
	m_pSmartAgent->Update(deltaTime);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);


	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if(m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0.f, m_TrimWorldSize)));
	}

	// Update the influence grid
	// Negative values represent the radius of enemies
	// Positive values represent food

	for(AgarioAgent* pEnemyAgent : m_pAgentVec)
	{
		// Get index of the agent using its worldposition
		// Update the influence in that cell, setting it to the radius of this enemy but negative
		const float enemyRadius{ pEnemyAgent->GetRadius() };
		const float agentRadius{ m_pSmartAgent->GetRadius() };
		if(enemyRadius > agentRadius)
		{			
			m_pInfluenceGrid->SetInfluenceAtPosition(pEnemyAgent->GetPosition(), -(enemyRadius - agentRadius) * 10.0f);
		}
		else 
		{
			m_pInfluenceGrid->SetInfluenceAtPosition(pEnemyAgent->GetPosition(), (agentRadius - enemyRadius) * 10.0f);
		}
	}

	for(AgarioFood* pFood : m_pFoodVec)
	{
		// Giving food a value higher then 1 for propagation of influence
		m_pInfluenceGrid->SetInfluenceAtPosition(pFood->GetPosition(), 50.0f);
	}

	m_pInfluenceGrid->PropagateInfluence(deltaTime);


}

void App_AgarioGame_IM::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);



	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
	if(m_RenderInfluenceMap)
		m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);

	for(AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for(AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pSmartAgent->Render(deltaTime);
}

Blackboard* App_AgarioGame_IM::CreateBlackboard(AgarioAgent* a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("AgentsVec", &m_pAgentVec);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	pBlackboard->AddData("Target", Elite::Vector2{});
	pBlackboard->AddData("AgentFleeTarget", static_cast<AgarioAgent*>(nullptr)); // Needs the cast for the type
	pBlackboard->AddData("Time", 0.0f);
	pBlackboard->AddData("InfluenceMap", m_pInfluenceGrid);



	return pBlackboard;
}

void App_AgarioGame_IM::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();

		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f", m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());


		ImGui::Checkbox("Render Influence Map", &m_RenderInfluenceMap);

		auto momentum = m_pInfluenceGrid->GetMomentum();
		auto decay = m_pInfluenceGrid->GetDecay();
		auto propagationInterval = m_pInfluenceGrid->GetPropagationInterval();

		ImGui::SliderFloat("Momentum", &momentum, 0.0f, 1.f, "%.2f");
		ImGui::SliderFloat("Decay", &decay, 0.f, 1.f, "%.2f");
		ImGui::SliderFloat("Propagation Interval", &propagationInterval, 0.f, 2.f, "%.2f");
		ImGui::Spacing();

		//Set data
		m_pInfluenceGrid->SetMomentum(momentum);
		m_pInfluenceGrid->SetDecay(decay);
		m_pInfluenceGrid->SetPropagationInterval(propagationInterval);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width / 2.0f - menuWidth, height / 2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
