#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteGraphs/ENavGraph.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			Polygon* navMeshPolygon = pNavGraph->GetNavMeshPolygon();
			const Triangle* startTriangle = navMeshPolygon->GetTriangleFromPosition(startPos);
			const Triangle* endTriangle = navMeshPolygon->GetTriangleFromPosition(endPos);
			
			// Check if neither are nullptrs (return if one is)
			if (!startTriangle || !endTriangle)
			{
				return finalPath;
			}

			// Make sure they aren't the same
			// If they are the same, add the endpos to the finalpath & return
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}
			
			// If both nodes are valid and aren't the same, we can continue with finding the path.
				
		
			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto pNavGraphClone = pNavGraph->Clone();
			
			//Create extra node for the Start Node (Agent's position)
			int startNodeIdx = pNavGraphClone->AddNode(new NavGraphNode(pNavGraphClone->GetNextFreeNodeIndex(), -1, startPos));
			
			// Create connections to every valid node of the triangle the startpos is in
			const auto& startTriangleLines = startTriangle->metaData.IndexLines;
			for (const auto& lineIdx : startTriangleLines)
			{
				// Get the node using the line idx & check if it's valid
				int nodeIdx = pNavGraph->GetNodeIdxFromLineIdx(lineIdx);
				if (nodeIdx != invalid_node_index)
				{
					// if it's valid, make a connection between the startpos node & the node
					const float connectionCost = Distance(pNavGraphClone->GetNodePos(nodeIdx), startPos);
					pNavGraphClone->AddConnection(new GraphConnection2D(startNodeIdx, nodeIdx, connectionCost));
				}
			}
			
			//Create extra node for the endNode
			int endNodeIdx = pNavGraphClone->AddNode(new NavGraphNode(pNavGraphClone->GetNextFreeNodeIndex(), -1, endPos));

			// Create connections to every valid node of the triangle the startpos is in
			const auto& endTriangleLines = endTriangle->metaData.IndexLines;
			for (const auto& lineIdx : endTriangleLines)
			{
				// Get the node using the line idx & check if it's valid
				int nodeIdx = pNavGraph->GetNodeIdxFromLineIdx(lineIdx);
				if (nodeIdx != invalid_node_index)
				{
					// if it's valid, make a connection between the startpos node & the node
					const float connectionCost = Distance(pNavGraphClone->GetNodePos(nodeIdx), endPos);
					pNavGraphClone->AddConnection(new GraphConnection2D(endNodeIdx, nodeIdx, connectionCost));
				}
			}


			//Run A star on new graph
			auto pathFinder = AStar<NavGraphNode, GraphConnection2D>(pNavGraphClone.get(), Elite::HeuristicFunctions::Euclidean);
			std::vector<NavGraphNode*> path = pathFinder.FindPath(pNavGraphClone->GetNode(startNodeIdx), pNavGraphClone->GetNode(endNodeIdx));

			debugNodePositions.clear();
			for (const NavGraphNode* node : path)
			{
				debugNodePositions.push_back(node->GetPosition());
			}
			
			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(path, navMeshPolygon);
			finalPath = SSFA::OptimizePortals(portals);
			debugPortals = portals;

			return finalPath;
		}
	};
}
