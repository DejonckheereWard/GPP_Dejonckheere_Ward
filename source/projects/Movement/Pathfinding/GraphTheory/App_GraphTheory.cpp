//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2D);
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_pGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>{ false };
	m_pGraph2D->AddNode(new GraphNode2D(0, { 20, 30 }));
	m_pGraph2D->AddNode(new GraphNode2D(1, { -10, -10 }));

	m_pGraph2D->AddConnection(new GraphConnection2D(0, 1));
}

void App_GraphTheory::Update(float deltaTime)
{
	m_GraphEditor.UpdateGraph(m_pGraph2D);
	m_pGraph2D->SetConnectionCostsToDistance();


	auto eulerFinder = EulerianPath<GraphNode2D, GraphConnection2D>(m_pGraph2D);
	Eulerianity eulerianity{};
	eulerianity = eulerFinder.IsEulerian();

	switch (eulerianity)
	{
	case Elite::Eulerianity::notEulerian:
		std::cout << "Not Eulerian" << "\n";
		break;
	case Elite::Eulerianity::semiEulerian:
		std::cout << "Semi-Eulerian" << "\n";
		break;
	case Elite::Eulerianity::eulerian:
		std::cout << "Eulerian" << "\n";
		break;
	default:
		break;
	}
	
	// Reset colors of all node connections
	for (auto node : m_pGraph2D->GetAllNodes())
	{
		node->SetColor({ 0.5f, 0.5f, 0.5f });
		auto& connections = m_pGraph2D->GetNodeConnections(node);
		if (eulerianity == Elite::Eulerianity::notEulerian)
		{
			for (Elite::GraphConnection2D* connection : connections)
			{
				connection->SetColor({ 0,0,0 });
			}
		}
	}


	// Find a euler path
	auto path = eulerFinder.FindPath(eulerianity);
	
	for (size_t i = 0; i < path.size(); i++)
	{
		auto& node = path[i];
		if (i == 0)
		{
			std::cout << node->GetIndex() << " ";
			node->SetColor({0.0f, 1.0f, 0.0f});
		}
		else
		{
			auto& previousNode = path[i - 1];
			const float percentage{ float(path.size() - i) / float(path.size() - 1) };
			
			const auto connection = m_pGraph2D->GetConnection(previousNode->GetIndex(), node->GetIndex());
			connection->SetColor(Elite::Color(1.0f * percentage, 0.0f, 0.0f));
			std::cout << node->GetIndex() << " ";
		}
	}
	
	std::cout << "\n";
	
	
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
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif


}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GraphRenderer.RenderGraph(m_pGraph2D, true, true, true, true);
}
