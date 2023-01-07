#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path{};
		std::vector<NodeRecord> openList{};
		std::vector<NodeRecord> closedList{};

		// Create noderecord
		NodeRecord currentRecord{};
		currentRecord.pNode = pStartNode;
		currentRecord.pConnection = nullptr;
		currentRecord.costSoFar = 0.f;
		currentRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		// Add start node to open list to kickstart the loop
		openList.push_back(currentRecord);

		while(!openList.empty())
		{
			// NodeRecord's are sorted by their estimatedTotalCost -> Heuristic Cost -> F-COST
			// Get the noderecord with the lowest cost
			currentRecord = *std::min_element(openList.begin(), openList.end());

			// Check if we found the goal node
			if(currentRecord.pNode == pGoalNode)
			{
				closedList.push_back(currentRecord);
				break;
			}

			// Loop over all connections of the current node & loop over them
			for(const auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				// For each connection, calculate the total cost SO FAR (not estimated) -> G-COST
				const auto connectionNode{ m_pGraph->GetNode(connection->GetTo()) };

				float costSoFar{ currentRecord.costSoFar + connection->GetCost() };  // G-Cost
				float estimatedTotalCost{ costSoFar + GetHeuristicCost(connectionNode, pGoalNode) };  // F-Cost

				// Check the closed list if the connectionNode is already in there
				// IF it is in there, check if it's cost so far is lower than the current record's cost so far
				// --> IF it is lower, skip this connection, if not, delete it from the list & add the new one to the open list
				// IF it isn't found in the closed list, we do the same check for the openList
				// if it isn't found in either, it means we don't have an existing connection to this node, so we add it to the open list.

				// Check if the connection is already in the closed list
				auto findNodeLambda = [connectionNode](NodeRecord rn) { return rn.pNode == connectionNode; };
				auto closedNode = std::find_if(closedList.begin(), closedList.end(), findNodeLambda);

				if(closedNode != closedList.end())
				{
					NodeRecord record = *closedNode;
					// Check if the existing connection has a higher cost than the new connection's cost
					if(record.costSoFar > costSoFar)
					{
						// If it is higher, erase it from the closedList
						closedList.erase(std::remove(closedList.begin(), closedList.end(), record));
					}
					continue;
				}


				// Check if the connection is already in the open list
				auto openNode = std::find_if(openList.begin(), openList.end(), findNodeLambda);
				if(openNode != openList.end())
				{
					NodeRecord record = *openNode;
					if(record.costSoFar > costSoFar)
					{
						// If it is higher, erase it from the closedList
						openList.erase(std::remove(openList.begin(), openList.end(), record));
					}
					continue;
				}

				NodeRecord newRecord{};
				newRecord.pNode = connectionNode;
				newRecord.pConnection = connection;
				newRecord.costSoFar = costSoFar;
				newRecord.estimatedTotalCost = estimatedTotalCost;

				openList.push_back(newRecord);
			}

			// Add current to closed list & remove from the open list
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);

		}

		if(currentRecord.pNode != pGoalNode)
		{
			// No path found
			return path;
		}

		// Create the path from the current record all the way back to the start
		while(currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);

			auto previousNode = m_pGraph->GetNode(currentRecord.pConnection->GetFrom());

			auto findNodeLambda = [previousNode](NodeRecord rn) { return rn.pNode == previousNode; };
			auto closedNode = std::find_if(closedList.begin(), closedList.end(), findNodeLambda);
			if(closedNode != closedList.end())
			{
				currentRecord = *closedNode;
			}
		}

		path.push_back(pStartNode);
		reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}