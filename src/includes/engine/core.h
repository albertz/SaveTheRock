/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __CORE_H
#define __CORE_H

class GfxMgr;
class UIMgr;
class CameraSceneNode;
class TransfSceneNode;
class AnimatedSpriteSceneNode;
class LevelSceneNode;
struct CollisionEvent;
class PhysMgr;
class UIImage;
class Game;

enum ORIENTATION {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3	
};

enum GAME_OBJECT_TYPE {
	GO_TYPE_NONE,
	GO_TYPE_ALL,
	GO_TYPE_RAGDOLL,
	GO_TYPE_ROCK,
	GO_TYPE_CHAR,
	GO_TYPE_WALLLASER,
	GO_TYPE_ROBOTGUARD,
	GO_TYPE_PLASMAPROJECTILE,
	GO_TYPE_SWITCH,
	GO_TYPE_PRESSUREPAD,
	GO_TYPE_MINE,
	GO_TYPE_LASERDOOR,
	GO_TYPE_EXIT,
	GO_TYPE_TEXT,
	GO_TYPE_GRAVITYLIFT,
	GO_TYPE_CHECKPOINT
};

enum KEY {
	KEY_UP = 0,
	KEY_DOWN = 1,
	KEY_LEFT = 2,
	KEY_RIGHT = 3,
	KEY_LEFT_MB = 4,
	KEY_RIGHT_MB = 5,
	KEY_SPACE = 6,
	KEY_ENTER = 7,
	KEY_BACKSPACE = 8,
	KEY_ESC = 9,
	KEY_CHR = 10 // character
};

enum KEY_STATE {
	KEY_STATE_PRESSED = 0,
	KEY_STATE_RELEASED = 1	
};

struct KeyEvent {
	int key;
	int chr;
	int state;
};	

enum PRIM_TYPE {
	// name     ---    number of vertices
	PRIM_TYPE_CIRCLE = 0,
	PRIM_TYPE_TRIANGLE = 3,
	PRIM_TYPE_QUAD = 4
};

struct Edge {
	vector2* vertexA;
	vector2* vertexB;
	float def_length; // default length
	Primitive* parent;
};


struct Primitive {
	PRIM_TYPE type;
	vector2 vertices[4];
	vector2 vertices_oldPos[4];
	vector2 vertices_accel[4];
	Edge edges[4]; // physical constraints
	Edge constraints[4]; // additional physical constraints
	int constraints_n;
	int grid_cell_x;
	int grid_cell_y;
	
	vector2 center; // center of bounding circle
	float radius; // radius of bounding circle
	int border;
};

struct DrawPrimitive {
	PRIM_TYPE type;
	vector2 size;
	vector2 tex_size;
	vector2 vertices[4];
	color vertices_colors[4];
	vector2 center; // center of bounding circle
	float radius; // radius of bounding_circle	
};

class aabb2 {
	// axis-aligned 2d bounding box
	public:
	vector2 bottom_left;
	vector2 top_right;
	
	bool checkPointInside(vector2 point);
};

enum SCENE_NODE_TYPE {
	SN_TYPE_SCENE,
	SN_TYPE_CAMERA,
	SN_TYPE_TRIANGLE,
	SN_TYPE_QUAD,
	SN_TYPE_LEVEL,
	SN_TYPE_SPRITE,
	SN_TYPE_LEDITOR,
	SN_TYPE_COLLISION,
	SN_TYPE_WIDGET
};

class SceneNode {
	std::list<SceneNode*>* children;
	
	public:
	char* name;
		
	bool _isActive;
	bool _isSleeping;
	bool _markForDeletion;
	bool _collisionReady;
		
	SceneNode* parent;
	SCENE_NODE_TYPE type;
	GAME_OBJECT_TYPE go_type;
	
	GfxMgr* gfx;
	renderer* glrenderer;
	
	SceneNode();
	virtual ~SceneNode();
	
	void setName(const char* newName);
	char* getName();

	void setActive(bool state);
	bool isActive();
	void setSleep(bool state);
	bool isSleeping();

	void deleteChildren();
	void deleteChild(SceneNode* node);
	
	virtual void update(float);
	void drawSelf();
	virtual std::list<Primitive*> getGeometry(aabb2 bounds); // returns geometry inside specified bounds, if any
	virtual Primitive* getCollisionPrimitive();
	virtual DrawPrimitive* getPrimitive();
	virtual void receiveKeyEvent(std::list<KeyEvent> events);
	virtual bool receiveCollisionEvent(CollisionEvent event);
	virtual void receiveAnimationEndSignal(AnimatedSpriteSceneNode* caller);
	virtual void setSwitch(bool state);
	virtual bool getSwitch();
	GAME_OBJECT_TYPE getGOType();
	void setGOType(GAME_OBJECT_TYPE go_type_n);
	
	void addChild(SceneNode*);
	std::list<SceneNode*>* getChildren();
	int getChildrenNum();
	int getAbsoluteChildrenNum();
	TransfSceneNode* getTransformation();
};

class ContainerSceneNode : public SceneNode {
	public:
	ContainerSceneNode(GfxMgr* gfx_n);
	~ContainerSceneNode();
	void update(float frameDelta);
};

class InputHandler {
	std::list<SceneNode*>* receivers;
	/*
	Nodes can take full control of input using takeControl(); and release it using releaseControl();
	During this time other receivers are not polled.
	If a second node requests full control it will be denied until release.
	*/
	SceneNode* controlNode;
	
	public:
	InputHandler();
	void init();
	void reinit();
	void addReceiver(SceneNode* receiver);
	void deleteReceiver(SceneNode* receiver);
	void takeControl(SceneNode* node);
	void releaseControl(SceneNode* node);
	void update(float frameDelta);
};

class TexMgr {
	GfxMgr* gfx;
	
	unsigned int** textures;
	int* textures_frames_n;
	vector2* textures_sizes;
	vector2* textures_real_sizes;
	char** textures_names;
	int textures_n;
	
	public:
	/* TexMgr provides a high-level interface for loading textures */
	TexMgr(GfxMgr* gfx_n);
	void init();
	void addTexture(const char* name, const char* filepath);
	
	int getId(const char* name);
	unsigned int* getTexture(int id);
	vector2 getSize(int id);
	vector2 getTexSize(int id);
	int getFramesN(int id);
};

class SfxMgr : public SceneNode{
	/* SfxMgr creates special effects */
	
	int effects_n;
	
	public:
	SfxMgr(GfxMgr* gfx_n);
	void init();
	void update(float frameDelta);
	void spawnDisintegrationEffect(vector2 position);
	void spawnExplosionEffect(vector2 position);
	void receiveAnimationEndSignal(AnimatedSpriteSceneNode* sprite);
};

class GfxMgr {
	/* GfxMgr keeps track of the scene graph, drawing, controller input, animation etc. */
	public:
	renderer* glrenderer;
	SceneNode* scene; // scene graph
	CameraSceneNode* camera; // camera scene node (first object in graph, created by GfxMgr)
				// all objects that are affected by camera view should be added as its children
	LevelSceneNode* level; // level scene node
	
	Game* game;
	
	InputHandler* input;
	PhysMgr* physics;
	UIMgr* ui;
	Filesystem* filesystem;
	TexMgr* tex;
	SfxMgr* sfx;
	SndMgr* snd;
	
	DrawPrimitive* cursor;
	int cursor_texture_id;
	unsigned int* cursor_textures;
	bool drawCursor;
	vector2 absoluteMousePos;
	vector2 currentMousePos;
	void C_initCursor();
	void C_updateCursor(float frameDelta);
	
	GfxMgr(Game* game_n);
	void init(int w_width, int w_height, bool fullscreen, bool vsync, int fsaa);
	void recomputeBackground();
	void frameStart();
	void frameEnd();
	void setWindowTitle(char* title);
	void updateScene(float frameDelta);
	void addSceneNode(SceneNode* node, SceneNode* parent=NULL);
	void deleteSceneNode(SceneNode* node);
//	void __updateMousePos();
	void setCursor(bool visible=true);
	void updateMousePos(float frameDelta);
	vector2 getMousePos(bool absolute=false);
	void reinitLevel();
	renderer* getRenderer();
	SceneNode* getScene();
	CameraSceneNode* getCamera();
	InputHandler* getInputHandler();
	PhysMgr* getPhysMgr();
	LevelSceneNode* getLevel();
	UIMgr* getUI();
	Filesystem* getFilesystem();
	TexMgr* getTexMgr();
	SfxMgr* getSfxMgr();
	SndMgr* getSndMgr();
	Game* getGame();
	
	int getObjectsNum();
};


class CameraSceneNode : public SceneNode {
	// Camera Scene Node - represents a rectangle camera. Currently, you can have only one.
	bool keyboardControl;
	Primitive* followPrimitive;
	
	vector2 position;
	
	public:
	bool isMoving;
	vector2 direction;
	float speed;
	float currentSpeed;
		
	CameraSceneNode(GfxMgr*);
	void setKeyboardControl(bool state);
	void update(float frameDelta);
	void _move(float frameDelta);
	void _follow(float frameDelta);
	void setMovement(vector2 direction, float speed);
	void setFollow(Primitive* primitive);
	void startMovement();
	void stopMovement();
	void receiveKeyEvent(std::list<KeyEvent> events);
	void setTranslation(vector2 translation_n);
	vector2 getAbsoluteTranslation();
};
#endif
