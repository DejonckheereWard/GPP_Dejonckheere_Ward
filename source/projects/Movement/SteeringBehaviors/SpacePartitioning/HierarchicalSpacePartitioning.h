#pragma once

/*=============================================================================*/
// Copyright 2023
// Authors: Dejonckheere Ward
/*=============================================================================*/
// HierarchicalSpacePartitioning.h: Contains Cell and Cellspace which are used to partition a space in segments.
// Cells contain pointers to all the agents within.
// These are used to avoid unnecessary distance comparisons to agents that are far away.
// Extended / edited version from spacepartitioning to make it Hierarchical using quadtrees

/*=============================================================================*/

#pragma once
#include <list>
#include <vector>
#include <iterator>
#include "framework\EliteMath\EVector2.h"
#include "framework\EliteGeometry\EGeometry2DTypes.h"

class SteeringAgent;


struct Boundary
{

	Boundary(Elite::Vector2 _center, float _halfSize): Center(_center), HalfSize(_halfSize) {};

	bool Contains(const Elite::Vector2& point) const;
	bool Intersects(const Boundary& other) const;
	std::vector<Elite::Vector2> GetRectPoints() const;


	Elite::Vector2 Center;
	float HalfSize;  // Apothem
};


// --- Partitioned Space ---
// -------------------------
class QuadCellSpace
{
public:
	QuadCellSpace(const Boundary& _boundary, int _bucketSize);

	~QuadCellSpace();
	QuadCellSpace(const QuadCellSpace& other) = delete;
	QuadCellSpace(QuadCellSpace&& other) = delete;
	QuadCellSpace& operator=(const QuadCellSpace& other) = delete;
	QuadCellSpace& operator=(QuadCellSpace&& other) = delete;

	// Need add agent function, subdivide function and a get agents in radius
	bool AddAgent(SteeringAgent* pAgent);  // Takes current position and adds it to the cell
	void Subdivide();  // Subdivides the cell into 4 cells
	std::vector<SteeringAgent*> QueryRange(const Boundary& queryRange) const;  // Returns all agents found inside the range
	void Clear(); // Deletes children and clears list
	void Render();
	void RenderActiveCells(const Boundary& boundary);

private:
	const Boundary m_Boundary;
	const int m_MaxAgentsPerCell;

	std::vector<SteeringAgent*> m_Agents;  // Agents in this node

	// Children
	QuadCellSpace* m_LeftTop;
	QuadCellSpace* m_RightTop;
	QuadCellSpace* m_LeftBottom;
	QuadCellSpace* m_RightBottom;



};
