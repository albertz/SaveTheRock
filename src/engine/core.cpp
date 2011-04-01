/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */

#include "engine/headers.h"

using namespace std;

// aabb2 class methods
bool aabb2::checkPointInside(vector2 point) {
	if(point[0] > bottom_left[0] && point[1] > bottom_left[1])
		if(point[0] < top_right[0] && point[1] < top_right[1])
			return true;
	return false;	
}

// SceneNode class methods

SceneNode::SceneNode() {
	children = new list<SceneNode*>;
	_markForDeletion = false;
	_collisionReady = false;
	go_type = GO_TYPE_NONE;	
}	

SceneNode::~SceneNode() {
	deleteChildren();
	delete children;
}

void SceneNode::setName(const std::string& newName) {
	name = newName;
}

std::string SceneNode::getName() {
	return name;
}

void SceneNode::setActive(bool state) {
	_isActive = state;
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		(*x)->setActive(state);	
	}
}

bool SceneNode::isActive() {
	return _isActive;
}

void SceneNode::setSleep(bool state) {
	_isSleeping = state;
}

bool SceneNode::isSleeping() {
	return _isSleeping;
}

void SceneNode::deleteChildren() {	
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		delete (*x);
	}

	children->clear();
}

void SceneNode::deleteChild(SceneNode* node) {
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		if(*x == node) {
			children->erase(x);
			delete *x;
			return;
		}
	}
}

void SceneNode::update(float frameDelta) {
	_collisionReady = true;
	if(children->size() <= 0) return;
	
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		(*x)->update(frameDelta);
	}
	
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		if((*x)->_markForDeletion) {
			deleteChild(*x);
			return;
		}
	}
}

void SceneNode::drawSelf() {	
}

list<Primitive*> SceneNode::getGeometry(aabb2 bounds) {
	list<Primitive*> x;
	return x;
}

Primitive* SceneNode::getCollisionPrimitive() {
	assert("Fatal error: SceneNode::getCollisionPrimitive() - this function should be overloaded!" == 0);
}

DrawPrimitive* SceneNode::getPrimitive() {
	assert("Fatal error: SceneNode::getPrimitive() - this function should be overloaded!" == 0);
}

void SceneNode::receiveKeyEvent(list<KeyEvent> events) {}
bool SceneNode::receiveCollisionEvent(CollisionEvent event) { return false; }
void SceneNode::receiveAnimationEndSignal(AnimatedSpriteSceneNode* caller) {}
void SceneNode::setSwitch(bool state) {}
bool SceneNode::getSwitch() {return false;}

GAME_OBJECT_TYPE SceneNode::getGOType() {
	return go_type;	
}

void SceneNode::setGOType(GAME_OBJECT_TYPE go_type_n) {
	go_type = go_type_n;
}

void SceneNode::addChild(SceneNode* node) {	
	children->push_back(node);
}

list<SceneNode*>* SceneNode::getChildren() {
	return children;	
}

int SceneNode::getChildrenNum() {
	return children->size();	
}

int SceneNode::getAbsoluteChildrenNum() {
	int n = children->size();
	for(list<SceneNode*>::iterator x = children->begin(); x != children->end(); x++) {
		n += (*x)->getAbsoluteChildrenNum();
	}
	return n;
}

// ContainerSceneNode methods

ContainerSceneNode::ContainerSceneNode(GfxMgr* gfx_n) {
	gfx = gfx_n;	
}

ContainerSceneNode::~ContainerSceneNode() {
	SceneNode::deleteChildren();
}

void ContainerSceneNode::update(float frameDelta) {
	SceneNode::update(frameDelta);	
}

// INPUT HANDLER ROUTINES
static list<KeyEvent> keylist;

void GLFWCALL glfw_callback(int key, int state) {
	KeyEvent newEvent;
	switch(key) {
	case GLFW_KEY_UP:
		newEvent.key = KEY_UP;
		newEvent.state = state;	
		break;
	case GLFW_KEY_DOWN:
		newEvent.key = KEY_DOWN;
		newEvent.state = state;
		break;
	case GLFW_KEY_LEFT:
		newEvent.key = KEY_LEFT;
		newEvent.state = state;
		break;
	case GLFW_KEY_RIGHT:
		newEvent.key = KEY_RIGHT;
		newEvent.state = state;
		break;
	case GLFW_KEY_ESC:
		newEvent.key = KEY_ESC;
		newEvent.state = state;
		break;
	case GLFW_KEY_SPACE:
		newEvent.key = KEY_SPACE;
		newEvent.state = state;
		break;
	case GLFW_KEY_ENTER:
		newEvent.key = KEY_ENTER;
		newEvent.state = state;
		break;
	case GLFW_KEY_BACKSPACE:
		newEvent.key = KEY_BACKSPACE;
		newEvent.state = state;
		break;
	default:
		newEvent.key = KEY_CHR;
		newEvent.state = state;
		newEvent.chr = key;
		break;
	}
	keylist.push_back(newEvent);
}

void GLFWCALL glfw_mouse_callback(int button, int state) {
	KeyEvent newEvent;
	switch(button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			newEvent.key = KEY_LEFT_MB;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			newEvent.key = KEY_RIGHT_MB;
			break;
	}
	newEvent.state = state;
	keylist.push_back(newEvent);
}

// Input handler
InputHandler::InputHandler() {
	controlNode = NULL;
	receivers = NULL;
	receivers = new list<SceneNode*>;
}

void InputHandler::init() {
	glfwSetKeyCallback(glfw_callback);
	glfwSetMouseButtonCallback(glfw_mouse_callback);
}

void InputHandler::reinit() {
	receivers->clear();
}

void InputHandler::addReceiver(SceneNode* receiver) {
	for(list<SceneNode*>::iterator x = receivers->begin(); x != receivers->end(); x++) {
		if(*x == receiver) {
			return;
		}	
	}
	receivers->push_back(receiver);	
}

void InputHandler::deleteReceiver(SceneNode* receiver) {
	for(list<SceneNode*>::iterator x = receivers->begin(); x != receivers->end(); x++) {
		if(*x == receiver) {
			receivers->erase(x);
			break;	
		}	
	}	
}

void InputHandler::takeControl(SceneNode* node) {
	if(controlNode == NULL) {
		controlNode = node;	
		keylist.clear();
	}
}

void InputHandler::releaseControl(SceneNode* node) {
	if(node == controlNode) {
		controlNode = NULL;	
		keylist.clear();
	}
}

void InputHandler::update(float frameDelta) {
	while(keylist.size() > 8) {
		keylist.pop_front();	
	}

	if(controlNode != NULL) {
		controlNode->receiveKeyEvent(keylist);
	} else {
		for(list<SceneNode*>::iterator x = receivers->begin(); x != receivers->end(); x++)
			(*x)->receiveKeyEvent(keylist);
	}
	keylist.clear();
}

// TexMgr class methods

TexMgr::TexMgr(GfxMgr* gfx_n) {
	gfx = gfx_n;
	textures = new unsigned int*[MAX_TEXTURES];
	textures_frames_n = new int[MAX_TEXTURES];
	textures_sizes = new vector2[MAX_TEXTURES];
	textures_real_sizes = new vector2[MAX_TEXTURES];
	textures_names = new std::string[MAX_TEXTURES];
	textures_n = 0;
}

void TexMgr::addTexture(const std::string& name, const std::string& filepath) {
	int frames_n, size_x, size_y, real_size_x, real_size_y;
	textures[textures_n] = gfx->getRenderer()->loadTexturesFromFile(filepath, &frames_n, &size_x, &size_y, &real_size_x, &real_size_y);
	textures_sizes[textures_n] = vector2(size_x, size_y);
	textures_real_sizes[textures_n] = vector2(real_size_x, real_size_y);
	textures_frames_n[textures_n] = frames_n;
	textures_names[textures_n] = name;
	textures_n++;	
}

void TexMgr::init() {
	/* load hard coded textures*/

	addTexture("CRATE", "../media/sprites/crate32x32.anim");
	addTexture("CRATE16X16", "../media/sprites/crate16x16.anim");
	addTexture("HERO" , "../media/sprites/hero.anim");
	addTexture("ROCK", "../media/sprites/rock.anim");
	addTexture("WALLLASER", "../media/sprites/walllaser.anim");
	addTexture("LASERBEAM", "../media/sprites/laserbeam.anim");
	addTexture("ROBOTGUARD", "../media/sprites/robotguard.anim");
	addTexture("PLASMAPROJECTILE", "../media/sprites/plasmaprojectile.anim");
	addTexture("DISINTEGRATION", "../media/sprites/disintegration.anim");
	addTexture("EXPLOSION", "../media/sprites/explosion.anim");
	addTexture("SWITCH", "../media/sprites/switch.anim");
	addTexture("PRESSUREPAD", "../media/sprites/pressurepad.anim");
	addTexture("MINE", "../media/sprites/mine.anim");
	addTexture("LASERDOOR", "../media/sprites/laserdoor.anim");
	addTexture("WALLDOOR", "../media/sprites/walldoor.anim");
	addTexture("GRAVITYLIFT", "../media/sprites/gravitylift.anim");
	addTexture("EXIT", "../media/sprites/exit_teleporter.anim");
	addTexture("LOGOS", "../media/sprites/logos.anim");
	addTexture("GAMELOGO", "../media/sprites/gamelogo.anim");
	addTexture("CURSOR", "../media/sprites/cursor.anim");
	addTexture("CHECKPOINT", "../media/sprites/checkpoint.anim");

}

int TexMgr::getId(const std::string& name) {
	for(int x=0; x < textures_n; x++) {
		if(textures_names[x] == name) {
			return x;
		}
	}
	return -1;
}

unsigned int* TexMgr::getTexture(int id) {
	if(id < 0 && id >= textures_n) return NULL;
	return textures[id];	
}
vector2 TexMgr::getSize(int id) {
	if(id < 0 && id >= textures_n) return vector2();
	return textures_real_sizes[id];	
}

vector2 TexMgr::getTexSize(int id) {
	if(id < 0 && id >= textures_n) return vector2();
	return textures_sizes[id];
}
int TexMgr::getFramesN(int id) {
	if(id < 0 && id >= textures_n) return 0;
	return textures_frames_n[id];	
}

// SfxMgr class methods

SfxMgr::SfxMgr(GfxMgr* gfx_n) {
	gfx = gfx_n;	
	effects_n = 0;
}

void SfxMgr::init() {
	
}

void SfxMgr::update(float frameDelta) {
	SceneNode::update(frameDelta);	
}

void SfxMgr::spawnDisintegrationEffect(vector2 position) {
	if(effects_n > MAX_EFFECTS_N) return;
	position -= gfx->getCamera()->getAbsoluteTranslation();
	AnimatedSpriteSceneNode* sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("DISINTEGRATION");
	sprite->setSprite(texid);
	DrawPrimitive* quad = sprite->getPrimitive();
	vector2 size = gfx->getTexMgr()->getSize(texid);
	quad->vertices[0] = position - size*0.5f;
	quad->vertices[1].set(position[0] + size[0]/2.f, position[1] - size[1]/2.f);
	quad->vertices[2] = position + size*0.5f;
	quad->vertices[3].set(position[0] - size[0]/2.f, position[1] + size[1]/2.f);
	quad->vertices_colors[0].set(1.f, 1.f, 1.f, 0.75f);
	sprite->setAnimationOnce(0, 6, 6, 32);
	sprite->setAnimationEndCallback(this);
	sprite->setActive(true);
	gfx->addSceneNode(sprite, this);
	effects_n++;
}

void SfxMgr::spawnExplosionEffect(vector2 position) {
	if(effects_n > MAX_EFFECTS_N) return;
	position -= gfx->getCamera()->getAbsoluteTranslation();
	AnimatedSpriteSceneNode* sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId("EXPLOSION");
	sprite->setSprite(texid);
	DrawPrimitive* quad = sprite->getPrimitive();
	vector2 size = gfx->getTexMgr()->getSize(texid);
	quad->vertices[0] = position - size*0.5f;
	quad->vertices[1].set(position[0] + size[0]/2.f, position[1] - size[1]/2.f);
	quad->vertices[2] = position + size*0.5f;
	quad->vertices[3].set(position[0] - size[0]/2.f, position[1] + size[1]/2.f);
	quad->vertices_colors[0].set(1.f, 1.f, 1.f, 0.75f);
	sprite->setAnimationOnce(0, 6, 6, 32);
	sprite->setAnimationEndCallback(this);
	sprite->setActive(true);
	gfx->addSceneNode(sprite, this);
	effects_n++;
}

void SfxMgr::receiveAnimationEndSignal(AnimatedSpriteSceneNode* sprite) {
	if(sprite) {
		sprite->_markForDeletion = true;
		effects_n--;
	}
}

// GfxMgr class methods

GfxMgr::GfxMgr(Game* game_n) {
	game = game_n;
	scene = new SceneNode();
	camera = new CameraSceneNode(this);
	scene->addChild(camera);
	input = new InputHandler();
	physics = new PhysMgr(this);
	ui = new UIMgr(this);
	filesystem = new Filesystem();
	tex = new TexMgr(this);
	sfx = new SfxMgr(this);
	snd = new SndMgr(this);

	level = new LevelSceneNode(this);
	addSceneNode(level, getCamera());
	
	drawCursor = false;
	cursor = NULL;
}

void GfxMgr::C_initCursor() {
	if(cursor) delete cursor;
	cursor = new DrawPrimitive;
	cursor_texture_id = tex->getId("CURSOR");
	for(int x=0; x<4; x++) cursor->vertices_colors[x][3] = 0.9f;
	cursor_textures = tex->getTexture(cursor_texture_id);
	cursor->size[0] = 32.f;
	cursor->size[1] = 32.f;
}

void GfxMgr::C_updateCursor(float frameDelta) {
	if(cursor && drawCursor) {
		cursor->vertices[0].set(0.f, -CURSOR_SIZE);
		cursor->vertices[1].set(CURSOR_SIZE, -CURSOR_SIZE);
		cursor->vertices[2].set(CURSOR_SIZE, 0.f);
		cursor->vertices[3].set(0.f, 0.f);
		for(int x=0; x<4; x++) cursor->vertices[x] += getMousePos(true);
		glrenderer->drawSprite(cursor, cursor_textures[0]);
	}
}

void GfxMgr::setWindowTitle(std::string title) {
	glrenderer->setWindowTitle(title);	
}

void GfxMgr::init(int w_width, int w_height, bool fullscreen, bool vsync, int fsaa) {
	glrenderer = new renderer(w_width, w_height, fullscreen, vsync, fsaa);
	input->init();
	physics->init(PHYSICS_TIME_STEP);
	physics->registerLevelSceneNode(level);
	ui->init();
	tex->init();
	sfx->init();
	snd->init();
	
	int tex_id = tex->getId("CRATE16X16");
	unsigned int* textures = tex->getTexture(tex_id);
	vector2 size = tex->getSize(tex_id);
	C_initCursor();
	
//	glrenderer->__generateBackground(textures[0], (int)size[0], (int)size[1]);
}

void GfxMgr::recomputeBackground() {
	int tex_id = tex->getId("CRATE16X16");
	unsigned int* textures = tex->getTexture(tex_id);
	vector2 size = tex->getSize(tex_id);
	glrenderer->generateBackground(textures[0], (int)size[0], (int)size[1]);	
}


void GfxMgr::frameStart() {
	/* parallax scrolling is implemented here */
	vector2 bg_displacement;
	vector2 camerapos = getCamera()->getAbsoluteTranslation();
	bg_displacement -= camerapos;
	float x = bg_displacement.length();
	x *= 0.10;
	bg_displacement.normalize();
	bg_displacement *= x;
	aabb2 view_bb;
	view_bb.bottom_left = camerapos;
	view_bb.bottom_left[0] *= 0.10f;
	view_bb.top_right = view_bb.bottom_left + glrenderer->getWindowParams();
	glrenderer->frameStart(bg_displacement, view_bb);
}

void GfxMgr::frameEnd() {
	glrenderer->frameEnd();
}

void GfxMgr::updateScene(float frameDelta) {
	updateMousePos(frameDelta);
	physics->update(frameDelta);
	input->update(frameDelta);
	scene->update(frameDelta);
	sfx->update(frameDelta);
	ui->update(frameDelta);
	snd->update(frameDelta);
	C_updateCursor(frameDelta);
}

void GfxMgr::addSceneNode(SceneNode* node, SceneNode* parent) {
	if(parent) {
		parent->addChild(node);
		node->parent = parent;
	}
	else {
		scene->addChild(node);
		node->parent = scene;
	}
}

void GfxMgr::deleteSceneNode(SceneNode* node) {
	node->parent->deleteChild(node);
}

void GfxMgr::setCursor(bool visible) {
	drawCursor = visible;
}

void GfxMgr::updateMousePos(float frameDelta) {
	int newX, newY;
	glfwGetMousePos(&newX, &newY);
	vector2 wparams = glrenderer->getWindowParams();
	float x, y;
	x = newX;
	y = wparams[1] - newY;
	bool inside = true;
	if(x < 0.f) x = 0.f, inside = false;
	if(y < 0.f) y = 0.f, inside = false;
	if(x > wparams[0]) x = wparams[0], inside = false;
	if(y > wparams[1]) y = wparams[1], inside = false;
	//glfwSetMousePos((int)x, int(wparams[1]-y));
	absoluteMousePos.set(x, y);
	currentMousePos = absoluteMousePos + camera->getAbsoluteTranslation();
}

vector2 GfxMgr::getMousePos(bool absolute) {
	if(absolute) return absoluteMousePos;
	else return currentMousePos;	
}

void GfxMgr::reinitLevel() {
	if(level) deleteSceneNode(level);
	level = new LevelSceneNode(this);
	addSceneNode(level, getCamera());
	physics->reinit();	
}

renderer* GfxMgr::getRenderer() {
	return glrenderer;
}

SceneNode* GfxMgr::getScene() {
	return scene;
}

CameraSceneNode* GfxMgr::getCamera() {
	return camera;
}

InputHandler* GfxMgr::getInputHandler() {
	return input;
}

PhysMgr* GfxMgr::getPhysMgr() {
	return physics;
}

LevelSceneNode* GfxMgr::getLevel() {
	return level;
}

UIMgr* GfxMgr::getUI() {
	return ui;	
}

Filesystem* GfxMgr::getFilesystem() {
	return filesystem;
}

TexMgr* GfxMgr::getTexMgr() {
	return tex;
}

SfxMgr* GfxMgr::getSfxMgr() {
	return sfx;
}

SndMgr* GfxMgr::getSndMgr() {
	return snd;
}

int GfxMgr::getObjectsNum() {
	return scene->getAbsoluteChildrenNum();	
}

Game* GfxMgr::getGame() {
	return game;	
}

// Camera Scene Node
CameraSceneNode::CameraSceneNode(GfxMgr* gfx_n) {
	type = SN_TYPE_CAMERA;
	gfx = gfx_n;
	glrenderer = gfx->getRenderer();
	isMoving = false;
	speed = MAX_CAMERA_MOVE_SPEED;
//	transf->setTranslation(vector2(-1.f, -1.f));
	keyboardControl = false;
	followPrimitive = NULL;
	position = vector2(1.f, 1.f);
}

void CameraSceneNode::setKeyboardControl(bool state) {
	keyboardControl = state;	
	if(state)
		followPrimitive = NULL;
}

void CameraSceneNode::update(float frameDelta) {
	_follow(frameDelta);
	_move(frameDelta);
	glPushMatrix();
	glTranslatef(-position[0], -position[1], 0.f);
	SceneNode::update(frameDelta);
	glPopMatrix();
}

void CameraSceneNode::_move(float frameDelta) {
	if(isMoving) {
		vector2 transl_vector = direction * currentSpeed * frameDelta;
		currentSpeed += frameDelta*500.f;
		if(currentSpeed > speed) currentSpeed = speed;
		position += transl_vector;
		if(position[0] < 1 || position[0] + gfx->getRenderer()->getWindowParams()[0] >= LEVEL_SIZE_X*PPT_X)
			position -= vector2(transl_vector[0], 0.f);
		if(position[1] < 1 || position[1] + gfx->getRenderer()->getWindowParams()[1] >= LEVEL_SIZE_Y*PPT_Y)
			position -= vector2(0.f, transl_vector[1]);
	}
}

void CameraSceneNode::_follow(float frameDelta) {
	if(followPrimitive) {
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		vector2 screencenter = wparams * 0.5f + position;

		vector2 spring = followPrimitive->center - screencenter;
	//	if(spring.length2() < 10.f) return;
		position += spring*frameDelta*5.f;
		
		if(position[0] < 1.f)
			position[0] = 1.f;
		else if(position[0] + wparams[0] >= LEVEL_SIZE_X*PPT_X)
			position[0] = LEVEL_SIZE_X*PPT_X - wparams[0];

		if(position[1] < 1.f)
			position[1] = 1.f;
		else if(position[1] + wparams[1] >= LEVEL_SIZE_Y*PPT_Y)
			position[1] = LEVEL_SIZE_Y*PPT_Y - wparams[1];
	}
}

void CameraSceneNode::setTranslation(vector2 translation_n) {
	position = translation_n;	
}

vector2 CameraSceneNode::getAbsoluteTranslation() {
	return position;
}

void CameraSceneNode::setMovement(vector2 direction_n, float speed_n) {
	speed = speed_n;
	direction = direction_n;
	direction.normalize();
}

void CameraSceneNode::setFollow(Primitive* primitive) {
	if(primitive) followPrimitive = primitive;
	keyboardControl = false;
	currentSpeed = 0;
	speed = 50;
}

void CameraSceneNode::startMovement() {
	isMoving = true;
	currentSpeed = 0;	
}

void CameraSceneNode::stopMovement() {
	isMoving = false;
	currentSpeed = 0;	
}

void CameraSceneNode::receiveKeyEvent(list<KeyEvent> events) {
	if(keyboardControl)
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		if((*x).state == 1)
		switch((*x).key) {
		case KEY_UP:
			setMovement(vector2(0.f, 1.f), MAX_CAMERA_MOVE_SPEED);
			startMovement();
			break;
		case KEY_LEFT:
			setMovement(vector2(-1.f, 0.f), MAX_CAMERA_MOVE_SPEED);
			startMovement();
			break;
		case KEY_RIGHT:
			setMovement(vector2(1.f, 0.f), MAX_CAMERA_MOVE_SPEED);
			startMovement();
			break;
		case KEY_DOWN:
			setMovement(vector2(0.f, -1.f), MAX_CAMERA_MOVE_SPEED);
			startMovement();
			break;
		}
		else	stopMovement();
	}
}
