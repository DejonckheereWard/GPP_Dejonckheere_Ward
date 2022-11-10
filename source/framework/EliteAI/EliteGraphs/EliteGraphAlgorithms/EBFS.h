#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{

	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		// Use BFS to find the correct type
		std::queue<T_NodeType*> openList{};
		std::unordered_map<T_NodeType*, T_NodeType*> closedList{}; // Key: current, Value: previous

		openList.push(pStartNode);  // Start with start node (duh)
		

		while (openList.empty() == false)
		{
			T_NodeType* pCurrentNode{ openList.front() };
			openList.pop();

			// Check if we found the correct destination, stop the loop if we did
			if (pCurrentNode == pDestinationNode)
			{
				break;
			}
			
			// Fill all the neighbours to the openList
			for (auto& connection : m_pGraph->GetNodeConnections(pCurrentNode))
			{
				T_NodeType* pNextNode{ m_pGraph->GetNode(connection->GetTo()) };
				
				// Check if the node is already in the closedList / node was already visited
				if (closedList.find(pNextNode) == closedList.end())
				{
					openList.push(pNextNode);					
					closedList[pNextNode] = pCurrentNode;
				}
			}
			
		}

		std::vector<T_NodeType*> path{};
		
		if (closedList.find(pDestinationNode) == closedList.end())
		{
			return path;
		}

		// Backtrack
		// Go from destination to start, then reverse the result
		
		T_NodeType* pCurrentNode{ pDestinationNode };
		while (pCurrentNode != pStartNode)
		{
			// Add current node to the path
			path.push_back(pCurrentNode);

			// Look in the closedList map to see the parent / previous node
			pCurrentNode = closedList[pCurrentNode];
		}

		// Add the start node to the path after finishing the loop
		path.push_back(pStartNode);
		
		// Reverse the path
		reverse(path.begin(), path.end());
		
		return path;
	}
}

