/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

/* !
various parts of physics and intersection algorithms originally published by:
	pogopixel.com
	gamedev.net
*/

using namespace std;

PhysMgr::PhysMgr(GfxMgr* gfx_n) {
	gfx = gfx_n;
	isRunning = false;
	world = new list<CollisionResponseSceneNode*>;
	iterations_num = DEFAULT_PHYS_ITERATIONS;
}

bool PhysMgr::doFastCircleCircleTest(Primitive* x, Primitive* y) {
	bool doCollide = true;
	if((x->center - y->center).length2() > (x->radius+y->radius)*(x->radius+y->radius)) {
		doCollide = false;
	}
	return doCollide;	
}

CollisionEvent PhysMgr::doCircleCircleTest(Primitive* x, Primitive* y) {
	CollisionEvent newEvent;
	newEvent.doIntersect = true;
	newEvent.type = COLLISION_TYPE_CIRCLECIRCLE;
	
	vector2 axis = y->center - x->center;
	axis.normalize();
	float minDistance;
	minDistance = x->radius + y->radius - (y->center - x->center).length();
	newEvent.distance = fabs(minDistance);
	newEvent.normal = axis;
	
	return newEvent;
}

bool PhysMgr::doPointInsidePolyTest(Primitive* x, vector2 point, bool* error) {
	if(error) *error = false;
	float result;
	int sign = 0;
	int result_i;
	int numVerticesX = x->type;
	if(numVerticesX	< 3) {
		if(error) *error = true;
		return false;
	}
	
	for(int i=0; i<numVerticesX; i++) {
		Edge* edge = &(x->edges[i]);
		vector2 test_point = point - *(edge->vertexA);
		result = (*(edge->vertexA) - *(edge->vertexB)).cross_product(test_point);
		result_i = int(result / abs(result));
		if(sign == 0) sign = result_i;
		else if(result_i != sign) return false;
	}
	return true;
}

CollisionEvent PhysMgr::doPolygonPolygonTest(Primitive* y, Primitive* x) {
	CollisionEvent newEvent;
	newEvent.type = COLLISION_TYPE_POLYPOLY;
	newEvent.doIntersect = true;
	newEvent.normal = vector2();
	newEvent.distance = P_INF;
	vector2 axis;
	Edge* edge;
	int numEdgesX = x->type;
	int numEdgesY = y->type;
	float minIntervalDistance = P_INF;
	
	for(int p=0; p < numEdgesX+numEdgesY; p++) {
		if(p < numEdgesX) edge = &x->edges[p];
		else edge = &y->edges[p-numEdgesX];
		
		axis.set((*edge->vertexA)[1] - (*edge->vertexB)[1], (*edge->vertexB)[0] - (*edge->vertexA)[0]);
		axis.normalize();
		
		vector2 interval_A = projectPrimitive(x, axis);
		vector2 interval_B = projectPrimitive(y, axis);
		float intervalDistance = findIntervalDistance(interval_A, interval_B);
		
		if(intervalDistance > 0) {
			newEvent.doIntersect = false; // the polygons are not intersecting
			return newEvent;
		}
		// the polygons are intersecting
		intervalDistance = fabs(intervalDistance);
		if(intervalDistance < minIntervalDistance) {
			minIntervalDistance = intervalDistance;
			newEvent.edge = edge;
			newEvent.edge_vertexA_cpy = *(edge->vertexA);
			newEvent.edge_vertexB_cpy = *(edge->vertexB);
			newEvent.normal = axis;	
		}	
	}
	newEvent.distance = minIntervalDistance;
	if(newEvent.edge->parent != y) {
		Primitive* tmp = y;
		y = x;
		x = tmp;	
	}
	
	calculatePolyCenter(x);
	calculatePolyCenter(y);
	vector2 tmp = x->center - y->center;
        if (tmp.dot_product(newEvent.normal) <= 0)
        	newEvent.normal *= -1.f;
	
	float min_d = P_INF;
	for(int n=0; n<numEdgesX; n++) {
		vector2 tmp = x->vertices[n] - y->center;
		float dist = newEvent.normal.dot_product(tmp);
		if(dist < min_d) {
			min_d = dist;
			newEvent.vertex = &(x->vertices[n]);	
		}
	}
	
	return newEvent;
}

CollisionEvent PhysMgr::doPolygonCircleTest(Primitive* y, Primitive* x) {
	CollisionEvent newEvent;
	newEvent.type = COLLISION_TYPE_POLYCIRCLE;
	newEvent.doIntersect = true;
	newEvent.vertexOrEdge = 0;
	newEvent.normal = vector2();
	
	int numVerticesX = x->type;
	float minIntervalDistance = P_INF;
	
	for(int p=0; p<numVerticesX; p++) {
		vector2* vertex = &(x->vertices[p]);
		vector2 axis = *vertex;
		axis.normalize();
		
		vector2 circleInterval = projectPrimitive(y, axis);
		vector2 polyInterval = projectPrimitive(x, axis);
		float intervalDistance = findIntervalDistance(circleInterval, polyInterval);
		
		if(intervalDistance > 0) {
			newEvent.doIntersect = false;
			return newEvent;
		}
		
		intervalDistance = fabs(intervalDistance);
		if(intervalDistance < minIntervalDistance) {
			minIntervalDistance = intervalDistance;
			newEvent.normal = axis;
			newEvent.edge = &(x->edges[p]);
			newEvent.vertexOrEdge = 1;
		}
	}
	
	
	for(int p=0, j=numVerticesX-1; p<numVerticesX; j=p, p++) {
		vector2 axis = x->vertices[p] - x->vertices[j];
		axis.set(-axis[1], axis[0]);
		axis.normalize();
		
		vector2 circleInterval = projectPrimitive(y, axis);
		vector2 polyInterval = projectPrimitive(x, axis);
		
		float intervalDistance = findIntervalDistance(circleInterval, polyInterval);
		if(intervalDistance > 0) {
			newEvent.doIntersect = false;
			return newEvent;	
		}
		
		intervalDistance = fabs(intervalDistance);
		if(intervalDistance < minIntervalDistance) {
			minIntervalDistance = intervalDistance;
			newEvent.normal = axis;
			newEvent.edge = &(x->edges[p]);
			newEvent.vertexOrEdge = 1;
		}
	}
	
	newEvent.distance = minIntervalDistance;
	
	calculatePolyCenter(x);
	vector2 tmp =  y->vertices[0] - x->center;
        if (tmp.dot_product(newEvent.normal) <= 0)
        	newEvent.normal *= -1.f;
        	
	return newEvent;
}


vector2 PhysMgr::projectPrimitive(Primitive* x, vector2 axis) {
	// axis MUST be normalized
	int numVertices = x->type;
	float tmp, min = P_INF, max = N_INF;
	
	switch(x->type) {
	case PRIM_TYPE_TRIANGLE:
	case PRIM_TYPE_QUAD:
		tmp = axis.dot_product(x->vertices[0]);
		min = tmp;
		max = tmp;
		for(int p=1; p<numVertices; p++) {
			tmp = axis.dot_product(x->vertices[p]);
			if(tmp < min) min = tmp;
			else if(tmp > max) max = tmp;	
		}
		return vector2(min, max);
		break;
	case PRIM_TYPE_CIRCLE:
		float tmp = axis.dot_product(x->vertices[0]);
		min = tmp - x->radius;
		max = tmp + x->radius;
		break;
	}
	return vector2(min, max);
}

vector2 PhysMgr::projectEdge(Edge* edge, vector2 axis) {
	float min, max, tmp;
	min = axis.dot_product(*(edge->vertexA));
	max = axis.dot_product(*(edge->vertexB));
	if(min > max) {
		tmp = min;
		min = max;
		max = tmp;	
	}
	return vector2(min, max);
}

float PhysMgr::projectVector(vector2 v, vector2 axis) {
	return axis.dot_product(v);
}

float PhysMgr::findIntervalDistance(vector2 interval_x, vector2 interval_y) {
	if(interval_x[0] < interval_y[0]) return interval_y[0] - interval_x[1];
	else return interval_x[0] - interval_y[1];	
}

float PhysMgr::findIntervalDistance(vector2 interval_x, float interval_y) {
	if(interval_x[0] < interval_y) return interval_x[0] - interval_y;
	else return interval_y - interval_x[1];	
}

float PhysMgr::findPointEdgeDistance(Edge* edge, vector2 p) {
	vector2 vA = *(edge->vertexA), vB = *(edge->vertexB);
	float x = (vB[1]-vA[1])*(p[0]-vA[0])-(vB[0]-vA[0])*(p[1]-vA[1]);
	float y = (vB[0]-vA[0])*(vB[0]-vA[0]) + (vB[1]-vA[1])*(vB[1]-vA[1]);
	x *= x;
	return sqrt(x/y);	
}

bool PhysMgr::doRayCircleTest(Primitive* x, ray* y) {
	vector2 center = x->center;
	float radius = x->radius;
	vector2 ray_start = y->start_pos;
	vector2 ray_end = y->end_pos;
	vector2 ray_dir = ray_end - ray_start;
	ray_dir.normalize();	
	if(x->center[0] > ray_end[0] && x->center[0] > ray_start[0]) return false;
	if(x->center[0] < ray_end[0] && x->center[0] < ray_start[0]) return false;
	vector2 pt_v = center - ray_start;
	
	float projection = pt_v.dot_product(ray_dir);
	vector2 closestPoint;
	if(projection < 0) closestPoint = ray_start;
	else if(projection * projection > (ray_end - ray_start).length2()) closestPoint = ray_end;
	else {
		vector2 projection_v = ray_dir * projection;
		closestPoint = ray_start + projection_v;
	}
	vector2 dist_v = center - closestPoint;
	
	
	if(dist_v.length2() < radius * radius) return true;
	return false;
}

void PhysMgr::updateGridPosition(Primitive* x) {
	x->grid_cell_x = int(x->vertices[0][0] / GRID_SIZE);
	x->grid_cell_y = int(x->vertices[0][1] / GRID_SIZE);	
}

void PhysMgr::updateForces(Primitive* x, vector2 netForce) {
	switch(x->type) {
	case PRIM_TYPE_TRIANGLE:
	case PRIM_TYPE_QUAD:
		for(int p=0; p<(x->type); p++) {
			x->vertices_accel[p] = vector2();
			x->vertices_accel[p] += netForce;
		}
		break;
	case PRIM_TYPE_CIRCLE:
		x->vertices_accel[0] = vector2();
		x->vertices_accel[0] += netForce;
		break;
	}
}

void PhysMgr::updateVerlet(float timeStep, Primitive* x) {
	int numVertices = x->type;
	float minVel = P_INF;
	float maxVel = PHYSICS_MAX_VELOCITY;
	
	switch(x->type) {
	case PRIM_TYPE_TRIANGLE:
	case PRIM_TYPE_QUAD:
		for(int i=0; i<numVertices; i++) {
			vector2 tmp = x->vertices[i];
			if((x->vertices[i] - x->vertices_oldPos[i]).length2() < PHYSICS_MAX_VELOCITY)
				x->vertices[i] += (x->vertices[i] - x->vertices_oldPos[i]) * 0.995f + x->vertices_accel[i]*timeStep*timeStep;
			else x->vertices[i] += (x->vertices[i] - x->vertices_oldPos[i]) * 0.5f + x->vertices_accel[i]*timeStep*timeStep;
			x->vertices_oldPos[i] = tmp;
		}
		break;
	case PRIM_TYPE_CIRCLE:
		vector2 tmp = x->vertices[0];
		if((x->vertices[0] - x->vertices_oldPos[0]).length2() < PHYSICS_MAX_VELOCITY)
			x->vertices[0] += (x->vertices[0] - x->vertices_oldPos[0]) * 0.995f + x->vertices_accel[0]*timeStep*timeStep;
		else x->vertices[0] += (x->vertices[0] - x->vertices_oldPos[0]) * 0.5f + x->vertices_accel[0]*timeStep*timeStep;
		x->vertices_oldPos[0] = tmp;
		break;
	}
}

void PhysMgr::solveConstraints(Primitive *x) {
	int numVertices = x->type;

	// can't do edge correction on circles, so return
	if(x->type == PRIM_TYPE_CIRCLE) return;
	
	for(int i=0; i<numVertices; i++) {
		Edge edge = x->edges[i];
		vector2 v1v2 = *(edge.vertexA) - *(edge.vertexB);
		float diff = v1v2.length() - edge.def_length;
		v1v2.normalize();
		*(edge.vertexA) -= v1v2*diff*0.2f;
		*(edge.vertexB) += v1v2*diff*0.2f;
	}
	for(int i=0; i<x->constraints_n; i++) {
		Edge edge = x->constraints[i];
		vector2 v1v2 = *(edge.vertexA) - *(edge.vertexB);
		float diff = v1v2.length() - edge.def_length;
		v1v2.normalize();
		*(edge.vertexA) -= v1v2*diff*0.2f;
		*(edge.vertexB) += v1v2*diff*0.2f;
	}
}

void PhysMgr::resolveCollisions(CollisionResponseSceneNode* node) {
	GAME_OBJECT_TYPE nodeType = node->getGOType();
	
	switch(nodeType) {
	case GO_TYPE_WALLLASER:
	case GO_TYPE_LASERDOOR:
		return;
	}
	
	aabb2 bounding_box = getBoundingBox(node->getCollisionPrimitive());
	list<Primitive*> levelGeometry = level->getGeometry(bounding_box);
	CollisionEvent newEvent;
	newEvent.doIntersect = false;
	Primitive* prim = node->getCollisionPrimitive();
	for(list<Primitive*>::iterator lvl = levelGeometry.begin(); lvl != levelGeometry.end(); lvl++) {
		if(doFastCircleCircleTest(prim, *lvl)) {
			switch(prim->type) {
			case PRIM_TYPE_TRIANGLE:
			case PRIM_TYPE_QUAD:
				newEvent = doPolygonPolygonTest(prim, *lvl);
				break;
			case PRIM_TYPE_CIRCLE:
				newEvent = doPolygonCircleTest(prim, *lvl);	
				break;		
			}
			if(newEvent.doIntersect) {
				newEvent.node_A = node;
				node->setSleep(false);
				newEvent.node_B = 0;
				if(!node->receiveCollisionEvent(newEvent)) // if node doesn't resolve the collision, do it internally
					processLevelCollision(newEvent);
			}
		}
		delete *lvl;
	}
	
	newEvent.doIntersect = false;
	
	for(list<CollisionResponseSceneNode*>::iterator x = world->begin(); x != world->end(); x++) {
		if(*x != node) {
			Primitive* prim_a = prim;
			Primitive* prim_b = (*x)->getCollisionPrimitive();
			if(abs(prim_a->grid_cell_x - prim_b->grid_cell_x) > 1 && abs(prim_a->grid_cell_y - prim_b->grid_cell_y) > 1) {
				switch((*x)->getGOType()) {
					case GO_TYPE_WALLLASER:
					case GO_TYPE_LASERDOOR:
						if(abs(prim_a->grid_cell_x - prim_b->grid_cell_x) > 2 && abs(prim_a->grid_cell_y - prim_b->grid_cell_y) > 2)
							continue;
						break;
					default:
						continue;
						break;	
				}
			}
			if(doFastCircleCircleTest(prim_a, prim_b)) {
				if(prim_a->type == PRIM_TYPE_CIRCLE && prim_b->type == PRIM_TYPE_CIRCLE) {	
					newEvent = doCircleCircleTest(prim_a, prim_b);
				} else if(prim_a->type == PRIM_TYPE_CIRCLE || prim_b->type == PRIM_TYPE_CIRCLE) {
					newEvent = doPolygonCircleTest(prim_a, prim_b);
				} else {
					newEvent = doPolygonPolygonTest(prim_a, prim_b);	
				}
				
				if(newEvent.doIntersect) {
					newEvent.node_A = node;
					node->setSleep(false);
					newEvent.node_B = *x;
					(*x)->setSleep(false);
					
					processCollision(newEvent);
				}	
			}
		}
	}
}

void PhysMgr::calculatePolyCenter(Primitive *x) {
	vector2 center;
	int vertices_n = (*x).type;
	switch(x->type) {
	case PRIM_TYPE_TRIANGLE:
	case PRIM_TYPE_QUAD:
		for(int i=0; i<vertices_n; i++) {
			center += (*x).vertices[i];	
		}
		center = center * (1.f/vertices_n);
		(*x).center = center;
		break;
	case PRIM_TYPE_CIRCLE:
		(*x).center = (*x).vertices[0];
		break;
	}
}

aabb2 PhysMgr::getBoundingBox(Primitive *x) {
	float min_x, max_x, min_y, max_y;
	int vertices_n = x->type;
	
	switch(x->type) {
	case PRIM_TYPE_TRIANGLE:
	case PRIM_TYPE_QUAD:
		min_x = x->vertices[0][0];
		min_y = x->vertices[0][1];
		max_x = x->vertices[0][0];
		max_y = x->vertices[0][1];
		for(int i=0; i<vertices_n; i++) {
			if(x->vertices[i][0] < min_x) min_x = x->vertices[i][0];
			if(x->vertices[i][1] < min_y) min_y = x->vertices[i][1];
			if(x->vertices[i][0] > max_x) max_x = x->vertices[i][0];
			if(x->vertices[i][1] > max_y) max_y = x->vertices[i][1];
		}
		break;
	case PRIM_TYPE_CIRCLE:
		min_x = x->vertices[0][0] - x->radius;
		min_y = x->vertices[0][1] - x->radius;
		max_x = x->vertices[0][0] + x->radius;
		max_y = x->vertices[0][1] + x->radius;
		break;
	}
	aabb2 bounding_box;
	bounding_box.bottom_left.set(min_x, min_y);
	bounding_box.top_right.set(max_x, max_y);
	return bounding_box;
}

void PhysMgr::processPolyLevelCollision(CollisionEvent event) {
	vector2 collisionVector = event.normal * event.distance;
	if(event.edge != NULL) {
		vector2* vertexA = event.edge->vertexA;
		vector2* vertexB = event.edge->vertexB;
	
		float t;
		if(fabs((*vertexA)[0] - (*vertexB)[0]) > fabs((*vertexA)[1] - (*vertexB)[1]))
			t = ((*event.vertex)[0] - collisionVector[0] - (*vertexA)[0]) / ((*vertexB)[0] - (*vertexA)[0]);
		else
			t = ((*event.vertex)[1] - collisionVector[1] - (*vertexA)[1]) / ((*vertexB)[1] - (*vertexA)[1]);
		
		float lambda = 1.0f/ (t*t + (1.f - t)*(1.f - t));
		*vertexA -= collisionVector*(1.f-t)*lambda*0.5f;
		*vertexB -= collisionVector*t*lambda*0.5f;
	}	

	*(event.vertex) += collisionVector*0.5f;
}

void PhysMgr::processPolyPolyCollision(CollisionEvent event) {
	vector2 collisionVector = event.normal * event.distance;
	if(event.edge != NULL) {
		if(event.edge->parent == event.node_A->getCollisionPrimitive()) {
			if(!event.node_A->receiveCollisionEvent(event)) {
				vector2* vertexA = event.edge->vertexA;
				vector2* vertexB = event.edge->vertexB;
				
				assert(event.node_A != NULL && event.node_B != NULL); 
				
				float t;
				if(fabs((*vertexA)[0] - (*vertexB)[0]) > fabs((*vertexA)[1] - (*vertexB)[1]))
					t = ((*event.vertex)[0] - collisionVector[0] - (*vertexA)[0]) / ((*vertexB)[0] - (*vertexA)[0]);
				else
					t = ((*event.vertex)[1] - collisionVector[1] - (*vertexA)[1]) / ((*vertexB)[1] - (*vertexA)[1]);
				
				float lambda = 1.0f/ (t*t + (1.f - t)*(1.f - t));
				*vertexA -= collisionVector*(1.f-t)*lambda*0.5f;
				*vertexB -= collisionVector*t*lambda*0.5f;
			}
			if(!event.node_B->receiveCollisionEvent(event)) {
				*(event.vertex) += collisionVector*0.5f;
			}
		} else if(event.edge->parent == event.node_B->getCollisionPrimitive()) {
			if(!event.node_B->receiveCollisionEvent(event)) {
				vector2* vertexA = event.edge->vertexA;
				vector2* vertexB = event.edge->vertexB;
				
				assert(event.node_A != NULL && event.node_B != NULL); 
				
				float t;
				if(fabs((*vertexA)[0] - (*vertexB)[0]) > fabs((*vertexA)[1] - (*vertexB)[1]))
					t = ((*event.vertex)[0] - collisionVector[0] - (*vertexA)[0]) / ((*vertexB)[0] - (*vertexA)[0]);
				else
					t = ((*event.vertex)[1] - collisionVector[1] - (*vertexA)[1]) / ((*vertexB)[1] - (*vertexA)[1]);
				
				float lambda = 1.0f/ (t*t + (1.f - t)*(1.f - t));
				*vertexA -= collisionVector*(1.f-t)*lambda*0.5f;
				*vertexB -= collisionVector*t*lambda*0.5f;
			}
			if(!event.node_A->receiveCollisionEvent(event)) {
				*(event.vertex) += collisionVector*0.5f;
			}
		}
	}
}

void PhysMgr::processPolyCircleCollision(CollisionEvent event) {
	assert(event.node_A->getCollisionPrimitive()->type == PRIM_TYPE_CIRCLE || event.node_B->getCollisionPrimitive()->type == PRIM_TYPE_CIRCLE);	
	vector2 collisionVector = event.normal * event.distance;
	
	CollisionResponseSceneNode* tmp;
	if(event.node_A->getCollisionPrimitive()->type != PRIM_TYPE_CIRCLE) {
		tmp = event.node_A;
		event.node_A = event.node_B;
		event.node_B = tmp;	
	}
	
	Primitive* circle;

	circle = event.node_A->getCollisionPrimitive();
	if(!event.node_A->receiveCollisionEvent(event)) {
		circle->vertices[0] += collisionVector*0.2f;	
	}

	if(!event.node_B->receiveCollisionEvent(event)) {
		if(event.vertexOrEdge) {
			*(event.edge->vertexA) -= collisionVector*0.49f;
			*(event.edge->vertexB) -= collisionVector*0.49f;
		}	
	}
}

void PhysMgr::processCircleCircleCollision(CollisionEvent event) {
	vector2 collisionVector = event.normal * event.distance;
	if(!event.node_A->receiveCollisionEvent(event))
		event.node_A->getCollisionPrimitive()->vertices[0] -= collisionVector*0.1f;
		
	if(!event.node_B->receiveCollisionEvent(event))
		event.node_B->getCollisionPrimitive()->vertices[0] += collisionVector*0.1f;	
}

void PhysMgr::processCircleLevelCollision(CollisionEvent event) {
	vector2 collisionVector = event.normal * event.distance;
	CollisionResponseSceneNode* tmp;
	if(event.node_A->getCollisionPrimitive()->type != PRIM_TYPE_CIRCLE) {
		tmp = event.node_A;
		event.node_A = event.node_B;
		event.node_B = tmp;	
	}
	
	event.node_A->getCollisionPrimitive()->vertices[0] += collisionVector*0.1f;
}

void PhysMgr::processLevelCollision(CollisionEvent event) {
	switch(event.type) {
	case COLLISION_TYPE_POLYPOLY:
		processPolyLevelCollision(event);
		break;
	case COLLISION_TYPE_POLYCIRCLE:
		processCircleLevelCollision(event);
		break;
	}
}

void PhysMgr::processCollision(CollisionEvent event) {
	switch(event.type) {
	case COLLISION_TYPE_POLYPOLY:
		processPolyPolyCollision(event);
		break;
	case COLLISION_TYPE_POLYCIRCLE:
		processPolyCircleCollision(event);
		break;
	case COLLISION_TYPE_CIRCLECIRCLE:
		processCircleCircleCollision(event);
		break;
	}	
}

bool PhysMgr::checkMinVel(Primitive* x) {
	float max = 0.f;
	for(int i=0; i<x->type; i++) {
		float vel = (x->vertices[i] - x->vertices_oldPos[i]).length2();
		if(vel > max) max = vel;
	}
	if(max < MIN_VELOCITY_SQUARED) return true;
	return false;
}

void PhysMgr::init(float timeStep_n) {
	timeStep = timeStep_n;
	timeAccumulator = 0.f;
}

void PhysMgr::startSimulation() {
	isRunning = true;	
}

void PhysMgr::stopSimulation() {
	isRunning = false;
}

void PhysMgr::reinit() {
	level = gfx->getLevel();
	delete world;
	isRunning = false;
	world = new list<CollisionResponseSceneNode*>;
	iterations_num = DEFAULT_PHYS_ITERATIONS;	
}

void PhysMgr::update(float frameDelta) {
	if(isRunning) {
	timeAccumulator += frameDelta;
	while(timeAccumulator >= timeStep) {
		for(list<CollisionResponseSceneNode*>::iterator x = world->begin(); x != world->end(); x++) {
			if((*x)->isActive()) {
				vector2 primcenter = (*x)->getCollisionPrimitive()->center;
				vector2 wparams = gfx->getRenderer()->getWindowParams();
				aabb2 view;
				view.bottom_left = gfx->getCamera()->getAbsoluteTranslation();
				view.top_right = view.bottom_left + gfx->getRenderer()->getWindowParams();
				if(primcenter[0] > view.top_right[0]+wparams[0]*0.5f + PHYSICS_SLEEP_EPSILON || primcenter[0] < view.bottom_left[0]-wparams[0]*0.5f - PHYSICS_SLEEP_EPSILON) continue;
				if(primcenter[1] > view.top_right[1]+wparams[1]*0.5f + PHYSICS_SLEEP_EPSILON || primcenter[1] < view.bottom_left[1]-wparams[1]*0.5f - PHYSICS_SLEEP_EPSILON) continue;
				if(!(*x)->checkStatic()) {
					updateForces((*x)->getCollisionPrimitive(), (*x)->getNetForce());
					updateVerlet(timeStep, (*x)->getCollisionPrimitive());
				}
				updateGridPosition((*x)->getCollisionPrimitive());
				calculatePolyCenter((*x)->getCollisionPrimitive());
				for(int iteration=0; iteration<iterations_num*2; iteration++) {
					solveConstraints((*x)->getCollisionPrimitive());
					if(iteration % 2) resolveCollisions(*x);
				}
			}
		}
		timeAccumulator -= timeStep;
	}
	}
};

void PhysMgr::registerLevelSceneNode(LevelSceneNode* level_n) {
	level = level_n;
}

void PhysMgr::registerSceneNode(CollisionResponseSceneNode* newNode) {
	world->push_back(newNode);
	newNode->setSleep(false);
}

void PhysMgr::unregisterSceneNode(CollisionResponseSceneNode* node) {
	for(list<CollisionResponseSceneNode*>::iterator x = world->begin(); x != world->end(); x++) {
		if(*x == node) {
			world->erase(x);
			return;	
		}	
	}
}

CollisionEvent PhysMgr::primitiveCollisionTest(Primitive* prim, bool doLevelTest, bool doObjectsTest, CollisionResponseSceneNode* caller, GAME_OBJECT_TYPE type) {
	aabb2 bounding_box = getBoundingBox(prim);
	list<Primitive*> levelGeometry = level->getGeometry(bounding_box);

	updateGridPosition(prim);

	CollisionEvent newEvent;
	newEvent.doIntersect = false;
	if(doLevelTest) {
		for(list<Primitive*>::iterator lvl = levelGeometry.begin(); lvl != levelGeometry.end(); lvl++) {
			if(doFastCircleCircleTest(prim, *lvl)) {
				switch(prim->type) {
				case PRIM_TYPE_TRIANGLE:
				case PRIM_TYPE_QUAD:
					newEvent = doPolygonPolygonTest(prim, *lvl);
					break;
				case PRIM_TYPE_CIRCLE:
					newEvent = doPolygonCircleTest(prim, *lvl);	
					break;		
				}
				if(newEvent.doIntersect) {
					newEvent.node_A = caller;
					newEvent.node_B = 0;
					return newEvent;
				}
			}
		}
	}
	newEvent.doIntersect = false;

	if(doObjectsTest)
	for(list<CollisionResponseSceneNode*>::iterator x = world->begin(); x != world->end(); x++) {
		if(*x == caller) continue;
		Primitive* prim_a = prim;
		Primitive* prim_b = (*x)->getCollisionPrimitive();
		updateGridPosition(prim_b);
		if(abs(prim_a->grid_cell_x - prim_b->grid_cell_x) > 1 && abs(prim_a->grid_cell_y - prim_b->grid_cell_y) > 1) {
			if((*x)->getGOType() != GO_TYPE_WALLLASER)
				continue;
			else if((*x)->getGOType() != GO_TYPE_LASERDOOR)
				continue;
		}
		if(doFastCircleCircleTest(prim, (*x)->getCollisionPrimitive())) {
			if(prim_a->type == PRIM_TYPE_CIRCLE && prim_b->type == PRIM_TYPE_CIRCLE) {
				newEvent = doCircleCircleTest(prim_a, prim_b);	
			} else if(prim_a->type == PRIM_TYPE_CIRCLE || prim_b->type == PRIM_TYPE_CIRCLE) {
				newEvent = doPolygonCircleTest(prim_a, prim_b);
			} else {
				newEvent = doPolygonPolygonTest(prim_a, prim_b);	
			}
			
			if(newEvent.doIntersect) {
				newEvent.node_A = caller;
				newEvent.node_B = *x;
				if(type != GO_TYPE_ALL)
					if(type == (*x)->getGOType())
						return newEvent;
					else continue;
				else
					return newEvent;
			}
		}		
	}
	return newEvent;
}

bool PhysMgr::rayVisibilityTest(Primitive* circle, ray* x, CollisionResponseSceneNode* caller) {
	float distance = (x->end_pos - x->start_pos).length2();
	if(distance > MAX_VIEW_RANGE_SQ) return false;
	float minDistance = P_INF;
	aabb2 bounding_box;

	if(x->start_pos[0] > x->end_pos[0] && x->start_pos[1] > x->end_pos[1]) {
		bounding_box.bottom_left = x->end_pos;
		bounding_box.top_right = x->start_pos;
	} else if(x->start_pos[0] < x->end_pos[0] && x->start_pos[1] < x->end_pos[1]) {
		bounding_box.bottom_left = x->start_pos;
		bounding_box.top_right = x->end_pos;
	} else if(x->start_pos[0] < x->end_pos[0] && x->start_pos[1] > x->end_pos[1]) {
		bounding_box.bottom_left.set(x->start_pos[0], x->end_pos[1]);
		bounding_box.top_right.set(x->end_pos[0], x->start_pos[1]);
	} else {
		bounding_box.bottom_left.set(x->end_pos[0], x->start_pos[1]);
		bounding_box.top_right.set(x->start_pos[0], x->end_pos[1]);
	}

	list<Primitive*> levelGeometry = level->getGeometry(bounding_box);

	for(list<Primitive*>::iterator lvl = levelGeometry.begin(); lvl != levelGeometry.end(); lvl++) {
		Primitive* prim = *lvl;
		float tmp = (prim->center - x->start_pos).length2();
				
		if(tmp < distance) {
			if(doRayCircleTest(prim, x)) return false;
		}
	}
	
	for(list<CollisionResponseSceneNode*>::iterator p = world->begin(); p != world->end(); p++) {
		if(*p == caller) continue;
		Primitive* prim = (*p)->getCollisionPrimitive();
		float tmp = (prim->center - x->start_pos).length2();
		
		if(tmp < distance && (*p)->getGOType() != GO_TYPE_WALLLASER && (*p)->getGOType() != GO_TYPE_SWITCH)
			if(doRayCircleTest(prim, x)) return false;
	}

	return true;
}

void PhysMgr::setQuadEdgesAndConstraints(Primitive* quad) {
	gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	
	quad->edges[0].vertexA = &(quad->vertices[0]);
	quad->edges[0].vertexB = &(quad->vertices[1]);
	quad->edges[0].def_length = (quad->vertices[0] - quad->vertices[1]).length();
	quad->edges[0].parent = quad;
	quad->edges[1].vertexA = &(quad->vertices[1]);
	quad->edges[1].vertexB = &(quad->vertices[2]);
	quad->edges[1].def_length = (quad->vertices[1] - quad->vertices[2]).length();
	quad->edges[1].parent = quad;
	quad->edges[2].vertexA = &(quad->vertices[2]);
	quad->edges[2].vertexB = &(quad->vertices[3]);
	quad->edges[2].def_length = (quad->vertices[2] - quad->vertices[3]).length();
	quad->edges[2].parent = quad;
	quad->edges[3].vertexA = &(quad->vertices[3]);
	quad->edges[3].vertexB = &(quad->vertices[0]);
	quad->edges[3].def_length = (quad->vertices[0] - quad->vertices[3]).length();
	quad->edges[3].parent = quad;
	quad->constraints[0].vertexA = &(quad->vertices[0]);
	quad->constraints[0].vertexB = &(quad->vertices[2]);
	quad->constraints[0].def_length = (quad->vertices[0] - quad->vertices[2]).length();
	quad->constraints[1].vertexA = &(quad->vertices[1]);
	quad->constraints[1].vertexB = &(quad->vertices[3]);
	quad->constraints[1].def_length = (quad->vertices[1] - quad->vertices[3]).length();
	quad->constraints_n = 2;	
}

void PhysMgr::setIterations(int iterations) {
	iterations_num = iterations;
}
