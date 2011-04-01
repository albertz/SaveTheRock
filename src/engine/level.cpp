/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

class bitmap2;

using namespace std;

bool get_bit(int index, char x) {
	return !((x & 1<<index) == 0);
}

LevelSceneNode::LevelSceneNode(GfxMgr* gfx_n) {
	type = SN_TYPE_LEVEL;
	tile_color = TILE_FACE_COLOR;
	lastCameraPos = vector2(-2.f, -2.f);
	gfx = gfx_n;
	glrenderer = gfx->getRenderer();
	for(int x=0; x<LEVEL_SIZE_X; x++) {
		for(int y=0; y<LEVEL_SIZE_Y;y++) {
			map[x][y] = 0;
		}
	}
	named_positions = new NamedPosition[MAX_NAMED_POSITIONS];
	named_positions_n = 0;
	slantedTiles = false;
	_isActive = false;
	objectsContainer = NULL;
	backgroundContainer = NULL;
	foregroundContainer = NULL;
	_markForDeletion = 0;
	showNames = false;
	showNamedPositions = false;
	parser = NULL;
	pchar = NULL;
	rock = NULL;
	render_vertex_list = NULL;
	levelTime = 0.f;
	loaded = false;
	checkpointIds = new int[MAX_CHECKPOINTS];
	checkpointPositions = new vector2*[MAX_CHECKPOINTS];
	for(int x=0; x<MAX_CHECKPOINTS; x++) checkpointPositions[x] = new vector2();
	checkpoints_n = 0;
	lastVisitedCheckpoint = 0;
}

LevelSceneNode::~LevelSceneNode() {
	if(objectsContainer) gfx->deleteSceneNode(objectsContainer);
	objectsContainer = NULL;
	backgroundContainer = NULL;
	foregroundContainer = NULL;
	delete [] checkpointIds;
	for(int x=0; x<checkpoints_n; x++) delete checkpointPositions[x];
	delete [] checkpointPositions;
}

void LevelSceneNode::createLabel(vector2 position, const std::string& text) {
	UILabel* label = new UILabel(gfx);
	label->setColor(MENU_TEXT_COLOR);
	label->setText(text, 0, 18.f);
	label->setPosition(position);
	label->setActive(true);
	gfx->addSceneNode(label, objectsContainer);
}

void LevelSceneNode::parseNamedPositionString(const std::string& string) {
	parser = new NamedPosStringParser();
	parser->parseString(string);
	named_pos_vars = parser->getVars();
	named_pos_values = parser->getValues();
}

bool LevelSceneNode::loadFromFile(const std::string& filename, bool setActive) {
	ifstream fin(filename.c_str(), ios::binary);
	if(!fin) return false;
	BinaryLevel level;
	fin.read((char*)(&level), sizeof(level));
	for(int x=0; x<(LEVEL_SIZE_X); x++)
	for(int y=0; y<(LEVEL_SIZE_Y); y++)
		if(level.bitmap[x][y])
			map[x][y] = 15;
		else map[x][y] = 0;
	named_positions_n = level.named_positions_n;
	for(int x=0; x<named_positions_n; x++) {
		named_positions[x] = level.named_positions[x];
	}
	computeSlanted();
	populateObjects(setActive);
	levelTime = 0.f;
	loaded = false;
	return true;
}

bool LevelSceneNode::writeToFile(const std::string& filename) {
	ofstream fout(filename.c_str(), ios::binary);
	if(!fout) return false;
	BinaryLevel level;
	for(int x=0; x<LEVEL_SIZE_X; x++) {
		for(int y=0; y<LEVEL_SIZE_Y; y++) {
			if(map[x][y] == 15)
				level.bitmap[x][y] = true;
			else
				level.bitmap[x][y] = false;
		}
	}

	level.named_positions_n = named_positions_n;
	for(int x=0; x<named_positions_n; x++) {
		level.named_positions[x] = named_positions[x];
	}

	fout.write((char*)(&level), sizeof(level));
	return true;
}

void LevelSceneNode::update(float frameDelta) {
	if(_isActive) {
		if(!loaded && levelTime > 1.f) {
			loaded = true;
			gfx->getUI()->darkenScreenFadeOut();
		}

		levelTime += frameDelta;
		SceneNode::update(frameDelta);

		vector2 camera_pos = gfx->getCamera()->getAbsoluteTranslation();
		vector2 window = gfx->getRenderer()->getWindowParams();
		if(drawSelf()) gfx->getRenderer()->drawLevel(render_vertex_list, rvl_ptr);
		else gfx->getRenderer()->redrawLevel();

		if(showNamedPositions) {
			for(int x=0; x<named_positions_n; x++) {
				vector2 center = vector2(named_positions[x].xy[0], named_positions[x].xy[1]) - gfx->getCamera()->getAbsoluteTranslation();
				gfx->getRenderer()->drawCircle(center, 20.f);
			}
		}
	}
}

Primitive* LevelSceneNode::buildTrianglePhysics(float ax, float ay, float bx, float by, float cx, float cy) {
	Primitive* triangle = new Primitive;
	triangle->type = PRIM_TYPE_TRIANGLE;
	triangle->vertices[0] = vector2(ax, ay);
	triangle->vertices[1] = vector2(bx, by);
	triangle->vertices[2] = vector2(cx, cy);
	gfx->getRenderer()->findPrimitiveBoundingCircle(triangle);
	triangle->edges[0].vertexA = &(triangle->vertices[0]);
	triangle->edges[0].vertexB = &(triangle->vertices[1]);
	triangle->edges[0].parent = triangle;
	triangle->edges[1].vertexA = &(triangle->vertices[1]);
	triangle->edges[1].vertexB = &(triangle->vertices[2]);
	triangle->edges[1].parent = triangle;
	triangle->edges[2].vertexA = &(triangle->vertices[2]);
	triangle->edges[2].vertexB = &(triangle->vertices[0]);
	triangle->edges[2].parent = triangle;
	return triangle;
}


Primitive* LevelSceneNode::buildQuadPhysics(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy) {
	Primitive* quad = new Primitive;
	quad->type = PRIM_TYPE_QUAD;
	quad->vertices[0] = vector2(ax, ay);
	quad->vertices[1] = vector2(bx, by);
	quad->vertices[2] = vector2(cx, cy);
	quad->vertices[3] = vector2(dx, dy);
	for(int x=0;x<4;x++)
		quad->vertices_oldPos[x] = vector2();
	quad->edges[0].vertexA = &(quad->vertices[0]);
	quad->edges[0].vertexB = &(quad->vertices[1]);
	quad->edges[0].parent = quad;
	quad->edges[1].vertexA = &(quad->vertices[1]);
	quad->edges[1].vertexB = &(quad->vertices[2]);
	quad->edges[1].parent = quad;
	quad->edges[2].vertexA = &(quad->vertices[2]);
	quad->edges[2].vertexB = &(quad->vertices[3]);
	quad->edges[2].parent = quad;
	quad->edges[3].vertexA = &(quad->vertices[3]);
	quad->edges[3].vertexB = &(quad->vertices[0]);
	quad->edges[3].parent = quad;

	gfx->getRenderer()->findPrimitiveBoundingCircle(quad);
	gfx->getPhysMgr()->calculatePolyCenter(quad);
	return quad;
}

void LevelSceneNode::redraw() {
	lastCameraPos.set(-2.f, -2.f);
}

bool LevelSceneNode::drawSelf() {
	unsigned char tile;
	int neighbors_n;
	float ax = 0.f, ay = 0.f, bx = 0.f, by = 0.f, cx = 0.f, cy = 0.f, dx = 0.f, dy = 0.f;

	int START_X = 0, END_X = LEVEL_SIZE_X;
	int START_Y = 0, END_Y = LEVEL_SIZE_Y;
	// cull tiles to camera
	CameraSceneNode* camera = gfx->getCamera();
	vector2 abstrans = camera->getAbsoluteTranslation();
	if(abstrans == lastCameraPos) {
		// no need to redraw, viewpoint hasn't changed
		return false;
	}

	if(render_vertex_list) delete [] render_vertex_list;
	render_vertex_list = new float[VERTEX_ARRAY_SIZE];

	lastCameraPos = abstrans;
	vector2 window = gfx->getRenderer()->getWindowParams();
	vector2 lower_left = abstrans, upper_right = abstrans + window;
	START_X = (int)floor(lower_left[0]/PPT_X - CULLING_EPSILON);
	END_X = (int)floor(upper_right[0]/PPT_X + CULLING_EPSILON);
	START_Y = (int)floor(lower_left[1]/PPT_Y - CULLING_EPSILON);
	END_Y = (int)floor(upper_right[1]/PPT_Y + CULLING_EPSILON);
	if(START_X < 0) START_X = 0;
	if(END_X > LEVEL_SIZE_X) END_X = LEVEL_SIZE_X;
	if(START_Y < 0) START_Y = 0;
	if(END_Y > LEVEL_SIZE_X) END_Y = LEVEL_SIZE_X;
	//

	rvl_ptr = 0;

	for(int x=START_X; x<END_X; x++)
		for(int y=START_Y; y<END_Y; y++) {
			tile = (unsigned char)map[x][y];
		//	neighbors_n = get_bit(0, tile) + get_bit(1, tile) + get_bit(2, tile) + get_bit(3, tile);
			if(tile != 0) {
				if((unsigned char)tile == 15) {
					render_vertex_list[rvl_ptr] = x*PPT_X;
					render_vertex_list[rvl_ptr+1] = y*PPT_Y;
					bx = x*PPT_X+PPT_X; by = y*PPT_Y;
					dx = x*PPT_X; dy = y*PPT_Y+PPT_Y;
					render_vertex_list[rvl_ptr+2] = bx;
					render_vertex_list[rvl_ptr+3] = by;
					render_vertex_list[rvl_ptr+4] = dx;
					render_vertex_list[rvl_ptr+5] = dy;
					rvl_ptr += 6;
					render_vertex_list[rvl_ptr] = dx;
					render_vertex_list[rvl_ptr+1] = dy;
					render_vertex_list[rvl_ptr+2] = bx;
					render_vertex_list[rvl_ptr+3] = by;
					render_vertex_list[rvl_ptr+4] = x*PPT_X+PPT_X;
					render_vertex_list[rvl_ptr+5] = y*PPT_Y+PPT_Y;
					rvl_ptr += 6;
				} else {
				if(get_bit(0, tile)) {
					render_vertex_list[rvl_ptr] = x*PPT_X + PPT_X/2;
					render_vertex_list[rvl_ptr+1] = y*PPT_Y + PPT_Y/2;
					render_vertex_list[rvl_ptr+2] = x*PPT_X;
					render_vertex_list[rvl_ptr+3] = y*PPT_Y;
					render_vertex_list[rvl_ptr+4] = x*PPT_X + PPT_X;
					render_vertex_list[rvl_ptr+5] = y*PPT_Y;
					rvl_ptr += 6;
				}
				if(get_bit(1, tile)) {
					render_vertex_list[rvl_ptr] = x*PPT_X + PPT_X;
					render_vertex_list[rvl_ptr+1] = y*PPT_Y+PPT_Y;
					render_vertex_list[rvl_ptr+2] = x*PPT_X + PPT_X/2;
					render_vertex_list[rvl_ptr+3] = y*PPT_Y + PPT_Y/2;
					render_vertex_list[rvl_ptr+4] = x*PPT_X+PPT_X;
					render_vertex_list[rvl_ptr+5] = y*PPT_Y;
					rvl_ptr += 6;
				}
				if(get_bit(2, tile)) {
					render_vertex_list[rvl_ptr] = x*PPT_X+PPT_X;
					render_vertex_list[rvl_ptr+1] = y*PPT_Y+PPT_Y;
					render_vertex_list[rvl_ptr+2] = x*PPT_X+PPT_X/2;
					render_vertex_list[rvl_ptr+3] = y*PPT_Y + PPT_Y/2;
					render_vertex_list[rvl_ptr+4] = x*PPT_X;
					render_vertex_list[rvl_ptr+5] = y*PPT_Y+PPT_Y;
					rvl_ptr += 6;
				}
				if(get_bit(3, tile)) {
					render_vertex_list[rvl_ptr] = x*PPT_X;
					render_vertex_list[rvl_ptr+1] = y*PPT_Y+PPT_Y;
					render_vertex_list[rvl_ptr+2] = x*PPT_X + PPT_X/2;
					render_vertex_list[rvl_ptr+3] = y*PPT_Y + PPT_Y/2;
					render_vertex_list[rvl_ptr+4] = x*PPT_X;
					render_vertex_list[rvl_ptr+5] = y*PPT_Y;
					rvl_ptr += 6;
				}
				}
			}
		}
	return true;
}

list<Primitive*> LevelSceneNode::getGeometry(aabb2 bounds) {
	list<Primitive*> geometryList;
	unsigned char tile;
	int neighbors_n;
	float ax = 0.f, ay = 0.f, bx = 0.f, by = 0.f, cx = 0.f, cy = 0.f, dx = 0.f, dy = 0.f;
	// compute tiles between bounds
	int START_X = 0, END_X = LEVEL_SIZE_X;
	int START_Y = 0, END_Y = LEVEL_SIZE_Y;
	vector2 lower_left = bounds.bottom_left, upper_right = bounds.top_right;
	START_X = (int)floor(lower_left[0]/PPT_X - COLLISION_EPSILON);
	END_X = (int)floor(upper_right[0]/PPT_X + COLLISION_EPSILON);
	START_Y = (int)floor(lower_left[1]/PPT_Y - COLLISION_EPSILON);
	END_Y = (int)floor(upper_right[1]/PPT_Y + COLLISION_EPSILON);
	if(START_X < 0) START_X = 0;
	if(END_X > LEVEL_SIZE_X) END_X = LEVEL_SIZE_X;
	if(START_Y < 0) START_Y = 0;
	if(END_Y > LEVEL_SIZE_X) END_Y = LEVEL_SIZE_X;

	for(int x=START_X; x<END_X; x++)
		for(int y=START_Y; y<END_Y; y++) {
			tile = (unsigned char)map[x][y];
			neighbors_n = get_bit(0, tile) + get_bit(1, tile) + get_bit(2, tile) + get_bit(3, tile);
			if(tile != 0) {
				if(neighbors_n > 2) {
					ax = x*PPT_X, ay = y*PPT_Y;
					bx = x*PPT_X+PPT_X, by = y*PPT_Y;
					cx = x*PPT_X+PPT_X, cy = y*PPT_Y+PPT_Y;
					dx = x*PPT_X; dy = y*PPT_Y+PPT_Y;
					geometryList.push_back(buildQuadPhysics(ax, ay, bx, by, cx, cy, dx, dy));
				} else {
				if(get_bit(0, tile) && get_bit(1, tile)) {
					ax = x*PPT_X; ay = y*PPT_Y;
					bx = x*PPT_X + PPT_X; by = y*PPT_Y + PPT_Y;
					cx = x*PPT_X + PPT_X; cy = y*PPT_Y;
					geometryList.push_back(buildTrianglePhysics(ax, ay, bx, by, cx, cy));
				}
				if(get_bit(1, tile) && get_bit(2, tile)) {
					ax = x*PPT_X+PPT_X; ay = y*PPT_Y;
					bx = x*PPT_X + PPT_X; by = y*PPT_Y + PPT_Y;
					cx = x*PPT_X; cy = y*PPT_Y+PPT_Y;
					geometryList.push_back(buildTrianglePhysics(ax, ay, bx, by, cx, cy));
				}
				if(get_bit(2, tile) && get_bit(3, tile)) {
					ax = x*PPT_X+PPT_X; ay = y*PPT_Y+PPT_Y;
					bx = x*PPT_X; by = y*PPT_Y;
					cx = x*PPT_X; cy = y*PPT_Y+PPT_Y;
					geometryList.push_back(buildTrianglePhysics(ax, ay, bx, by, cx, cy));
				}
				if(get_bit(0, tile) && get_bit(3, tile)) {
					ax = x*PPT_X; ay = y*PPT_Y;
					bx = x*PPT_X + PPT_X; by = y*PPT_Y;;
					cx = x*PPT_X; cy = y*PPT_Y+PPT_Y;
					geometryList.push_back(buildTrianglePhysics(ax, ay, bx, by, cx, cy));
				}
				}
			}
		}
	return geometryList;
}

void LevelSceneNode::setShowNames(bool state) {
	showNames = state;
}

void LevelSceneNode::setShowNamedPositions(bool state) {
	showNamedPositions = state;
}

void LevelSceneNode::addNamedPosition(vector2 coords, const std::string& string) {
	named_positions[named_positions_n].xy[0] = coords[0];
	named_positions[named_positions_n].xy[1] = coords[1];
	strcpy(named_positions[named_positions_n].string, string.c_str());
	named_positions_n++;
	populateObjects();
}

void LevelSceneNode::delNamedPosition(int index) {
	if(index < 0 || index > named_positions_n-1) return;

	if(getNamedPosValue(index, "OBJ") == "HERO") {
		pchar = NULL;
	}

	NamedPosition* tmp = new NamedPosition[MAX_NAMED_POSITIONS];
	int x = 0, y = 0;
	while(y < named_positions_n-1) {
		if(x == index) {
			x++;
		}
		tmp[y] = named_positions[x];
		x++;
		y++;
	}
	delete [] named_positions;
	named_positions = tmp;
	named_positions_n--;
	populateObjects();
}

int LevelSceneNode::getNamedPositionsN() {
	return named_positions_n;
}

std::string LevelSceneNode::getNamedPosString(int index) {
	if(index >= named_positions_n || index < 0) return NULL;
	return named_positions[index].string;
}

std::string LevelSceneNode::getNamedPosValue(int index, const std::string& var_n) {
	if(index >= named_positions_n || index < 0) return NULL;
	parseNamedPositionString(named_positions[index].string);

	for(list<std::string>::iterator varit = named_pos_vars.begin(), valueit = named_pos_values.begin(); varit != named_pos_vars.end(); varit++, valueit++) {
		std::string var = *varit;
		std::string value = *valueit;
		if(var == var_n) {
			return value;
		}
	}
	return "";
}

vector2 LevelSceneNode::getNamedPosPosition(int index) {
	if(index >= named_positions_n || index < 0) return vector2();
	return vector2(named_positions[index].xy[0], named_positions[index].xy[1]);
}

SceneNode* LevelSceneNode::getObjectByName(const std::string& name) {
	if(name == "") return NULL;
	if(!objectsContainer) return NULL;

	list<SceneNode*>* nodes = objectsContainer->getChildren();

	for(list<SceneNode*>::iterator x = nodes->begin(); x != nodes->end(); x++) {
#ifdef _WIN32
		if(stricmp((*x)->getName().c_str(), name.c_str()) == 0) {
#else
		if(strcasecmp((*x)->getName().c_str(), name.c_str()) == 0) {
#endif
			return *x;
		}
	}
	return NULL;
}

int LevelSceneNode::checkNamedPosArea(aabb2 bb) {
	for(int x=0; x<named_positions_n; x++) {
		NamedPosition np = named_positions[x];
		if(np.xy[0] > bb.bottom_left[0] && np.xy[0] < bb.top_right[0])
		if(np.xy[1] > bb.bottom_left[1] && np.xy[1] < bb.top_right[1])
			return x;
	}
	return -1;
}

int LevelSceneNode::checkNamedPosTile(int tile_x, int tile_y) {
	for(int x=0; x<named_positions_n; x++) {
		NamedPosition np = named_positions[x];
		if(np.xy[0] > tile_x*PPT_X && np.xy[0] < tile_x*PPT_X+PPT_X)
		if(np.xy[1] > tile_y*PPT_Y && np.xy[1] < tile_y*PPT_Y+PPT_Y)
			return x;
	}
	return -1;
}

int LevelSceneNode::findNamedPosition(vector2 coords) {
	for(int x=0; x<named_positions_n; x++) {
		NamedPosition np = named_positions[x];
		if(np.xy[0] > coords[0]-NP_SEARCH_BOX_X && np.xy[0] < coords[0]+NP_SEARCH_BOX_X)
		if(np.xy[1] > coords[1]-NP_SEARCH_BOX_Y && np.xy[1] < coords[1]+NP_SEARCH_BOX_Y)
			return x;
	}
	return -1;
}



void LevelSceneNode::computeSlanted() {
	unsigned char neighbors_map = 0;
	unsigned int neighbors_num = 0;
	for(int x=0; x<LEVEL_SIZE_X; x++) {
		for(int y=0; y<LEVEL_SIZE_Y; y++) {
			if(map[x][y] != 15) {
				map[x][y] = 0;
				neighbors_map = 0;
				if(y-1>=0)
				if(map[x][y-1] == 15) {
					neighbors_num++;
					neighbors_map = 1;
				}
				if(map[x+1][y] == 15) {
					neighbors_num++;
					neighbors_map += 2;
				}
				if(map[x][y+1] == 15) {
					neighbors_num++;
					neighbors_map += 4;
				}
				if(x-1>=0)
				if(map[x-1][y] == 15) {
					neighbors_num++;
					neighbors_map += 8;
				}
				if(neighbors_map == 15) map[x][y] = 15;
				else if(neighbors_map == 4) map[x][y] = 0;
				else if(neighbors_num > 1 && neighbors_map > 2 && neighbors_map != 5 && neighbors_map != 10)
					map[x][y] = neighbors_map;
				neighbors_num = 0;
			}
		}
	}
	redraw();
	slantedTiles = true;
}

void LevelSceneNode::removeSlanted() {
	for(int x=0; x<LEVEL_SIZE_X; x++) {
		for(int y=0; y<LEVEL_SIZE_Y; y++) {
			if(map[x][y] != 15)
				map[x][y] = 0;
		}
	}
	redraw();
	slantedTiles = false;
}

void LevelSceneNode::populateObjects(bool setActive) {
	gfx->getPhysMgr()->reinit();

	if(objectsContainer) gfx->deleteSceneNode(objectsContainer);
	objectsContainer = new ContainerSceneNode(gfx);
	gfx->addSceneNode(objectsContainer, gfx->getCamera());
	backgroundContainer = new ContainerSceneNode(gfx);
	gfx->addSceneNode(backgroundContainer, objectsContainer);

	pchar = NULL;
	rock = NULL;

	for(int x=0; x<named_positions_n; x++) {
		std::string buffer = named_positions[x].string;
		parseNamedPositionString(buffer);

		// VAR : VALUE pairs
		std::string name;
		float interval; // floating point interval in seconds
		float destination_x = 0.f; // vector2 position
		float destination_y = 0.f;
		float moveSpeed = 0.f;
		float firingSpeed = 0.f;
		float projectileSpeed = 0.f;
		bool state = false;
		std::string target;
		std::string objName;
		std::string text;
		float size = 0.f;
		int switch_times = 0;
		float type = 0.f;
		float force = 0.f;
		int id = 0;
		//

		std::string var;
		std::string value;

		for(list<std::string>::iterator varit = named_pos_vars.begin(), valueit = named_pos_values.begin(); varit != named_pos_vars.end(); varit++, valueit++) {
			var = *varit;
			value = *valueit;
			if(var == "OBJ") {
				name = value;
			} else if(var == "INTERVAL") {
				interval = atof(value.c_str());
			} else if(var == "DESTX") {
				destination_x = atof(value.c_str());
			} else if(var == "DESTY") {
				destination_y = atof(value.c_str());
			} else if(var == "FIRINGSPEED") {
				firingSpeed = atof(value.c_str());
			} else if(var == "MOVESPEED") {
				moveSpeed = atof(value.c_str());
			} else if(var == "PROJECTILESPEED") {
				projectileSpeed = atof(value.c_str());
			} else if(var == "STATE") {
				state = atof(value.c_str());
			} else if(var == "NAME") {
				objName = value;
			} else if(var == "TARGET") {
				target = value;
			} else if(var == "SWITCHTIMES") {
				switch_times = atoi(value.c_str());
			} else if(var == "TYPE") {
				type = atof(value.c_str());
			} else if(var == "SIZE") {
				size = atof(value.c_str());
			} else if(var == "TEXT") {
				text = value;
			} else if(var == "FORCE") {
				force = atof(value.c_str());
			} else if(var == "ID") {
				id = atoi(value.c_str());
			}
		}

		vector2 destination(destination_x, destination_y);
		vector2 position(named_positions[x].xy[0], named_positions[x].xy[1]);

		if(name == "CRATE") {
			Ragdoll* ragdoll = new Ragdoll(gfx);
			ragdoll->init(position, "CRATE");
			ragdoll->setActive(true);
			ragdoll->setName(objName);
			gfx->addSceneNode(ragdoll, objectsContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "HERO") {
			if(!pchar) {
				pchar = new PlayableCharacter(gfx);
				pchar->init(position);
				pchar->setActive(true);
				pchar->setName(objName);
				if(setActive)
					gfx->getCamera()->setFollow(pchar->getCollisionPrimitive());
				gfx->addSceneNode(pchar, objectsContainer);
				if(showNames) createLabel(position, objName);
			}
		} else if(name == "WALLLASER") {
			WallLaser* laser = new WallLaser(gfx);
			laser->setName(objName);
			laser->init(position, type, interval);
			laser->setActive(true);
			gfx->addSceneNode(laser, objectsContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "ROBOTGUARD") {
			RobotGuard* robot = new RobotGuard(gfx);
			robot->init(position);
			robot->setName(objName);
			robot->setPatrol(setActive, position, destination, moveSpeed);
			robot->setFire(setActive, firingSpeed, projectileSpeed);
			robot->setActive(true);
			gfx->addSceneNode(robot, objectsContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "SWITCH") {
			SceneNode* tg = getObjectByName(target);
			Switch* swtch = new Switch(gfx);
			swtch->setName(objName);
			swtch->init(position, switch_times, tg);
			swtch->setActive(true);
			gfx->addSceneNode(swtch, backgroundContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "MINE") {
			Mine* mine = new Mine(gfx);
			mine->setName(objName);
			mine->init(position);
			mine->setActive(true);
			gfx->addSceneNode(mine, objectsContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "EXIT") {
			LevelExit* levelexit = new LevelExit(gfx);
			levelexit->setName(objName);
			levelexit->init(position);
			levelexit->setActive(true);
			gfx->addSceneNode(levelexit, backgroundContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "TEXT") {
			UILabel* label = new UILabel(gfx);
			if(!setActive) label->setColor(color(0.f, 0.f, 0.f, 1.f));
			else label->setColor(LEVEL_TEXT_COLOR);
			label->setText(text, 0, size, true);
			int tile_x, tile_y;
			getTileByXY(position[0], position[1], &tile_x, &tile_y);
			label->setPosition(vector2((tile_x-1)*PPT_X, tile_y*PPT_Y+PPT_Y/2.f));
			label->setActive(true);
			gfx->addSceneNode(label, backgroundContainer);
		} else if(name == "PRESSUREPAD") {
			SceneNode* tg = getObjectByName(target);
			PressurePad* pad = new PressurePad(gfx);
			pad->setName(objName);
			pad->init(position, tg);
			pad->setActive(true);
			gfx->addSceneNode(pad, backgroundContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "GRAVITYLIFT") {
			GravityLift* lift = new GravityLift(gfx);
			lift->setName(objName);
			lift->init(position, force);
			lift->setActive(true);
			gfx->addSceneNode(lift, backgroundContainer);
			if(showNames) createLabel(position, objName);
		} else if(name == "ROCK") {
			if(!rock) {
				rock = new TheRock(gfx);
				rock->setName(objName);
				rock->init(position);
				rock->setActive(true);
				gfx->addSceneNode(rock, objectsContainer);
				if(showNames) createLabel(position, objName);
			}
		} else if(name == "CHECKPOINT") {
			Checkpoint* cp = new Checkpoint(gfx);
			cp->init(position, id);
			cp->setActive(true);
			gfx->addSceneNode(cp, backgroundContainer);
			if(showNames) createLabel(position, objName);
		}
	}
}

void LevelSceneNode::depopulateObjects() {
	if(objectsContainer) gfx->deleteSceneNode(objectsContainer);
	objectsContainer = NULL;
	pchar = NULL;
}

void LevelSceneNode::clear() {
	for(int x=0; x<LEVEL_SIZE_X; x++)
	for(int y=0; y<LEVEL_SIZE_Y; y++)
		if(x == 0 || y == 0 || x == LEVEL_SIZE_X-1 || y == LEVEL_SIZE_Y-1) map[x][y] = 15;
		else map[x][y] = 0;
	rvl_ptr = 0;
	if(slantedTiles) computeSlanted();
	if(objectsContainer) gfx->deleteSceneNode(objectsContainer);
	pchar = NULL;
	rock = NULL;
	objectsContainer = NULL;
	named_positions_n = 0;
	redraw();
}

void LevelSceneNode::getTileByXY(float x, float y, int* result_x, int* result_y) {
	*result_x = (int)floor(x / PPT_X);
	*result_y = (int)floor(y / PPT_Y);
}

vector2 LevelSceneNode::getTileByXY(vector2 coords) {
	vector2 result;
	result[0] = floor(coords[0] / PPT_X);
	result[1] = floor(coords[1] / PPT_Y);
	return result;
}

void LevelSceneNode::fillTile(int x, int y) {
	map[x][y] = 15;
	redraw();
	if(slantedTiles) computeSlanted();
	else removeSlanted();
}

void LevelSceneNode::clearTile(int x, int y) {
	map[x][y] = 0;
	redraw();
	if(slantedTiles) computeSlanted();
	else removeSlanted();
}

void LevelSceneNode::fillTile(vector2 mousecoords) {
	int x, y;
	getTileByXY(mousecoords[0], mousecoords[1], &x, &y);
	if(x>0 && y>0 && x<LEVEL_SIZE_X-1 && y<LEVEL_SIZE_Y-1)
		fillTile(x, y);
}

void LevelSceneNode::clearTile(vector2 mousecoords) {
	int x, y;
	getTileByXY(mousecoords[0], mousecoords[1], &x, &y);
	if(x>0 && y>0 && x<LEVEL_SIZE_X-1 && y<LEVEL_SIZE_Y-1) {
		int namedPos = checkNamedPosTile(x, y);
		if(namedPos >= 0 && map[x][y] == 15) {
			delNamedPosition(namedPos);
			populateObjects();
		}
		clearTile(x, y);
	}
}

void LevelSceneNode::invertTileRegion(int start_x, int end_x, int start_y, int end_y) {
	if(start_x <= 0) start_x = 1;
	if(start_y <= 0) start_y = 1;

	if(end_x >= LEVEL_SIZE_X) end_x = LEVEL_SIZE_X-1;
	if(end_y >= LEVEL_SIZE_Y) end_y = LEVEL_SIZE_Y-1;

	for(int x=start_x; x<end_x; x++)
	for(int y=start_y; y<end_y; y++) {
		if(map[x][y] != 15) {
			Primitive* tile;
			tile = new Primitive;
			tile->type = PRIM_TYPE_QUAD;
			tile->vertices[0].set(x*PPT_X, y*PPT_Y);
			tile->vertices[1].set(x*PPT_X+PPT_X, y*PPT_Y);
			tile->vertices[2].set(x*PPT_X+PPT_X, y*PPT_Y+PPT_Y);
			tile->vertices[3].set(x*PPT_X, y*PPT_Y+PPT_Y);
			gfx->getPhysMgr()->setQuadEdgesAndConstraints(tile);
			CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(tile, false);
			if(!event.doIntersect)
				map[x][y] = 15;
		}
		else map[x][y] = 0;
	}

	redraw();
	if(slantedTiles) computeSlanted();
	else removeSlanted();
}

bool LevelSceneNode::isSlope(float x, float y) {
	int x_n, y_n;
	getTileByXY(x, y, &x_n, &y_n);
	if(map[x_n][y_n] != 0 && map[x_n][y_n] != 15) {
		return true;
	}
	return false;
}

int LevelSceneNode::getTile(int x, int y) {
	return map[x][y];
}

int LevelSceneNode::getTile(vector2 coords) {
	coords = getTileByXY(coords);
	return map[(int)coords[0]][(int)coords[1]];
}

PlayableCharacter* LevelSceneNode::getPlayableCharacter() {
	return pchar;
}

TheRock* LevelSceneNode::getTheRock() {
	return rock;
}

float LevelSceneNode::getTime() {
	return levelTime;
}

void LevelSceneNode::addCheckpoint(int id, vector2 position) {
	checkpointIds[checkpoints_n] = id;
	*(checkpointPositions[checkpoints_n]) = position;
	checkpoints_n++;
}

void LevelSceneNode::setLastVisitedCheckpoint(int id) {
	lastVisitedCheckpoint = id;
}

int LevelSceneNode::getLastVisitedCheckpoint() {
	return lastVisitedCheckpoint;
}

vector2 LevelSceneNode::getCheckpointPosition(int id) {
	for(int x=0; x<checkpoints_n; x++) {
		if(checkpointIds[x] == id) {
			return *(checkpointPositions[x]);
		}
	}

	return vector2();
}

bool LevelSceneNode::receiveCollisionEvent(CollisionEvent event) {
	// stub
}

void LevelSceneNode::populate_rand() {
	for(int x=0; x<LEVEL_SIZE_X; x++)
		for(int y=0; y<LEVEL_SIZE_Y; y++)
			if(rand() % 10 < 3)
				map[x][y] = 15;
			else
				map[x][y] = 0;
}

void LevelSceneNode::exportLevelInfo(const std::string& filename_n) {
	ofstream fout(filename_n.c_str(), ios::binary);
	if(!fout) return;

	// level size
	fout << "*** Objects (" << named_positions_n << ") ***" << endl;
	for(int x=0; x<named_positions_n; x++) {
		fout << named_positions[x].xy[0] << " " << named_positions[x].xy[0] << " - " << named_positions[x].string << endl;
	}

	// map
	fout << endl << endl << "*** MAP ***" << endl << endl;
	for(int y=LEVEL_SIZE_Y-1; y>=0; y--) {
		for(int x=0; x<LEVEL_SIZE_X; x++) {
			if(map[x][y] == 15) fout << "#";
			else fout << ".";
		}
		fout << endl;
	}
}
