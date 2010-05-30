/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

AnimatedSpriteSceneNode::AnimatedSpriteSceneNode(GfxMgr* gfx_n) {
	type = SN_TYPE_SPRITE;
	gfx = gfx_n;
	currentTexture = 0;
	currentFrame = 0;
	quad = new DrawPrimitive;
	quad->type = PRIM_TYPE_QUAD;
	_isActive = false;
	_isSleeping = false;
	isAnimRunning = false;
	isAnimLooping = false;
	isAnimForward = true;
	restFrame = 0;
	_markForDeletion = 0;
	animationCallback = NULL;
}

AnimatedSpriteSceneNode::~AnimatedSpriteSceneNode() {
	if(quad) delete quad;
}

bool AnimatedSpriteSceneNode::setSprite(int texture_id) {
	if(texture_id < 0) return false;
	frames_n = gfx->getTexMgr()->getFramesN(texture_id);
	textures = gfx->getTexMgr()->getTexture(texture_id);
	size = gfx->getTexMgr()->getSize(texture_id);
	tex_size = gfx->getTexMgr()->getTexSize(texture_id);

	currentTexture = textures[0];
//	gfx->getPhysMgr()->registerSceneNode(this);
	quad->size = size;
	quad->tex_size = tex_size;
	quad->vertices[0] = vector2(0, 0);
	quad->vertices[1] = vector2(size[0], 0);
	quad->vertices[2] = vector2(size[0], size[1]);
	quad->vertices[3] = vector2(0, size[1]);
	quad->vertices_colors[0][3] = 1.f;
}

void AnimatedSpriteSceneNode::setFrame(int frame_n) {
	currentFrame = frame_n;	
}

void AnimatedSpriteSceneNode::setAnimationLoop(int start_frame, int end_frame, float speed) {
	if(start_frame == startFrame && end_frame == endFrame && speed == animSpeed && isAnimRunning) return;
	startFrame = start_frame;
	endFrame = end_frame;
	if(endFrame < startFrame) isAnimForward = false;
	else isAnimForward = true;
	animSpeed = speed;
	currentFrame = startFrame;
	currentTexture = textures[currentFrame];
	timeToNextFrame = 1.f/animSpeed;
	isAnimLooping = true;
	startAnimation();
}

void AnimatedSpriteSceneNode::setAnimationOnce(int start_frame, int end_frame, int restFrame, float speed) {
	startFrame = start_frame;
	endFrame = end_frame;
	if(endFrame < startFrame) isAnimForward = false;
	else isAnimForward = true;
	animSpeed = speed;
	currentFrame = startFrame;
	currentTexture = textures[currentFrame];
	timeToNextFrame = 1.f/animSpeed;	
	isAnimLooping = false;
	startAnimation();
}

void AnimatedSpriteSceneNode::startAnimation() {
	isAnimRunning = true;
}

void AnimatedSpriteSceneNode::stopAnimation() {
	isAnimRunning = false;
}

bool AnimatedSpriteSceneNode::isAnimationRunning() {
	return isAnimRunning;	
}

void AnimatedSpriteSceneNode::animateSelf(float frameDelta) {
		if(isAnimRunning) {
			if(timeToNextFrame <= 0.f) {
				if(currentFrame != endFrame) {
					if(isAnimForward)
						currentFrame++;
					else currentFrame--;
					timeToNextFrame = 1.f/animSpeed;
				} else if(isAnimLooping) {
					currentFrame = startFrame;
					if(animationCallback) {
						animationCallback->receiveAnimationEndSignal(this);	
					}
					timeToNextFrame = 1.f/animSpeed;
				} else {
					currentFrame = restFrame;
					stopAnimation();
					if(animationCallback)
						animationCallback->receiveAnimationEndSignal(this);
					timeToNextFrame = 1.f/animSpeed;	
				}
			} else timeToNextFrame -= frameDelta;
		}
		currentTexture = textures[currentFrame];
}

void AnimatedSpriteSceneNode::update(float frameDelta) {
	if(_isActive) {
		animateSelf(frameDelta);
		gfx->getRenderer()->drawSprite(quad, currentTexture);
		SceneNode::update(frameDelta);
	}
}

void AnimatedSpriteSceneNode::setAnimationEndCallback(SceneNode* node) {
	if(node)
		animationCallback = node;	
}

DrawPrimitive* AnimatedSpriteSceneNode::getPrimitive() {
	gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	return quad;
}

// CollisionResponseSceneNode methods

CollisionResponseSceneNode::CollisionResponseSceneNode(GfxMgr* gfx_n) {
	gfx = gfx_n;
	type = SN_TYPE_COLLISION;
	netForce = vector2(0.f, GRAVITY_Y);
	callbackNode = false;
	shape = new Primitive;
	isStatic = false;
	_markForDeletion = 0;
	doGravity = true;
}

CollisionResponseSceneNode::~CollisionResponseSceneNode() {
	gfx->getPhysMgr()->unregisterSceneNode(this);
}

void CollisionResponseSceneNode::setQuadEdgesAndConstraints(Primitive* quad) {
	gfx->getPhysMgr()->setQuadEdgesAndConstraints(quad);
}

void CollisionResponseSceneNode::init(Primitive* shape_n, vector2 initPosition, vector2 initVelocity, vector2 initAcceleration, bool friction) {
	doFriction = friction;

	if(shape_n->type == PRIM_TYPE_QUAD) {
		shape->type = PRIM_TYPE_QUAD;
		for(int i=0; i<4; i++) {
			shape->vertices[i] = shape_n->vertices[i] + initPosition;
			shape->vertices_oldPos[i] = shape->vertices[i] - initVelocity;
			shape->vertices_accel[i] = initAcceleration;
		}
		setQuadEdgesAndConstraints(shape);
		gfx->getRenderer()->findPrimitiveBoundingCircle(shape);
	} else if(shape_n->type == PRIM_TYPE_CIRCLE) {
		shape->type = PRIM_TYPE_CIRCLE;
		shape->vertices[0] = initPosition;
		shape->center = shape->vertices[0];
		shape->vertices_oldPos[0] = initPosition - initVelocity;
		shape->vertices_accel[0] = initAcceleration;
		shape->radius = shape_n->radius;
	}
	gfx->getPhysMgr()->registerSceneNode(this);
}

void CollisionResponseSceneNode::setPosition(vector2 position_n) {
		
}

void CollisionResponseSceneNode::update(float frameDelta) {
	doFriction = true;
	SceneNode::update(frameDelta);
//	gfx->getRenderer()->drawCircle(shape->vertices[0], shape->radius);
}

void CollisionResponseSceneNode::setStatic(bool state) {
	isStatic = state;	
}

bool CollisionResponseSceneNode::checkStatic() {
	return isStatic;	
}

bool CollisionResponseSceneNode::receiveCollisionEvent(CollisionEvent event) {
	if(callbackNode)
		if(callbackNode->receiveCollisionEvent(event)) return true;
	return false;
}

void CollisionResponseSceneNode::registerCollisionCallback(SceneNode* node) {
	callbackNode = node;	
}

SceneNode* CollisionResponseSceneNode::getCollisionCallback() {
	return callbackNode;	
}

Primitive* CollisionResponseSceneNode::getCollisionPrimitive() {
	return shape;
}

int CollisionResponseSceneNode::findVertex(vector2* vertex) {
	for(int x=0; x<shape->type; x++) {
		if(&(shape->vertices[x]) == vertex)
			return x;
	}
	return -1;
}

void CollisionResponseSceneNode::setGravity(bool state) {
	if(state && !doGravity) {
		netForce += vector2(0.f, GRAVITY_Y);
		doGravity = true;
	} else if(!state && doGravity) {
		netForce -= vector2(0.f, GRAVITY_Y);
		doGravity = false;
	}
}

void CollisionResponseSceneNode::addForce(vector2 accel) {
	netForce += accel;	
}

void CollisionResponseSceneNode::setForce(vector2 force) {
	netForce = force;
	if(doGravity) netForce += vector2(0.f, GRAVITY_Y);
}

void CollisionResponseSceneNode::stopMotion() {
	netForce = vector2();
	for(int x=0; x<shape->type; x++) shape->vertices_oldPos[x] = shape->vertices[x];
	if(shape->type == 0) shape->vertices_oldPos[0] = shape->vertices[0];	
}

vector2 CollisionResponseSceneNode::getNetForce() {
	return netForce;
}

bool CollisionResponseSceneNode::getFriction() {
	return doFriction;
}

bool CollisionResponseSceneNode::inView() {
	aabb2 view;
	view.bottom_left = gfx->getCamera()->getAbsoluteTranslation();
	view.top_right = view.bottom_left + gfx->getRenderer()->getWindowParams();
	vector2 primcenter = getCollisionPrimitive()->center;
	if(primcenter[0] > view.top_right[0] + PHYSICS_SLEEP_EPSILON || primcenter[0] < view.bottom_left[0] - PHYSICS_SLEEP_EPSILON)
		return false;
	if(primcenter[1] > view.top_right[1] + PHYSICS_SLEEP_EPSILON || primcenter[1] < view.bottom_left[1] - PHYSICS_SLEEP_EPSILON)
		return false;
	return true;	
}
