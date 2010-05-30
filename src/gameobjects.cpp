/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */

#include "engine/headers.h"

using namespace std;

// Switch class

Switch::Switch(GfxMgr* gfx_n) {
	gfx = gfx_n;
	target = NULL;
	state = false;
	timer = 0.f;
	switch_times = 0;
	_collisionReady = true;
}

Switch::~Switch() {
	
}

void Switch::init(vector2 coords, int switch_times_n, SceneNode* target_n) {
	target = target_n;
	if(target)
		state = target->getSwitch();
	switch_times = switch_times_n;
	
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("SWITCH");
	sprite->setSprite(texid);
	sprite->setActive(true);
	if(state) {
		sprite->setFrame(1);	
	} else {
		sprite->setFrame(2);
	}
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0] = size*-0.5f;
	shape->vertices[1].set(size[0]/2.f, -size[1]/2.f);
	shape->vertices[2] = size*0.5f;
	shape->vertices[3].set(-size[0]/2.f, size[1]/2.f);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(0.f, 0.f), vector2(0.f, 0.f), false);
	for(int x=0; x != 4; x++) sprite->getPrimitive()->vertices[x] = body->getCollisionPrimitive()->vertices[x];
	body->setActive(true);
	body->setStatic(true);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_SWITCH);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
	if(target) target->setSwitch(state);
}
void Switch::update(float frameDelta) {
	if(_isActive) {
		if(target)
			state = target->getSwitch();
		if(state) {
				sprite->setFrame(1);	
			} else {
				sprite->setFrame(2);
		}
		timer -= frameDelta;
	
		SceneNode::update(frameDelta);
	}
}

bool Switch::receiveCollisionEvent(CollisionEvent event) {
	if(target && _collisionReady && timer <= 0.f && (switch_times > 0 || switch_times == -1)) {
		state = !state;
		target->setSwitch(state);
		_collisionReady = false;
		timer = 1.f;
		if(switch_times != -1)
			switch_times--;
	}
	return true;
}

Primitive* Switch::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// Checkpoint class

Checkpoint::Checkpoint(GfxMgr* gfx_n) {
	gfx = gfx_n;
	state = false;
	_collisionReady = true;
}

Checkpoint::~Checkpoint() {
	
}

void Checkpoint::init(vector2 coords, int id_n) {
	id = id_n;
	gfx->getLevel()->addCheckpoint(id, coords);
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("CHECKPOINT");
	sprite->setSprite(texid);
	sprite->setActive(true);
	state = false;
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0] = size*-0.5f;
	shape->vertices[1].set(size[0]/2.f, -size[1]/2.f);
	shape->vertices[2] = size*0.5f;
	shape->vertices[3].set(-size[0]/2.f, size[1]/2.f);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(0.f, 0.f), vector2(0.f, 0.f), false);
	for(int x=0; x != 4; x++) sprite->getPrimitive()->vertices[x] = body->getCollisionPrimitive()->vertices[x];
	body->setActive(true);
	body->setStatic(true);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_CHECKPOINT);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
}
void Checkpoint::update(float frameDelta) {
	SceneNode::update(frameDelta);	
}

bool Checkpoint::receiveCollisionEvent(CollisionEvent event) {
	if(_collisionReady && event.node_A->getGOType() == GO_TYPE_CHAR && !state) {
		state = true;
		_collisionReady = false;
		sprite->setFrame(1);
		gfx->getLevel()->setLastVisitedCheckpoint(id);
	}
	
	return true;
}

Primitive* Checkpoint::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// Pressure Pad class

PressurePad::PressurePad(GfxMgr* gfx_n) {
	gfx = gfx_n;
	target = NULL;
	state = false;
	timer = 0.1f;
	timer2 = 0.0f;
	_collisionReady = true;
}

PressurePad::~PressurePad() {
	
}

void PressurePad::init(vector2 coords, SceneNode* target_n) {
	if(target_n) {
		target = target_n;
		obj_state = target->getSwitch();
		state = obj_state;
	} else {
		target = NULL;	
	}	
	
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("PRESSUREPAD");
	sprite->setSprite(texid);
	sprite->setActive(true);
	if(state) {
		sprite->setFrame(0);	
	} else {
		sprite->setFrame(1);
	}
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0] = size*-0.5f;
	shape->vertices[1].set(size[0]/2.f, -size[1]/2.f);
	shape->vertices[2] = size*0.5f;
	shape->vertices[3].set(-size[0]/2.f, size[1]/2.f);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(0.f, 0.f), vector2(0.f, 0.f), false);
	for(int x=0; x != 4; x++) sprite->getPrimitive()->vertices[x] = body->getCollisionPrimitive()->vertices[x];
	body->setActive(true);
	body->setStatic(true);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_PRESSUREPAD);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);

}
void PressurePad::update(float frameDelta) {
	SceneNode::update(frameDelta);	
	timer -= frameDelta;
	if(timer <= 0) {
		if(timer2 == 0.f)
			timer2 = 0.1f;
	}
	if(timer2 > 0.f) {
		if(body->inView())
			timer2 -= frameDelta;
		if(timer2 <= 0) {
			state = false;
			sprite->setFrame(1);
			if(target)
				target->setSwitch(obj_state);	
		}	
	}
}

bool PressurePad::receiveCollisionEvent(CollisionEvent event) {
	if(target && timer <= 0.f && event.node_B) {
		if(event.node_A->getGOType() == GO_TYPE_CHAR || event.node_B->getGOType() == GO_TYPE_CHAR) {
			_collisionReady = false;
			return true;
		}
		vector2 collisionVector = event.normal * event.distance;
	//	collisionVector.normalize();
		vector2 axis(1.f, 0.f);
		axis.normalize();
		float anglecos = event.normal.dot_product(axis);
		if(anglecos > -0.2f && anglecos < 0.2f) {
			state = true;
			sprite->setFrame(0);	
			if(target)
				target->setSwitch(!obj_state);
		}
		_collisionReady = false;
		timer2 = 0.f;
		timer = 0.1f;
	}
	return true;
}

Primitive* PressurePad::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// Level exit class

LevelExit::LevelExit(GfxMgr* gfx_n) {
	gfx = gfx_n;	
}

LevelExit::~LevelExit() {
	
}

void LevelExit::init(vector2 coords) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("EXIT");
	sprite->setSprite(texid);
	sprite->setActive(true);
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0] = size*-0.5f;
	shape->vertices[1].set(size[0]/2.f, -size[1]/2.f);
	shape->vertices[2] = size*0.5f;
	shape->vertices[3].set(-size[0]/2.f, size[1]/2.f);
	sprite->getPrimitive()->vertices_colors[0].set(1.f, 1.f, 1.f, 1.f);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(0.f, 0.f), vector2(0.f, 0.f), false);
	for(int x=0; x<4; x++) sprite->getPrimitive()->vertices[x] = body->getCollisionPrimitive()->vertices[x];
	body->setActive(true);
	body->setStatic(true);
	body->setSleep(false);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_EXIT);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
		
	gfx->getRenderer()->findPrimitiveBoundingCircle(body->getCollisionPrimitive());
	gfx->getPhysMgr()->calculatePolyCenter(body->getCollisionPrimitive());
	
}

void LevelExit::update(float frameDelta) {
	SceneNode::update(frameDelta);
}

bool LevelExit::receiveCollisionEvent(CollisionEvent event) {
	return true;	
}

Primitive* LevelExit::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// Mine class

Mine::Mine(GfxMgr* gfx_n) {
	gfx = gfx_n;
}

Mine::~Mine() {
	
}

void Mine::init(vector2 coords) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("MINE");
	sprite->setSprite(texid);
	sprite->setActive(true);
	sprite->setAnimationLoop(0, 1, 1);
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_CIRCLE;
	shape->radius = 8.f;
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(), vector2(0.f, 0.f), true);
	body->setActive(true);
	body->setStatic(true);
//	body->setSleep(false);
//	body->setGravity(false);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_MINE);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);

	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	x->vertices[0] = y->vertices[0] - (x->size * 0.5f);
	x->vertices[1].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
	x->vertices[2] = y->vertices[0] + (x->size * 0.5f);
	x->vertices[3].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
	delete shape;
}
void Mine::update(float frameDelta) {
	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	x->vertices[0] = y->vertices[0] - (x->size * 0.5f);
	x->vertices[1].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
	x->vertices[2] = y->vertices[0] + (x->size * 0.5f);
	x->vertices[3].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
	SceneNode::update(frameDelta);	
}

bool Mine::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_B) {
		switch(event.node_B->getGOType()) {
			case GO_TYPE_WALLLASER:
			case GO_TYPE_LASERDOOR:
			case GO_TYPE_ROBOTGUARD:
				break;	
			default:
				gfx->getSfxMgr()->spawnExplosionEffect(getCollisionPrimitive()->center);
				_markForDeletion = true;
				gfx->getSndMgr()->playSound("EXPLOSION", getCollisionPrimitive()->center);
				break;
		}
	}
	return true;
}

Primitive* Mine::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// GravityLift class

GravityLift::GravityLift(GfxMgr* gfx_n) {
	gfx = gfx_n;
	soundsCount = 0;
	soundsTimer = 0.3f;
}

GravityLift::~GravityLift() {
	
}

void GravityLift::init(vector2 coords, float force_n) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("GRAVITYLIFT");
	sprite->setSprite(texid);
	sprite->setActive(true);
	sprite->setAnimationLoop(0, 3, 5);
	force = force_n;
	
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];

	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0] = size*-0.5f + vector2(10.f, 10.f);
	shape->vertices[1].set(size[0]/2.f-10.f, -size[1]/2.f+10.f);
	shape->vertices[2] = size*0.5f + vector2(-10.f, -10.f);
	shape->vertices[3].set(-size[0]/2.f+10.f, size[1]/2.f-10.f);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(), vector2(0.f, 0.f), true);
	body->setActive(true);
	body->setStatic(true);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_GRAVITYLIFT);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);

	DrawPrimitive* drawshape = sprite->getPrimitive();
	drawshape->vertices[0] = size*-0.5f;
	drawshape->vertices[1].set(size[0]/2.f, -size[1]/2.f);
	drawshape->vertices[2] = size*0.5f;
	drawshape->vertices[3].set(-size[0]/2.f, size[1]/2.f);
	for(int x=0; x<4; x++) drawshape->vertices[x] += coords;
}
void GravityLift::update(float frameDelta) {
	SceneNode::update(frameDelta);
	soundsTimer -= frameDelta;
	if(soundsTimer < 0.f) {
		soundsCount = 0;
		soundsTimer = 0.3f;	
	}
}

bool GravityLift::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_A)
	if(event.node_A->getGOType() != GO_TYPE_WALLLASER && event.node_A->getGOType() != GO_TYPE_LASERDOOR) {
		if(event.node_A->getCollisionPrimitive()->type == PRIM_TYPE_CIRCLE)
			event.node_A->getCollisionPrimitive()->vertices_oldPos[0].set(event.node_A->getCollisionPrimitive()->vertices_oldPos[0][0], event.node_A->getCollisionPrimitive()->vertices[0][1]-force);
		else for(int x=0; x<event.node_A->getCollisionPrimitive()->type; x++)
			event.node_A->getCollisionPrimitive()->vertices_oldPos[x].set(event.node_A->getCollisionPrimitive()->vertices_oldPos[x][0], event.node_A->getCollisionPrimitive()->vertices[x][1]-force);
		if(_collisionReady && soundsCount < 1) {
			gfx->getSndMgr()->playSound("GRAVLIFT", event.node_A->getCollisionPrimitive()->center);
			soundsCount++;
		}
	}
	_collisionReady = false;
	return true;
}

Primitive* GravityLift::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// WallLaser class

WallLaser::WallLaser(GfxMgr* gfx_n) {
	gfx = gfx_n;
	go_type = GO_TYPE_WALLLASER;
	beamLength = 0;
	orientation = UP;
	beamState = false;
	beamInterval = 0;

	sprite = NULL;
	beam = NULL;
	beamCollision = NULL;
	
	beamAnimationStart = 1;
	beamAnimationEnd = 14;
	
	_markForDeletion = false;
}

WallLaser::~WallLaser() {
	stopBeam();
}

void WallLaser::findBeamLength() {
	int x, y;
	beamLength = 0;
	vector2 tilexy = gfx->getLevel()->getTileByXY(sprite->getPrimitive()->center);
	x = (int)tilexy[0];
	y = (int)tilexy[1];
	switch(orientation) {
	case UP:
		y++;
		break;
	case DOWN:
		y--;
		break;
	case RIGHT:
		x ++;
		break;
	case LEFT:
		x--;
		break;
	}
	int tile = gfx->getLevel()->getTile(x, y);
	while(tile < 15) {
		if(tile == 0)
			beamLength += PPT_X;
		else beamLength += PPT_X/2+2.f;
		switch(orientation) {
		case UP:
			y++;
			break;
		case DOWN:
			y--;
			break;
		case RIGHT:
			x++;
			break;
		case LEFT:
			x--;
			break;
		}
		tile = gfx->getLevel()->getTile(x, y);	
	}
}

void WallLaser::init(vector2 coords, bool type_n, float interval) {
	collisionPrimitive = new Primitive;
	collisionPrimitive->center = coords;
	
	type = type_n;
	
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid;
	if(type) texid = gfx->getTexMgr()->getId("WALLLASER");
	else texid = gfx->getTexMgr()->getId("WALLDOOR");
	sprite->setSprite(texid);
	sprite->setActive(true);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	DrawPrimitive* quad = sprite->getPrimitive();
	
	vector2 tilecoords = gfx->getLevel()->getTileByXY(coords);
	int tile = gfx->getLevel()->getTile(coords);
	
	vector2 tile_center;
	tile_center.set(tilecoords[0]*PPT_X+PPT_X/2, tilecoords[1]*PPT_Y+PPT_Y/2);
	
	vector2 dfc;
	dfc = tile_center - coords; // distance from center of tile
	
	if(tile == 15) {
		// implicit tile grid
		if(dfc[0] > -6 && dfc[0] < 6 && dfc[1] > - 12 && dfc[1] < 0) {
			quad->vertices[0].set(0.f, 0.f);
			quad->vertices[1].set(size[0], 0.f);
			quad->vertices[2].set(size[0], size[1]);
			quad->vertices[3].set(0.f, size[1]);
			orientation = UP;
			for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(size[0]/2, 0.f); //(PPT_Y-size[1])/2
		} else if(dfc[0] > -6 && dfc[0] < 6 && dfc[1] < 12 && dfc[1] > 0) { 
			quad->vertices[3].set(0.f, 0.f);
			quad->vertices[2].set(size[0], 0.f);
			quad->vertices[1].set(size[0], size[1]);
			quad->vertices[0].set(0.f, size[1]);
			orientation = DOWN;
			for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(size[0]/2, PPT_Y/2);
		} else if(dfc[0] < 0 && dfc[0] > -12 && dfc[1] > - 6 && dfc[1] < 6) {
			quad->vertices[0].set(0.f, 0.f);
			quad->vertices[3].set(size[0], 0.f);
			quad->vertices[2].set(size[0], size[1]);
			quad->vertices[1].set(0.f, size[1]);
			orientation = RIGHT;
			for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(0.f, size[1]/2);	
		} else if(dfc[0] > 0 && dfc[0] < 12 && dfc[1] > -6 && dfc[1] < 6) {
			quad->vertices[3].set(0.f, 0.f);
			quad->vertices[0].set(size[0], 0.f);
			quad->vertices[1].set(size[0], size[1]);
			quad->vertices[2].set(0.f, size[1]);
			orientation = LEFT;
			for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(PPT_X/2, size[1]/2);	
		}
	}
	gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	beamInterval = interval;
	timeToSwitch = beamInterval;
	
	gfx->addSceneNode(sprite, this);
	findBeamLength();
	if(interval == -1.f) startBeam();
}

void WallLaser::update(float frameDelta) {
	if(timeToSwitch > 0) {
		timeToSwitch -= frameDelta;
		if(timeToSwitch <= 1.f) {
			if(!beamState && !sprite->isAnimationRunning()) {
				sprite->setAnimationOnce(beamAnimationStart, beamAnimationEnd, 0, 14);
			}
		}
		if(timeToSwitch <= 0.f) {
			switchBeam();	
			timeToSwitch = beamInterval;
		}
	}
	
	SceneNode::update(frameDelta);
}

void WallLaser::setSwitch(bool state) {
	if(timeToSwitch > 0) timeToSwitch = 0.f;
	if(state && !beamState) startBeam();
	else if(!state && beamState) stopBeam();
}

bool WallLaser::getSwitch() {
	return beamState;	
}

void WallLaser::switchBeam() {
	if(beamState) stopBeam();
	else startBeam();
}

void WallLaser::startBeam() {
	if(beam) gfx->deleteSceneNode(beam);
	beam = new AnimatedSpriteSceneNode(gfx);
	int texid;
	if(type)
		texid = gfx->getTexMgr()->getId("LASERBEAM");
	else texid = gfx->getTexMgr()->getId("LASERDOOR");
	beam->setSprite(texid);
	beam->setActive(true);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	DrawPrimitive* quad = beam->getPrimitive();
	DrawPrimitive* laser = sprite->getPrimitive();
	
	float start_x, start_y, displacement = 4.f;
	if(type == 0) displacement = 6.f;
	vector2 tmp = (laser->vertices[2] + laser->vertices[3]) * 0.5f;
	
	switch(orientation) {
	case UP:
		start_x = tmp[0] - displacement/2.f;
		start_y = tmp[1];
		quad->vertices[0].set(start_x, start_y);
		quad->vertices[1].set(start_x+displacement, start_y);
		quad->vertices[2].set(start_x+displacement, start_y + beamLength);
		quad->vertices[3].set(start_x, start_y + beamLength);
		break;
	case DOWN:
		start_x = tmp[0] - displacement/2.f;
		start_y = tmp[1];
		quad->vertices[0].set(start_x, start_y);
		quad->vertices[1].set(start_x+displacement, start_y);
		quad->vertices[2].set(start_x+displacement, start_y - beamLength);
		quad->vertices[3].set(start_x, start_y - beamLength);
		break;
	case LEFT:
		start_x = tmp[0];
		start_y = tmp[1] - displacement/2.f;
		quad->vertices[0].set(start_x, start_y);
		quad->vertices[1].set(start_x, start_y+displacement);
		quad->vertices[2].set(start_x - beamLength, start_y+displacement);
		quad->vertices[3].set(start_x - beamLength, start_y);
		break;
	case RIGHT:
		start_x = tmp[0];
		start_y = tmp[1] - displacement/2.f;
		quad->vertices[0].set(start_x, start_y);
		quad->vertices[1].set(start_x, start_y+displacement);
		quad->vertices[2].set(start_x + beamLength, start_y+displacement);
		quad->vertices[3].set(start_x + beamLength, start_y);
		break;
	}
	quad->vertices_colors[0].set(1.f, 1.f, 1.f, 0.75f);
	
	if(beamCollision) gfx->deleteSceneNode(beamCollision);
	
	Primitive* quadPhys = new Primitive;
	quadPhys->type = PRIM_TYPE_QUAD;
	for(int x=0; x<4; x++) quadPhys->vertices[x] = quad->vertices[x];
	beamCollision = new CollisionResponseSceneNode(gfx);
	beamCollision->init(quadPhys, vector2(), vector2(), vector2(), false);
	beamCollision->setStatic(true);
	beamCollision->setActive(true);
	beamCollision->setSleep(false);
	beamCollision->registerCollisionCallback(this);
	if(type)
		beamCollision->setGOType(GO_TYPE_WALLLASER);
	else beamCollision->setGOType(GO_TYPE_LASERDOOR);
	gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	gfx->addSceneNode(beam, gfx->getLevel());
	gfx->addSceneNode(beamCollision, beam);
	beamState = true;
}

void WallLaser::stopBeam() {
	beamState = false;
	if(beam) gfx->deleteSceneNode(beam);
	beam = NULL;
	beamCollision = NULL;
}

bool WallLaser::receiveCollisionEvent(CollisionEvent event) {	
	return true;
}


Primitive* WallLaser::getCollisionPrimitive() {
	return collisionPrimitive;
}

// RobotGuard class

RobotGuard::RobotGuard(GfxMgr* gfx_n) {
	gfx = gfx_n;
	patrol_direction = false;
	patrol_switch = false;
}

RobotGuard::~RobotGuard() {
	
}

void RobotGuard::init(vector2 coords) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("ROBOTGUARD");
	sprite->setSprite(texid);
	sprite->setActive(true);
//	sprite->setAnimationLoop(6, 9, 3);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_CIRCLE;
	shape->radius = 30.f;
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, coords, vector2(0.f, 0.f), vector2(0.f, 0.f), true);
	body->setActive(true);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_ROBOTGUARD);
	gfx->getRenderer()->findPrimitiveBoundingCircle(body->getCollisionPrimitive());
	gfx->getPhysMgr()->calculatePolyCenter(body->getCollisionPrimitive());
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
}

void RobotGuard::update(float frameDelta) {
	SceneNode::update(frameDelta);
	
	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	
	vector2 center = y->vertices[0];
	
	if(doPatrol && state) {
		if(patrol_direction) {
			if(center[0] > patrol_startpos[0]) {
				if(!patrol_switch) {
					body->addForce(vector2(-moveSpeed, 0));
					patrol_switch = true;
				}
			} else {
			//	fireProjectile(vector2(1.f, 1.f));
				patrol_direction = false;	
				body->setForce(vector2());
				patrol_switch = false;
			}		
		} else {
			if(center[0] < patrol_endpos[0]) {
				if(!patrol_switch) {
					body->addForce(vector2(moveSpeed, 0));
					patrol_switch = true;
				}
			} else {
			//	fireProjectile(vector2(1.f, 1.f));
				patrol_direction = true;	
			 	body->setForce(vector2());
			 	patrol_switch = false;
			}
		}
	}
	
	if(doFire && state) {
		fireTimer -= frameDelta;
		if(fireTimer <= 0) {
			Primitive* target = gfx->getLevel()->getPlayableCharacter()->getCollisionPrimitive();
			ray* a = new ray;
			a->start_pos = body->getCollisionPrimitive()->center;
			a->end_pos = target->center;
			
			if(gfx->getPhysMgr()->rayVisibilityTest(target, a, body)) {
				fireProjectile(a->end_pos - a->start_pos);	
			}
			delete a;
			fireTimer = 1.f/firingSpeed;
		}
	}

	if(fabs(y->vertices[0][0] - y->vertices_oldPos[0][0]) < 0.25f) {
		sprite->setAnimationLoop(6, 9, 3);
	} else {
		sprite->setAnimationLoop(0, 5, 5);
	}

	if(y->vertices[0][0] - y->vertices_oldPos[0][0] > 0) {
		x->vertices[0] = y->vertices[0] - (x->size * 0.5f);
		x->vertices[1].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
		x->vertices[2] = y->vertices[0] + (x->size * 0.5f);
		x->vertices[3].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
	} else {
		x->vertices[1] = y->vertices[0] - (x->size * 0.5f);
		x->vertices[0].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
		x->vertices[3] = y->vertices[0] + (x->size * 0.5f);
		x->vertices[2].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
	}	
}

void RobotGuard::setSwitch(bool state) {
	if(state) {
		doPatrol = true;
		doFire = true;
		state = true;
	} else {
		doPatrol = false;
		doFire = false;
		state = false;
		body->setForce(vector2());
		patrol_switch = false;
	}
}

bool RobotGuard::getSwitch() {
	return doPatrol;
}

void RobotGuard::setPatrol(bool state_n, vector2 startpos, vector2 endpos, float moveSpeed_n) {
	state = state_n;
	if(startpos[0] < endpos[0]) {
		patrol_startpos = startpos;
		patrol_endpos = endpos;
	} else {
		patrol_startpos = endpos;
		patrol_endpos = startpos;	
	}
	patrol_direction = false;
	moveSpeed = moveSpeed_n;
	doPatrol = true;
}
void RobotGuard::setFire(bool state_n, float firingSpeed_n, float projectileSpeed_n) {
	state = state_n;
	firingSpeed = firingSpeed_n;
	fireTimer = 1.f/firingSpeed;
	projectileSpeed = projectileSpeed_n;
	if(firingSpeed > 0) {
		doFire = true;	
	}
}

void RobotGuard::fireProjectile(vector2 direction) {
	direction.normalize();
	vector2 axis(1.f, 0.f);
	axis.normalize();
	float dotprod = direction.dot_product(axis);
	if(dotprod > -0.1f && dotprod < 0.1f) return;
	PlasmaProjectile* projectile = new PlasmaProjectile(gfx);
	Primitive* prim = body->getCollisionPrimitive();
	vector2 vel = prim->vertices[0] - prim->vertices_oldPos[0];
	float speed = projectileSpeed + vel.length2();
	projectile->init(prim->vertices[0] + direction*40.f, speed, direction, body);
	projectile->setActive(true);
	gfx->addSceneNode(projectile, this);
	gfx->getSndMgr()->playSound("PLASMA", prim->center);
}

bool RobotGuard::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_B) {
		switch(event.node_B->getGOType()) {
		case GO_TYPE_SWITCH:
		case GO_TYPE_MINE:
			return true;
		case GO_TYPE_PLASMAPROJECTILE:
			gfx->getSfxMgr()->spawnDisintegrationEffect(event.node_B->getCollisionPrimitive()->center);
			_markForDeletion = true;
			break;
		}
	}
	
	return false;
}

Primitive* RobotGuard::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// PlasmaProjectile class
PlasmaProjectile::PlasmaProjectile(GfxMgr* gfx_n) {
	gfx = gfx_n;
	moveSpeed = 1.f;
	direction = vector2(1.f, 0.f);	
	lifeTime = MAX_PROJECTILE_LIFETIME;
	parent_body = NULL;
}

PlasmaProjectile::~PlasmaProjectile() {
	gfx->getSfxMgr()->spawnDisintegrationEffect(getCollisionPrimitive()->center);
}
	
void PlasmaProjectile::update(float frameDelta) {
	if(_isActive) {
		lifeTime -= frameDelta;
		if(lifeTime <= 0) _markForDeletion = true;
		SceneNode::update(frameDelta);
		DrawPrimitive* quad = sprite->getPrimitive();
		Primitive* physQuad = body->getCollisionPrimitive();
		
		quad->vertices[0] = physQuad->vertices[0] - (quad->size * 0.5f);
		quad->vertices[1].set(physQuad->vertices[0][0]+quad->size[0]*0.5f, physQuad->vertices[0][1]-quad->size[1]*0.5f);
		quad->vertices[2] = physQuad->vertices[0] + (quad->size * 0.5f);
		quad->vertices[3].set(physQuad->vertices[0][0]-quad->size[0]*0.5f, physQuad->vertices[0][1]+quad->size[1]*0.5f);
	}		
}

void PlasmaProjectile::init(vector2 coords, float moveSpeed_n, vector2 direction_n, CollisionResponseSceneNode* parent_body_n) {
	direction = direction_n;
	moveSpeed = moveSpeed_n;
	direction.normalize();
	parent_body = parent_body_n;

	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("PLASMAPROJECTILE");
	sprite->setSprite(texid);
	sprite->setActive(true);
	sprite->setAnimationLoop(0, 1, 12);
	
	Primitive* quadPhys = new Primitive;
	quadPhys->type = PRIM_TYPE_CIRCLE;
	quadPhys->radius = 8.f;
	
	body = new CollisionResponseSceneNode(gfx);
	body->init(quadPhys, coords, direction * moveSpeed, vector2(0.f, 0.f), false);
//	body->setStatic(false);
	body->setActive(true);
	body->setSleep(false);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_PLASMAPROJECTILE);
	body->setGravity(false);
	//gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
}

bool PlasmaProjectile::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_B)
	if(event.node_B->getGOType() == GO_TYPE_SWITCH) {
		return true;	
	}
	
	if(event.node_B == parent_body || event.node_A == parent_body) return true;

	if(event.distance < 2.f)
		_markForDeletion = true;

	return true;
}

Primitive* PlasmaProjectile::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}


CollisionResponseSceneNode* PlasmaProjectile::getParentBody() {
	return parent_body;
}

// Ragdoll class

Ragdoll::Ragdoll(GfxMgr* gfx_n) {
	gfx = gfx_n;
	go_type = GO_TYPE_RAGDOLL;
	_markForDeletion = false;
	_collisionReady = true;
}

Ragdoll::~Ragdoll() {}

void Ragdoll::addFriction(CollisionEvent event) {
	if(event.node_B) if(event.node_B->getGOType() == GO_TYPE_CHAR) return;
	if(event.node_A->getGOType() == GO_TYPE_CHAR) return;
	
	vector2 collisionVector = event.normal * event.distance;
	vector2 axis(0.f, 1.f);
	axis.normalize();
	
	float angle = event.normal.dot_product(axis);
	
	bool horizontal = false;
	if(angle < 0.2f && angle > -0.2f) horizontal = true;
	
	if(body->getFriction() && _collisionReady && !horizontal) {
		int vertex_id = body->findVertex(event.vertex);
		if(vertex_id >= 0) {
			bool side = false;
			Primitive* shape = body->getCollisionPrimitive();
			vector2 vel = shape->vertices[vertex_id] - shape->vertices_oldPos[vertex_id];
			float vel_length = vel.length();
			
			vector2 tmp(1.f, 0.f);
			tmp.normalize();
			float tmp2 = tmp.dot_product(event.normal);
	
			vector2 axis;
			if(tmp2 < 0)
				axis.set(event.normal[1], -event.normal[0]);	
			else {
				axis.set(-event.normal[1], event.normal[0]);
				if(vel[0] < 0) axis[0] *= -1.f;	
			}		
			
			if(vel_length > 0.1f) {
				vector2 friction = axis*(RAGDOLL_FRICTION_COEFF*0.5f);
				vel += friction;
				shape->vertices_oldPos[vertex_id] = shape->vertices[vertex_id] - vel;
			} else {
				shape->vertices_oldPos[vertex_id] = shape->vertices[vertex_id];
			}
		} else if(!horizontal) {
			int vertexA_id = body->findVertex(event.edge->vertexA);
			int vertexB_id = body->findVertex(event.edge->vertexB);
			if(vertexA_id >= 0 && vertexB_id >= 0) {
				bool side = false;
				Primitive* shape = body->getCollisionPrimitive();
				vector2 vel = shape->vertices[vertexA_id] - shape->vertices_oldPos[vertexA_id];
				float vel_length = vel.length();
				vector2 vel2 = shape->vertices[vertexB_id] - shape->vertices_oldPos[vertexB_id];
				float vel_length2 = vel2.length();
				
				event.normal *= -1.f;
				
				vector2 tmp(1.f, 0.f);
				tmp.normalize();
				float tmp2 = tmp.dot_product(event.normal);
				vector2 axis;
				if(tmp2 < -0.1f)
					axis.set(event.normal[1], -event.normal[0]);
					//if(vel[0] < 0) axis[0] *= -1.f;	
				else {
					axis.set(-event.normal[1], event.normal[0]);
					if(vel[0] < 0) axis[0] *= -1.f;	
				}		
				
				if(vel_length > 0.1f) {
					vector2 friction = axis*RAGDOLL_FRICTION_COEFF;
					vel += friction;
					vel2 += friction;
					shape->vertices_oldPos[vertexA_id] = shape->vertices[vertexA_id] - vel;
					shape->vertices_oldPos[vertexB_id] = shape->vertices[vertexB_id] - vel2;
				} else {
					shape->vertices_oldPos[vertexA_id] = shape->vertices[vertexA_id];
					shape->vertices_oldPos[vertexB_id] = shape->vertices[vertexB_id];
				}
			}
		}
	}	
}

void Ragdoll::init(vector2 position, char* texture_name) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId(texture_name);
	if(texid == -1) {
		_markForDeletion = true;
		return;	
	}
	sprite->setSprite(texid);
	sprite->setActive(true);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_QUAD;
	shape->vertices[0].set(0.f, 0.f);
	shape->vertices[1].set(size_x, 0.f);
	shape->vertices[2].set(size_x, size_y);
	shape->vertices[3].set(0.f, size_y);
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, position-size*0.5f, vector2(0.f, 0.f), vector2(0.f, 0.f), true);
	body->setSleep(false);
	body->setActive(true);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_RAGDOLL);
}
void Ragdoll::update(float frameDelta) {
	SceneNode::update(frameDelta);

	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	for(int p=0; p<(x->type); p++)
		x->vertices[p] = y->vertices[p];
}

bool Ragdoll::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_B) {
		if(event.node_B->getGOType() == GO_TYPE_WALLLASER && _collisionReady) {
			_markForDeletion = true;
			gfx->getSfxMgr()->spawnDisintegrationEffect(body->getCollisionPrimitive()->center);
			gfx->getSndMgr()->playSound("LASER", body->getCollisionPrimitive()->center);
			_collisionReady = false;
		} else if(event.node_B->getGOType() == GO_TYPE_SWITCH) {
			return true;	
		} else if(event.node_B->getGOType() == GO_TYPE_GRAVITYLIFT) {
			return true;	
		} else if(event.node_B->getGOType() == GO_TYPE_EXIT) {
			return true;	
		}
	}
	
	if(event.node_A->getGOType() == GO_TYPE_SWITCH) {
		return true;	
	} else if(event.node_A->getGOType() == GO_TYPE_GRAVITYLIFT) {
		return true;	
	} else if(event.node_A->getGOType() == GO_TYPE_EXIT) {
		return true;	
	}
	
	addFriction(event);
	
	return false;
}

Primitive* Ragdoll::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

// TheRock class

TheRock::TheRock(GfxMgr* gfx_n) {
	gfx = gfx_n;
	sprite = NULL;
	body = NULL;	
	exitState = false;
}
TheRock::~TheRock() {}

void TheRock::init(vector2 position) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("ROCK");
	sprite->setSprite(texid);
	sprite->setActive(true);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_CIRCLE;
	shape->radius = 20.f;
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, position, vector2(0.f, 0.f), vector2(0.f, 0.f), true);
	body->setSleep(false);
	body->setActive(true);
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_RAGDOLL);
}

void TheRock::update(float frameDelta) {
	SceneNode::update(frameDelta);
	
	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	
	x->vertices[0] = y->vertices[0] - (x->size * 0.5f);
	x->vertices[1].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
	x->vertices[2] = y->vertices[0] + (x->size * 0.5f);
	x->vertices[3].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
}

bool TheRock::receiveCollisionEvent(CollisionEvent event) {
	if(event.node_B)
	if(event.node_B->getGOType() == GO_TYPE_EXIT) {
		exitState = true;
		return true;
	} else if(event.node_B->getGOType() == GO_TYPE_SWITCH) {
		return true;	
	} else if(event.node_B->getGOType() == GO_TYPE_GRAVITYLIFT) {
		return true;
	}
	
	return false;
}

Primitive* TheRock::getCollisionPrimitive() {
	return body->getCollisionPrimitive();
}

bool TheRock::getExitState() {
	return exitState;	
}

// Playable Character class

PlayableCharacter::PlayableCharacter(GfxMgr* gfx_n) {
	gfx = gfx_n;
	_markForDeletion = 0;
	gfx->getInputHandler()->addReceiver(this);
	facingRight = true;
	inAir = true;
	objectPicked = false;
	pickedObjectPrimitive = NULL;
	alive = true;
	exitState = false;
}

PlayableCharacter::~PlayableCharacter() {
	gfx->getInputHandler()->deleteReceiver(this);	
}

void PlayableCharacter::pickObject() {
	if(objectPicked) return;
	
	// check space in front of the body for pickable objects (ragdolls)
	Primitive* space = new Primitive;
	Primitive* body_shape = getCollisionPrimitive();
	space->type = PRIM_TYPE_CIRCLE;
	space->radius = 8.f;
	space->vertices[0] = body_shape->center;
	if(facingRight) space->vertices[0] += vector2(body_shape->radius*2.f, 0.f);
	else space->vertices[0] -= vector2(body_shape->radius*2.f, 0.f);
	space->center = space->vertices[0];
	
	CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(space, false, true, body, GO_TYPE_RAGDOLL); // test only objects
	if(event.doIntersect) {
		if(event.node_B->getGOType() == GO_TYPE_RAGDOLL) {
			pickedObject = event.node_B;
			gfx->getPhysMgr()->unregisterSceneNode(pickedObject);
			pickedObjectPrimitive = pickedObject->getCollisionPrimitive();
			oldCallback = pickedObject->getCollisionCallback();
			objectPicked = true;
		}
	}
	
	delete space;
}

void PlayableCharacter::dropObject() {
	if(!objectPicked) return;
	CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(pickedObjectPrimitive, true, true, body);
	if(event.doIntersect) {
		if(event.node_B) {
			switch(event.node_B->getGOType()) {
			case GO_TYPE_SWITCH:
			case GO_TYPE_GRAVITYLIFT:
			case GO_TYPE_EXIT:
			case GO_TYPE_CHECKPOINT:
				break;
			default:
				return;
			}
		} else return;
	}
	gfx->getPhysMgr()->registerSceneNode(pickedObject);
//	pickedObject->registerCollisionCallback(oldCallback);
	pickedObject = NULL;
	objectPicked = false;
	pickedObjectPrimitive = NULL;	
}

void PlayableCharacter::updatePickedObject() {
	if(objectPicked) {
		Primitive* body_shape = body->getCollisionPrimitive();
		DrawPrimitive* drawprim = sprite->getPrimitive();
		vector2 center;
		if(facingRight)
			center = body_shape->center + vector2(drawprim->size[0], drawprim->size[1]/4.f+6.f);
		else 
			center = body_shape->center - vector2(drawprim->size[0], -drawprim->size[1]/4.f-6.f);
		for(int x=0; x<4; x++) {
			pickedObjectPrimitive->vertices[x] += center - pickedObjectPrimitive->center;
			pickedObjectPrimitive->vertices_oldPos[x] = pickedObjectPrimitive->vertices[x];
		}
		gfx->getPhysMgr()->calculatePolyCenter(pickedObjectPrimitive);
		
		CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(pickedObjectPrimitive, false, true, body, GO_TYPE_PLASMAPROJECTILE);

		if(event.doIntersect) {
			event.distance = 0.f;
			event.node_B->receiveCollisionEvent(event);	
		}
	//	if( == GO_TYPE_PLASMAPROJECTILE) event.node_A->receiveCollisionEvent(event);	
	}
}

void PlayableCharacter::init(vector2 position) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("HERO");
	sprite->setSprite(texid);
	sprite->setActive(true);
	sprite->setAnimationEndCallback(this);
	vector2 size = gfx->getTexMgr()->getSize(texid);
	float size_x = size[0];
	float size_y = size[1];
	Primitive* shape = new Primitive;
	shape->type = PRIM_TYPE_CIRCLE;
	shape->radius = 16.f;
	body = new CollisionResponseSceneNode(gfx);
	body->init(shape, position, vector2(0.f, 0.f), vector2(0.f, 0.f), true);
	body->setActive(true);
	
	gfx->addSceneNode(sprite, this);
	gfx->addSceneNode(body, this);
	body->registerCollisionCallback(this);
	body->setGOType(GO_TYPE_CHAR);
}

void PlayableCharacter::update(float frameDelta) {
	_collisionReady = true;
	updatePickedObject();
	SceneNode::update(frameDelta);
	DrawPrimitive* x = sprite->getPrimitive();
	Primitive* y = body->getCollisionPrimitive();
	
	vector2 velocity = y->vertices[0] - y->vertices_oldPos[0]; // Not real velocity! It's really velocity and some acceleration
									// useful only to compute animations
	
	if(inAir) {
		if(!objectPicked) {
			sprite->setFrame(14);
			sprite->stopAnimation();	
		} else {
			sprite->setAnimationLoop(30, 32, 9);	
		}
	} else if(fabs(velocity[0]) > 0.25f && fabs(velocity[1]) < 2.f) {
		if(!objectPicked) {
			sprite->setAnimationLoop(3, 11, 9);
		} else {
			sprite->setAnimationLoop(27, 29, 9);	
		}
	} else {
		if(!objectPicked) {
			sprite->setFrame(0);
			sprite->stopAnimation();
		} else {
			sprite->setAnimationLoop(24, 26, 6);	
		}	
	}
	

	if(facingRight) {
		x->vertices[0] = y->vertices[0] - (x->size * 0.5f);
		x->vertices[1].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
		x->vertices[2] = y->vertices[0] + (x->size * 0.5f);
		x->vertices[3].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);
	} else {
		x->vertices[1] = y->vertices[0] - (x->size * 0.5f);
		x->vertices[0].set(y->vertices[0][0]+x->size[0]*0.5f, y->vertices[0][1]-x->size[1]*0.5f);
		x->vertices[3] = y->vertices[0] + (x->size * 0.5f);
		x->vertices[2].set(y->vertices[0][0]-x->size[0]*0.5f, y->vertices[0][1]+x->size[1]*0.5f);	
	}
	
}

void PlayableCharacter::setPosition(vector2 position_n) {
		
}

void PlayableCharacter::receiveKeyEvent(list<KeyEvent> events) {
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		switch((*x).key) {
		case KEY_RIGHT:
			if((*x).state == GLFW_PRESS) {
				facingRight = true;
				body->getCollisionPrimitive()->vertices_oldPos[0][0] = body->getCollisionPrimitive()->vertices[0][0];
				body->getCollisionPrimitive()->vertices_oldPos[0] += vector2(-0.7f, 0.f);
				body->addForce(vector2(750.f, 0.f));
			} else {
				body->addForce(vector2(-750.f, 0.f));
			}
			break;
		case KEY_LEFT:
			if((*x).state == GLFW_PRESS) {
				facingRight = false;
				body->getCollisionPrimitive()->vertices_oldPos[0][0] = body->getCollisionPrimitive()->vertices[0][0];
				body->getCollisionPrimitive()->vertices_oldPos[0] += vector2(0.7f, 0.f);
				body->addForce(vector2(-750.f, 0.f));
			} else {
				body->addForce(vector2(750.f, 0.f));
			}
			break;
		case KEY_UP:
			if((*x).state == GLFW_PRESS) {
				if(jumpsLeft) {
					body->getCollisionPrimitive()->vertices_oldPos[0].set(body->getCollisionPrimitive()->vertices_oldPos[0][0], body->getCollisionPrimitive()->vertices[0][1]-4.5f);
					jumpsLeft -= 1;
					inAir = true;
					if(!objectPicked) {
						sprite->setAnimationOnce(12, 13, 13, 12);
					} else {
						sprite->setAnimationOnce(30, 32, 32, 9);	
					}
				}
			}
			break;
		case KEY_DOWN:
			if((*x).state == GLFW_PRESS && !inAir) {
				body->getCollisionPrimitive()->vertices_oldPos[0] = vector2(body->getCollisionPrimitive()->vertices[0][0], body->getCollisionPrimitive()->vertices_oldPos[0][1]);
			}	
			break;
		case KEY_SPACE:
			if((*x).state == GLFW_PRESS) {
				if(!objectPicked)
					pickObject();
				else dropObject();			
			}
			break;
		}	
	}
}

bool PlayableCharacter::receiveCollisionEvent(CollisionEvent event) {
	vector2 collisionVector = event.normal * event.distance;
	
	vector2 axis(0.f, 1.f);
	axis.normalize();	
	float angle = event.normal.dot_product(axis);
	if(angle > 0) {
		jumpsLeft = 2;
		inAir = false;
	}
	
	if(event.node_B) {
		if(event.node_B->getGOType() == GO_TYPE_SWITCH) {
			return true;	
		} else if(event.node_B->getGOType() == GO_TYPE_PLASMAPROJECTILE) {
			alive = false;
			return true;	
		} else if(event.node_B->getGOType() == GO_TYPE_EXIT) {
			exitState = true;
			return true;
		} else if(event.node_B->getGOType() == GO_TYPE_GRAVITYLIFT) {
			jumpsLeft = 0;
			inAir = true;
			return true; 
		} else if(event.node_B->getGOType() == GO_TYPE_CHECKPOINT) {
			return true;	
		} else if(event.node_B->getGOType() == GO_TYPE_WALLLASER) {
			if(event.distance < 2.f) {
				alive = false;
				gfx->getSfxMgr()->spawnDisintegrationEffect(getCollisionPrimitive()->center);
			}
		} else if(event.node_B->getGOType() == GO_TYPE_MINE) {
			alive = false;	
		}
	} 
	
	if(event.node_A) {
		if(event.node_A->getGOType() == GO_TYPE_PLASMAPROJECTILE) {
			alive = false;
			return true;	
		}
	}

	bool horizontal = false;
	if(angle < 0.2f && angle > -0.2f) horizontal = true;

	if(body->getFriction() && _collisionReady && !horizontal) {
		bool side = false;
		Primitive* shape = body->getCollisionPrimitive();
		vector2 vel = shape->vertices[0] - shape->vertices_oldPos[0];
		float vel_length = vel.length();
	
		vector2 tmp(-1.f, 0.f);
		tmp.normalize();
		float tmp2 = tmp.dot_product(event.normal);
		
		vector2 axis;
		if(tmp2 < 0)
			axis.set(-event.normal[1], event.normal[0]);
		//	if(vel[0] < 0) axis *= -1.f;
		else {
			axis.set(event.normal[1], -event.normal[0]);
			if(vel[0] > 0) axis *= -1.f;
		}
				
				
		if(vel_length > 0.25f) {
			vector2 friction = axis*HERO_FRICTION_COEFF;
			vel += friction;
			shape->vertices_oldPos[0] = shape->vertices[0] - vel;
		} else {
			shape->vertices_oldPos[0] = shape->vertices[0];
		}
		_collisionReady = false;
	}
	
	return false;
}

void PlayableCharacter::receiveAnimationEndSignal(AnimatedSpriteSceneNode* caller) {
	if(inAir) {
		sprite->setFrame(14);
		sprite->stopAnimation();	
	}	
}

Primitive* PlayableCharacter::getCollisionPrimitive() {
	return body->getCollisionPrimitive();	
}

bool PlayableCharacter::isAlive() {
	return alive;	
}

bool PlayableCharacter::getExitState() {
	return exitState;	
}
