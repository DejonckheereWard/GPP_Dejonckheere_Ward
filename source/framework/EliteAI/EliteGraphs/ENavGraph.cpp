#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	// Clear existing graph before creating new one
	Clear();
	
	//1. Go over all the edges of the navigationmesh and create nodes	
	const auto& navMeshLines = m_pNavMeshPolygon->GetLines();
	int nodeIdx = 0;
	for (const auto& line : navMeshLines)
	{
		const auto connections = m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index);
		if (connections.size() > 1)  // Bigger than one to include itself
		{
			// Create a node in the middle of the line
			auto node = new NavGraphNode(nodeIdx++, line->index, (line->p1 + line->p2) / 2.0f);
			AddNode(node);			
		}
	}


	//2. Create connections now that every node is created
	const auto& navMeshTriangles = m_pNavMeshPolygon->GetTriangles();
	for (const auto& triangle : navMeshTriangles)
	{
		const auto& triangleLines = triangle->metaData.IndexLines;
		std::vector<int> currentNodeIdxs;
		for (const int lineIdx : triangleLines)
		{			
			// Get the node using the line idx & check if it's valid
			int nodeIdx = GetNodeIdxFromLineIdx(lineIdx);
			if (nodeIdx != invalid_node_index)
			{
				// if it's valid, add to the list of valid noes
				currentNodeIdxs.push_back(nodeIdx);
			}
		}
		// Create connections between nodes
		if (currentNodeIdxs.size() == 2)
		{
			// Only one connection needs to be made (2 lines)
			AddConnection(new GraphConnection2D(currentNodeIdxs[0], currentNodeIdxs[1]));
		}
		else if (currentNodeIdxs.size() == 3)
		{
			// 3 connections need to be made (3 lines)
			AddConnection(new GraphConnection2D(currentNodeIdxs[0], currentNodeIdxs[1]));
			AddConnection(new GraphConnection2D(currentNodeIdxs[1], currentNodeIdxs[2]));
			AddConnection(new GraphConnection2D(currentNodeIdxs[2], currentNodeIdxs[0]));
		}

	}
	
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

