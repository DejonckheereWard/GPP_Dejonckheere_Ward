#include "stdafx.h"
#include "HierarchicalSpacePartitioning.h"
#include "stdafx.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- BOUNDARY STRUCT ---
// -----------------------
bool Boundary::Contains(const Elite::Vector2& point) const
{
	// Check if point in boundary,
	// Boundary has a center and a halfsize

	// Check if point is in x range
	if(point.x < (Center.x - HalfSize) || point.x >(Center.x + HalfSize))
		return false;

	// Check if point is in y range
	if(point.y < (Center.y - HalfSize) || point.y >(Center.y + HalfSize))
		return false;

	return true;
}

bool Boundary::Intersects(const Boundary& other) const
{
	// Check if other boundary is in this boundary
	// Boundary has a center and a halfsize

	// Check if other boundary is in x range
	if((other.Center.x - other.HalfSize) > (Center.x + HalfSize) || (other.Center.x + other.HalfSize) < (Center.x - HalfSize))
		return false;

	// Check if other boundary is in y range
	if((other.Center.y - other.HalfSize) > (Center.y + HalfSize) || (other.Center.y + other.HalfSize) < (Center.y - HalfSize))
		return false;

	return true;
}

std::vector<Elite::Vector2> Boundary::GetRectPoints() const
{
	float left = Center.x - HalfSize;
	float bottom = Center.y - HalfSize;
	float width = HalfSize * 2;
	float height = HalfSize * 2;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}




// --- Partitioned Space ---
// -------------------------
QuadCellSpace::QuadCellSpace(const Boundary& _boundary, int _bucketSize):
	m_Boundary{ _boundary },
	m_MaxAgentsPerCell{ _bucketSize }
{
	m_Agents.reserve(_bucketSize);

}

bool QuadCellSpace::AddAgent(SteeringAgent* pAgent)
{
	// Check if agent is in boundary or not
	if(!m_Boundary.Contains(pAgent->GetPosition()))
		return false; // Agent not in boundary

	// Check if cell is subdivided or not, and if it still has space
	if(m_LeftTop == nullptr && int(m_Agents.size()) < m_MaxAgentsPerCell)
	{
		m_Agents.push_back(pAgent);
		return true;
	}

	// Subdivide if not yet subdivided
	if(m_LeftTop == nullptr)
		Subdivide();

	// We dont want to add agents to a subdivided cell (since its children need to hold the data instead)
	// So we check if the agent is in the children cells
	if(m_LeftTop->AddAgent(pAgent))
		return true;
	if(m_RightTop->AddAgent(pAgent))
		return true;
	if(m_LeftBottom->AddAgent(pAgent))
		return true;
	if(m_RightBottom->AddAgent(pAgent))
		return true;

	// Shouldn't ever happen
	std::cout << "FAILED TO INSERT INTO THE QUADTREE" << std::endl;
	return false;
}

void QuadCellSpace::Subdivide()
{
	// Subdivides the current tree
	// Needs to add its current cells to the children

	// Calculate the new halfsize
	float newHalfSize = m_Boundary.HalfSize / 2.f;

	// Calculate the centers
	Elite::Vector2 leftTopCenter{ m_Boundary.Center.x - newHalfSize, m_Boundary.Center.y + newHalfSize };
	Elite::Vector2 rightTopCenter{ m_Boundary.Center.x + newHalfSize, m_Boundary.Center.y + newHalfSize };
	Elite::Vector2 leftBottomCenter{ m_Boundary.Center.x - newHalfSize, m_Boundary.Center.y - newHalfSize };
	Elite::Vector2 rightBottomCenter{ m_Boundary.Center.x + newHalfSize, m_Boundary.Center.y - newHalfSize };

	// Create the new boundaries
	Boundary leftTopBoundary{ leftTopCenter, newHalfSize };
	Boundary rightTopBoundary{ rightTopCenter, newHalfSize };
	Boundary leftBottomBoundary{ leftBottomCenter, newHalfSize };
	Boundary rightBottomBoundary{ rightBottomCenter, newHalfSize };

	// Create the new children
	m_LeftTop = new QuadCellSpace(leftTopBoundary, m_MaxAgentsPerCell);
	m_RightTop = new QuadCellSpace(rightTopBoundary, m_MaxAgentsPerCell);
	m_LeftBottom = new QuadCellSpace(leftBottomBoundary, m_MaxAgentsPerCell);
	m_RightBottom = new QuadCellSpace(rightBottomBoundary, m_MaxAgentsPerCell);

	// Add the current agents to the children
	for(SteeringAgent* pAgent : m_Agents)
	{
		AddAgent(pAgent); // Should work since it will now find the children
	}

	// Clear the current agents
	m_Agents.clear();
}

void QuadCellSpace::Clear()
{
	// Clear the current agents
	m_Agents.clear();

	// Clear the children
	if(m_LeftTop != nullptr)
	{
		m_LeftTop->Clear();
		m_RightTop->Clear();
		m_LeftBottom->Clear();
		m_RightBottom->Clear();

		delete m_LeftTop;
		delete m_RightTop;
		delete m_LeftBottom;
		delete m_RightBottom;

		m_LeftTop = nullptr;
		m_RightTop = nullptr;
		m_LeftBottom = nullptr;
		m_RightBottom = nullptr;
	}
}

void QuadCellSpace::Render()
{
	// If subdivided, render the children
	if(m_LeftTop != nullptr)
	{
		m_LeftTop->Render();
		m_RightTop->Render();
		m_LeftBottom->Render();
		m_RightBottom->Render();
	}
	else
	{
		// Render own boundary
		const Elite::Color gridColor{ 0.67f, 0.67f, 0.0f, 0.8f };
		std::vector<Elite::Vector2> rectPoints = m_Boundary.GetRectPoints();

		//// Create Polygon object with the rectPoints
		DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), gridColor, DEBUGRENDERER2D->NextDepthSlice());
		//Elite::Vector2 leftTop = rectPoints[1];
		//leftTop.x += 1.0f;
		//leftTop.y -= 1.0f;

		//if(m_DrawCellAgentCount)

		//	DEBUGRENDERER2D->DrawString(leftTop, std::to_string(c->agents.size()).c_str());
	}
}

void QuadCellSpace::RenderActiveCells(const Boundary& boundary)
{
	// Colors all cells in this boundary green
	// If subdivided, render the children
	if(m_LeftTop != nullptr)
	{
		m_LeftTop->RenderActiveCells(boundary);
		m_RightTop->RenderActiveCells(boundary);
		m_LeftBottom->RenderActiveCells(boundary);
		m_RightBottom->RenderActiveCells(boundary);
	}
	else
	{
		// Check if this cell is inside the given boundary
		if(boundary.Intersects(m_Boundary))
		{
			// Render own boundary
			const Elite::Color activeColor{ 0.0f, 1.0f, 0.0f, 0.8f };
			std::vector<Elite::Vector2> rectPoints = m_Boundary.GetRectPoints();

			// Create Polygon object with the rectPoints
			DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), activeColor, -0.1f);
		}

	}
}

std::vector<SteeringAgent*> QuadCellSpace::QueryRange(const Boundary& queryRange) const
{
	// Find all the agents within the given range

	// Create vector to store the result
	std::vector<SteeringAgent*> results{};

	// Check if query range intersects with this cell
	if(!m_Boundary.Intersects(queryRange))
		return results;  // Empty

	// Check if the tree is subdivided
	if(m_LeftTop == nullptr)
	{
		// Check if the agents in this cell are in the query range
		for(SteeringAgent* pAgent : m_Agents)
		{
			if(queryRange.Contains(pAgent->GetPosition()))
				results.push_back(pAgent);
		}
		return results;
	}

	// Cell is subdivided, so it wont contain any cells itself, so we need to check its children
	const std::vector<SteeringAgent*>& leftTopResults{ m_LeftTop->QueryRange(queryRange) };
	const std::vector<SteeringAgent*>& rightTopResults{ m_RightTop->QueryRange(queryRange) };
	const std::vector<SteeringAgent*>& leftBottomResults{ m_LeftBottom->QueryRange(queryRange) };
	const std::vector<SteeringAgent*>& rightBottomResults{ m_RightBottom->QueryRange(queryRange) };

	// Add all the results to the results vector
	results.insert(results.end(), leftTopResults.begin(), leftTopResults.end());
	results.insert(results.end(), rightTopResults.begin(), rightTopResults.end());
	results.insert(results.end(), leftBottomResults.begin(), leftBottomResults.end());
	results.insert(results.end(), rightBottomResults.begin(), rightBottomResults.end());


	return results;
}

QuadCellSpace::~QuadCellSpace()
{
	// Clear the agents
	m_Agents.clear();

	// Delete all the children
	if(m_LeftTop != nullptr)
	{
		delete m_LeftTop;
		delete m_RightTop;
		delete m_LeftBottom;
		delete m_RightBottom;
	}
}
