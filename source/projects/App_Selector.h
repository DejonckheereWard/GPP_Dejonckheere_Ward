#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

//#define ActiveApp_Sandbox
//#define ActiveApp_Steering
//#define ActiveApp_CombinedSteering
#define ActiveApp_Flocking
//#define ActiveApp_GraphTheory
//#define ActiveApp_ASTAR
//#define ActiveApp_NavMeshGraph
//#define ActiveApp_FSM
//#define ActiveApp_BT
//#define ActiveApp_IM
//#define ActiveApp_AgarioIM


//---------- Registered Applications -----------
#ifdef ActiveApp_Sandbox
#include "projects/Movement/Sandbox/App_Sandbox.h"
typedef App_Sandbox CurrentApp;
#endif // Sandbox


#ifdef ActiveApp_Steering
#include "projects/Movement/SteeringBehaviors/Steering/App_SteeringBehaviors.h"
typedef App_SteeringBehaviors CurrentApp;
#endif  // Steering


#ifdef ActiveApp_CombinedSteering
#include "projects/Movement/SteeringBehaviors/CombinedSteering/App_CombinedSteering.h"
typedef App_CombinedSteering CurrentApp;
#endif  // CombinedSteering

#ifdef ActiveApp_Flocking
#include "projects/Movement/SteeringBehaviors/Flocking/App_Flocking.h"
typedef App_Flocking CurrentApp;
#endif  // Flocking

#ifdef ActiveApp_GraphTheory
#include "projects/Movement/Pathfinding/GraphTheory/App_GraphTheory.h"
typedef App_GraphTheory CurrentApp;
#endif  // GraphTheory

#ifdef ActiveApp_ASTAR
#include "projects/Movement/Pathfinding/AStar/App_PathfindingAStar/App_PathfindingAStar.h"
typedef App_PathfindingAStar CurrentApp;
#endif  // GraphTheory

#ifdef ActiveApp_NavMeshGraph
#include "projects/Movement/Pathfinding/NavMeshGraph/App_NavMeshGraph.h"
typedef App_NavMeshGraph CurrentApp;
#endif  // GraphTheory

#ifdef ActiveApp_FSM
#include "projects/DecisionMaking/FiniteStateMachines/App_AgarioGame.h"
typedef App_AgarioGame CurrentApp;
#endif  // FiniteStateMachine

#ifdef ActiveApp_BT
#include "projects/DecisionMaking/BehaviorTrees/App_AgarioGame_BT.h"
typedef App_AgarioGame_BT CurrentApp;
#endif

#ifdef ActiveApp_IM
#include "projects/DecisionMaking/InfluenceMaps/App_InfluenceMap.h"
typedef App_InfluenceMap CurrentApp;
#endif

#ifdef ActiveApp_AgarioIM
#include "projects/DecisionMaking/InfluenceMaps/App_AgarioGame_IM.h"
typedef App_AgarioGame_IM CurrentApp;
#endif


class App_Selector {
public:
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};