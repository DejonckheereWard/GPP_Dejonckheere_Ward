#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

Elite::Vector2 Cell::GetRectCenter() const
{
	float x = boundingBox.bottomLeft.x;
	float y = boundingBox.bottomLeft.y;
	float width = boundingBox.width;
	float height = boundingBox.height;

	return Elite::Vector2(x + width / 2.0f, y + height / 2.0f);
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities):
	m_SpaceWidth{ width },
	m_SpaceHeight{ height },
	m_NrOfRows{ rows },
	m_NrOfCols{ cols },
	m_Neighbors{ maxEntities },
	m_NrOfNeighbors{ 0 },
	m_NeighborCells{ rows * cols },
	m_NrOfNeighborCells{ 0 },
	m_CellWidth{ width / cols },
	m_CellHeight{ height / rows },
	m_DrawCellAgentCount{ false }
{
	for(int row{}; row < rows; ++row)
	{
		for(int col{}; col < cols; ++col)
		{
			float left = col * m_CellWidth;
			float bottom = row * m_CellHeight;

			m_Cells.push_back(new Cell(left, bottom, m_CellWidth, m_CellHeight));
		}
	}

}

CellSpace::~CellSpace()
{
	for(Cell* c : m_Cells)
	{
		delete c;
	}
	m_Cells.clear();
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	// Get the correct cell
	const int cellIndex{ PositionToIndex(agent->GetPosition()) };

	// Add the agent to the found cell
	m_Cells[cellIndex]->agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent)
{
	// Get index of old pos & new pos
	const int newCellIndex{ PositionToIndex(agent->GetPosition()) };
	const int previousCellIndex{ PositionToIndex(agent->GetOldPosition()) };

	// If the agent has moved to another cell, remove it from the old & add it to the new
	if(newCellIndex != previousCellIndex)
	{
		m_Cells[previousCellIndex]->agents.remove(agent);
		m_Cells[newCellIndex]->agents.push_back(agent);
	}


}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	// Find out what cells are within the bounding box of the agent
	// Find out which agents are in these cells & add them to the m_Neighbors vector

	const Elite::Vector2 agentPos{ agent->GetPosition() };
	const Elite::Vector2 boundingBoxLeftBottom{ agentPos.x - queryRadius, agentPos.y - queryRadius };
	const Elite::Vector2 boundingBoxRightTop{ agentPos.x + queryRadius, agentPos.y + queryRadius };

	const Elite::Vector2 startIndex{ PositionToRowColumn(boundingBoxLeftBottom) };
	const Elite::Vector2 endIndex{ PositionToRowColumn(boundingBoxRightTop) };


	// Loop over every row & column, add all the agents to a list of agents
	m_NrOfNeighbors = 0;  // Reset the value to 0	
	m_NrOfNeighborCells = 0;  // Reset amount of cells

	for(int row{ int(startIndex.y) }; row <= int(endIndex.y); ++row)
	{
		for(int column{ int(startIndex.x) }; column <= int(endIndex.x); ++column)
		{
			// Get the index of the cell
			const int cellIndex{ (row * m_NrOfCols + column) % (m_NrOfCols * m_NrOfRows) };
			for(SteeringAgent* cellAgent : m_Cells[cellIndex]->agents)
			{
				m_Neighbors[m_NrOfNeighbors++] = cellAgent;
			}
			// Keep track of the active cells for debug purposes
			m_NeighborCells[m_NrOfNeighborCells++] = m_Cells[cellIndex];
		}
	}

}

void CellSpace::EmptyCells()
{
	for(Cell* c : m_Cells)
	{
		c->agents.clear();
	}
}

void CellSpace::RenderCells() const
{
	const Elite::Color gridColor{ 0.67f, 0.67f, 0.0f, 0.8f };
	for(Cell* c : m_Cells)
	{
		std::vector<Elite::Vector2> rectPoints = c->GetRectPoints();
		// Create Polygon object with the rectPoints
		DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), gridColor, 0.8f);
		Elite::Vector2 leftTop = rectPoints[1];
		leftTop.x += 1.0f;
		leftTop.y -= 1.0f;

		if(m_DrawCellAgentCount)

			DEBUGRENDERER2D->DrawString(leftTop, std::to_string(c->agents.size()).c_str());
	}
}

void CellSpace::RenderActiveCells() const
{
	const Elite::Color activeColor{ 0.0f, 1.0f, 0.0f, 0.8f };
	for(int i{}; i < m_NrOfNeighborCells; ++i)
	{
		std::vector<Elite::Vector2> rectPoints = m_NeighborCells[i]->GetRectPoints();
		DEBUGRENDERER2D->DrawPolygon(rectPoints.data(), rectPoints.size(), activeColor, -0.1f);
	}
}



int CellSpace::PositionToIndex(const Elite::Vector2& pos) const
{
	// Returns the index of the cell the given position is in.
	Elite::Vector2 gridIndex{ PositionToRowColumn(pos) };
	return int(gridIndex.y) * m_NrOfCols + int(gridIndex.x);
}

Elite::Vector2 CellSpace::PositionToRowColumn(const Elite::Vector2& pos) const
{
	// Clamping the value between 0 and the max value, to prevent out of bounds errors
	const int column{ Elite::Clamp(int(pos.x / m_CellWidth), 0, m_NrOfCols - 1) };
	const int row{ Elite::Clamp(int(pos.y / m_CellHeight), 0, m_NrOfRows - 1) };

	// Debug checks
	assert(column >= 0 && column < m_NrOfCols);
	assert(row >= 0 && column < m_NrOfRows);

	return Elite::Vector2(float(column), float(row));
}