/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __PHYSICS_H
#define __PHYSICS_H

class SceneNode;
class CollisionResponseSceneNode;
class LevelSceneNode;
class aabb2;
class GfxMgr;
struct Edge;
struct Primitive;

enum COLLISION_TYPE {
	COLLISION_TYPE_POLYPOLY = 0,
	COLLISION_TYPE_POLYCIRCLE = 1,
	COLLISION_TYPE_CIRCLECIRCLE = 2
};

class CollisionEvent {
	public:
	COLLISION_TYPE type;
	bool doIntersect; // are the two shapes currently intersecting
	bool willIntersect; // will the two shapes intersect in the next timestep
	CollisionResponseSceneNode* node_A; // collision node A
	CollisionResponseSceneNode* node_B; // collision node B
	Edge* edge; // edge of collision
	vector2 edge_vertexA_cpy; // copy of edge vertex A 
	// (in case you're colliding with the level which generates its geometry on the fly and doesn't always have a copy)
	vector2 edge_vertexB_cpy; // copy of edge vertex B
	bool vertexOrEdge;
	vector2* vertex; // vertex of collision
	vector2 normal; // collision normal
	float distance; // distance along normal
};

class PhysMgr {
	GfxMgr* gfx;
	std::list<CollisionResponseSceneNode*>* world;
	LevelSceneNode* level;
	bool isRunning;
	
	bool doFastCircleCircleTest(Primitive* x, Primitive* y);
	CollisionEvent doCircleCircleTest(Primitive* x, Primitive* y);	
	CollisionEvent doPolygonPolygonTest(Primitive* y, Primitive* x);
	CollisionEvent doPolygonCircleTest(Primitive* y, Primitive* x);
	
	bool doPointInsidePolyTest(Primitive* x, vector2 point, bool* error=NULL);
	vector2 projectPrimitive(Primitive* x, vector2 axis); // returns interval of projection in a vector2(interval_min, interval_max)
	vector2 projectEdge(Edge* edge, vector2 axis);
	float projectVector(vector2 v, vector2 axis);
	float findIntervalDistance(vector2 interval_x, vector2 interval_y); // returns signed distance between two intervals
	float findIntervalDistance(vector2 interval_x, float interval_y); 
	float findPointEdgeDistance(Edge* edge, vector2 point);
	bool doRayCircleTest(Primitive* x, ray* y);

	void updateGridPosition(Primitive* x);
	void updateForces(Primitive* x, vector2 netForce);
	void updateVerlet(float timeStep, Primitive* x);
	void updateCenter(Primitive* x);
	void solveConstraints(Primitive *x);
	void resolveCollisions(CollisionResponseSceneNode* node);
	aabb2 getBoundingBox(Primitive *x);	
	void processPolyLevelCollision(CollisionEvent event);
	void processCircleLevelCollision(CollisionEvent event);
	void processPolyPolyCollision(CollisionEvent event);
	void processPolyCircleCollision(CollisionEvent event);
	void processCircleCircleCollision(CollisionEvent event);
	void processLevelCollision(CollisionEvent event);
	void processCollision(CollisionEvent event);
	bool checkMinVel(Primitive* x);
	
	float timeStep;
	float timeAccumulator;
	int iterations_num;
	
	public:
	PhysMgr(GfxMgr* gfx_n);
	void init(float timeStep);
	void startSimulation();
	void stopSimulation();
	void reinit();
	void update(float frameDelta);
	void registerLevelSceneNode(LevelSceneNode* level_n);
	void registerSceneNode(CollisionResponseSceneNode* newNode);
	void unregisterSceneNode(CollisionResponseSceneNode* node);
	CollisionEvent primitiveCollisionTest(Primitive* prim, bool doLevelTest=true, bool doObjectsTest=true, CollisionResponseSceneNode* caller=NULL, GAME_OBJECT_TYPE type=GO_TYPE_ALL);
	bool rayVisibilityTest(Primitive* circle, ray* x, CollisionResponseSceneNode* caller);
	void setQuadEdgesAndConstraints(Primitive* quad);
	void setIterations(int iterations);
	void calculatePolyCenter(Primitive *x);
};

#endif
