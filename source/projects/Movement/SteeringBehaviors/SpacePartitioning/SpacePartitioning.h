/*=============================================================================*/
// Copyright 2019-2020
// Authors: Yosha Vandaele
/*=============================================================================*/
// SpacePartitioning.h: Contains Cell and Cellspace which are used to partition a space in segments.
// Cells contain pointers to all the agents within.
// These are used to avoid unnecessary distance comparisons to agents that are far away.

// Heavily based on chapter 3 of "Programming Game AI by Example" - Mat Buckland
/*=============================================================================*/

#pragma once
#include <list>
#include <vector>
#include <iterator>
#include "framework\EliteMath\EVector2.h"
#include "framework\EliteGeometry\EGeometry2DTypes.h"

class SteeringAgent;

// --- Cell ---
// ------------
struct Cell
{
	Cell(float left, float bottom, float width, float height);

	std::vector<Elite::Vector2> GetRectPoints() const;
	Elite::Vector2 GetRectCenter() const;

	// all the agents currently in this cell
	std::list<SteeringAgent*> agents;  // Linked list for more performant inserts & deletes
	Elite::Rect boundingBox;
};

// --- Partitioned Space ---
// -------------------------
class CellSpace
{
public:
	CellSpace(float width, float height, int rows, int cols, int maxEntities);
	~CellSpace();

	void AddAgent(SteeringAgent* agent);
	void UpdateAgentCell(SteeringAgent* agent);

	void RegisterNeighbors(SteeringAgent* agent, float queryRadius);
	const std::vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }

	//empties the cells of entities
	void EmptyCells();

	void RenderCells() const;
	void RenderActiveCells() const;

	void SetDrawCellAgentCount(bool value) { m_DrawCellAgentCount = value; }
	bool GetDrawCellAgentCount() const { return m_DrawCellAgentCount; }

private:
	// Cells and properties
	std::vector<Cell*> m_Cells;

	float m_SpaceWidth;
	float m_SpaceHeight;

	int m_NrOfRows;
	int m_NrOfCols;

	float m_CellWidth;
	float m_CellHeight;

	bool m_DrawCellAgentCount;
	
	// Members to avoid memory allocation on every frame
	std::vector<SteeringAgent*> m_Neighbors;
	int m_NrOfNeighbors;

	std::vector<Cell*> m_NeighborCells;
	int m_NrOfNeighborCells;

	// Helper functions
	int PositionToIndex(const Elite::Vector2& pos) const;
	Elite::Vector2 PositionToRowColumn(const Elite::Vector2& pos) const;
};
