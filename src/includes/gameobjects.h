/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

class Switch : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	SceneNode* target;
	bool state;
	
	int switch_times;
	
	float timer;
	
	public:
	Switch(GfxMgr* gfx_n);
	~Switch();
	void init(vector2 coords, int switch_times_n, SceneNode* target_n);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class Checkpoint : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	bool state;
	
	int id;
	
	public:
	Checkpoint(GfxMgr* gfx_n);
	~Checkpoint();
	void init(vector2 coords, int id_n);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class PressurePad : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	SceneNode* target;
	bool state;
	float timer;
	float timer2;
	bool obj_state;
	
	public:
	PressurePad(GfxMgr* gfx_n);
	~PressurePad();
	void init(vector2 coords, SceneNode* target_n);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class LevelExit : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	
	public:
	LevelExit(GfxMgr* gfx_n);
	~LevelExit();
	void init(vector2 coords);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();	
};

class Mine : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	SceneNode* target;
	
	public:
	Mine(GfxMgr* gfx_n);
	~Mine();
	void init(vector2 coords);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class GravityLift : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;

	float force;
	
	int soundsCount;
	float soundsTimer;
	
	public:
	GravityLift(GfxMgr* gfx_n);
	~GravityLift();
	void init(vector2 coords, float force_n);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();	
};

class WallLaser : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	AnimatedSpriteSceneNode* beam;
	CollisionResponseSceneNode* beamCollision;
	Primitive* collisionPrimitive;
	
	ORIENTATION orientation;
	
	bool type; // 0 - laser door, 1 - laser weapon
	
	float beamLength;
	bool beamState;
	float beamInterval;
	float timeToSwitch;
	int beamAnimationStart;
	int beamAnimationEnd;
	
	void findBeamLength();
	
	public:
	WallLaser(GfxMgr* gfx_n);
	~WallLaser();
	void init(vector2 coords, bool type_n, float interval);
	void setPosition(int tile_x, int tile_y, ORIENTATION orientation);
	void update(float frameDelta);
	void setSwitch(bool state);
	bool getSwitch();
	void switchBeam();
	void startBeam();
	void stopBeam();
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class RobotGuard : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	bool doPatrol;
	vector2 patrol_startpos, patrol_endpos;
	bool patrol_direction;
	bool patrol_switch;
	float moveSpeed;
	bool doFire;
	float firingSpeed;
	float projectileSpeed;
	float fireTimer;
	bool state;
	
	public:
	RobotGuard(GfxMgr* gfx_n);
	~RobotGuard();
	void update(float frameDelta);
	void setSwitch(bool state);
	bool getSwitch();
	void init(vector2 coords);
	void setPatrol(bool state_n, vector2 startpos=vector2(), vector2 endpos=vector2(), float moveSpeed_n=0.f);
	void setFire(bool state_n, float firingSpeed_n=0.f, float projectileSpeed_n=0.f);
	void fireProjectile(vector2 direction);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class PlasmaProjectile : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	
	CollisionResponseSceneNode* parent_body;
	
	float moveSpeed;
	vector2 direction;
	float lifeTime;
	
	public:
	PlasmaProjectile(GfxMgr* gfx_n);
	~PlasmaProjectile();
	void update(float frameDelta);
	void init(vector2 coords, float moveSpeed_n, vector2 direction_n, CollisionResponseSceneNode* parent_body_n);
	CollisionResponseSceneNode* getParentBody();
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class Ragdoll : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	
	void addFriction(CollisionEvent event);
	
	public:
	Ragdoll(GfxMgr* gfx_n);
	~Ragdoll();
	void init(vector2 position, const std::string& texture_name);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
};

class TheRock : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	
	bool exitState;
	
	public:
	TheRock(GfxMgr* gfx_n);
	~TheRock();
	void init(vector2 position);
	void update(float frameDelta);
	bool receiveCollisionEvent(CollisionEvent event);
	Primitive* getCollisionPrimitive();
	bool getExitState();
};

class PlayableCharacter : public SceneNode {
	CollisionResponseSceneNode* body;
	AnimatedSpriteSceneNode* sprite;
	bool facingRight;
	bool inAir;
	
	bool alive;
	bool exitState;
	
	bool objectPicked;
	Primitive* pickedObjectPrimitive;
	
	int jumpsLeft;
	
	CollisionResponseSceneNode* pickedObject;
	SceneNode* oldCallback;
	void pickObject();
	void dropObject();
	void updatePickedObject();
	
	public:
	PlayableCharacter(GfxMgr* gfx_n);
	~PlayableCharacter();
	void init(vector2 position);
	void update(float frameDelta);
	void setPosition(vector2 position);
	void receiveKeyEvent(std::list<KeyEvent> events);
	bool receiveCollisionEvent(CollisionEvent event);
	void receiveAnimationEndSignal(AnimatedSpriteSceneNode* caller);
	Primitive* getCollisionPrimitive();
	bool isAlive();
	bool getExitState();
};


#endif
