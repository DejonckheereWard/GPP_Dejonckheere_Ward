#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		std::vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{

		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
			return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		auto nodes = m_pGraph->GetAllNodes();
		int oddCount = 0;
		for (auto node : nodes)
		{
			auto connecteions = m_pGraph->GetNodeConnections(node);
			// Checks if the last bit is 1 or 0 (if it's 1 it's Odd)
			if (connecteions.size() & 1)
				oddCount++;
			
		}


		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
			return Eulerianity::notEulerian;

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if (oddCount == 2 && nodes.size() != 2)
			return Eulerianity::semiEulerian;

		// A connected graph with no odd nodes is Eulerian
		else
			return Eulerianity::eulerian;

		return Eulerianity::notEulerian; // REMOVE AFTER IMPLEMENTING
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		auto path = std::vector<T_NodeType*>();
		int nrOfNodes = graphCopy->GetNrOfNodes();

		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		// Else we need to find a valid starting index for the algorithm
		if (eulerianity == Eulerianity::notEulerian)
			return path;

		// Find valid starting index
		int currentIndex{};
		for (auto node : graphCopy->GetAllNodes())
		{			
			// Find a node with odd amount of connections and use it as the start point
			// If none are found, index will remain 0
			if (graphCopy->GetNodeConnections(node).size() & 1)
			{
				currentIndex = node->GetIndex();
				break;
			}
		}		
		
		
		// Start algorithm loop
		std::stack<int> nodeStack;
		
		// Check if node has neighbors
		while (true)
		{
			auto connections = graphCopy->GetNodeConnections(currentIndex);
			if (connections.size() > 0) // Node has neighbors
			{
				nodeStack.push(currentIndex);
				currentIndex = connections.front()->GetTo();
				graphCopy->RemoveConnection(connections.front());
			}
			else if (nodeStack.size() > 0) // Node doesn't have neighbors
			{
				path.push_back(m_pGraph->GetNode(currentIndex));  // Add the current node (from original) to the path
				//if (nodeStack.size() == 0)
					//break;
				currentIndex = nodeStack.top();  // Take last element in the stack
				nodeStack.pop();  // Remove last element from the stack
				if (nodeStack.size() == 0)
					break;
			}
			else
			{
				break;
			}
		}
		path.push_back(m_pGraph->GetNode(currentIndex));
			


		std::reverse(path.begin(), path.end()); // reverses order of the path
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			// if the node was not visited yet, visit it (get to gets the index of the node it's connected to)
			if(visited[connection->GetTo()] == false)
				VisitAllNodesDFS(connection->GetTo(), visited);			
		}


	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(m_pGraph->GetNrOfNodes(), false);


		if (nodes.size() > 1 && m_pGraph->GetAllConnections().size() == 0)
			return false;

		// find a valid starting node that has connections
		int nodeIndex = invalid_node_index;  // Starts at -1
		for (const auto& n : nodes)
		{
			// Loop over every node & find a node with at least 1 connection
			auto connections = m_pGraph->GetNodeConnections(n);
			if (connections.size() != 0)
			{
				nodeIndex = n->GetIndex();
				break;
			}
			
		}
		
		// if no valid node could be found, return false
		if (nodeIndex == invalid_node_index)
			return false;

		// start a depth-first-search traversal from the node that has at least one connection
		// Second parameter is an output var (results of the function are stored in this)
		VisitAllNodesDFS(nodeIndex, visited);

		// if at least 1 node was never visited, this graph is not connected
		for (auto n : nodes)
		{
			if (visited[n->GetIndex()] == false)
				return false;
		}

		return true;
	}

}