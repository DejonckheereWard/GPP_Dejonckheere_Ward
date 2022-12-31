#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include <framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphEditor.h>

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;
class NavigationColliderElement;

class App_AgarioGame_IM final : public IApp
{
public:
	App_AgarioGame_IM();
	~App_AgarioGame_IM();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
	//using InfluenceGraph = Elite::Graph2D<Elite::InfluenceNode, Elite::GraphConnection2D>;
private:
	float m_TrimWorldSize = 150.f;
	const int m_AmountOfAgents{ 20 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pSmartAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	Elite::GraphRenderer m_GraphRenderer{};
	Elite::GraphEditor m_GridEditor{};

	/* INFLUENCE MAP VARIABLES */
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid = nullptr;
	bool m_RenderInfluenceMap{ true };
	float m_InfluenceMapCellSize = 5.0f;
	int m_InfluenceMapCols = int(round(m_TrimWorldSize / m_InfluenceMapCellSize));
	int m_InfluenceMapRows = int(round(m_TrimWorldSize / m_InfluenceMapCellSize));

	//const float m_PropagationInterval{ 0.3f };
	//const float m_Momentum{ 0.5f };

	//--Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};
private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime);

	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGame_IM(const App_AgarioGame_IM&) {};
	App_AgarioGame_IM& operator=(const App_AgarioGame_IM&) {};
};

template<class T_AgarioType>
inline void App_AgarioGame_IM::UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
#endif