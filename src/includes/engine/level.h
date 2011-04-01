/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __LEVEL_H
#define __LEVEL_H

/*
NamedPosition is a struct holding data for a level object;
xy are vector coordinates for the position of the object (usually its center)
string is a string describing the object. It has the following format:

string = OBJ=%name_%variable1=%value1_$variable2=$value2_..._$variableN=$valueN
*/

class PlayableCharacter;
class TheRock;

struct NamedPosition {
	char string[NAMED_POSITION_LEN];
	float xy[2];
};

struct BinaryLevel {
	bool bitmap[LEVEL_SIZE_X][LEVEL_SIZE_Y];
	unsigned int named_positions_n;
	NamedPosition named_positions[MAX_NAMED_POSITIONS];	
};

class LevelSceneNode : public SceneNode {
	color tile_color;
	bool slantedTiles;
	vector2 lastCameraPos;
	
	ContainerSceneNode* objectsContainer;
	ContainerSceneNode* backgroundContainer;
	ContainerSceneNode* foregroundContainer;
	
	PlayableCharacter* pchar;
	TheRock* rock;
	
	int* checkpointIds;
	vector2** checkpointPositions;
	int checkpoints_n;
	
	int lastVisitedCheckpoint;
	
	bool showNames;
	bool showNamedPositions;
	
	NamedPosStringParser* parser;
	
	float levelTime;
	bool loaded;
	
	Primitive* buildQuadPhysics(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy);
	Primitive* buildTrianglePhysics(float ax, float ay, float bx, float by, float cx, float cy);
	void createLabel(vector2 position, const std::string& text);
	void parseNamedPositionString(const std::string& string);
	
	public:
		float* render_vertex_list;
		int rvl_ptr;
		unsigned char map[LEVEL_SIZE_X][LEVEL_SIZE_Y];
		NamedPosition* named_positions;
		unsigned int named_positions_n;
		std::list<std::string> named_pos_vars;
		std::list<std::string> named_pos_values;
		
		LevelSceneNode(GfxMgr* gfx);
		~LevelSceneNode();
		void populateWorld();
		bool loadFromFile(const std::string& filename, bool setActive=false);
		bool writeToFile(const std::string& filename);
		void update(float frameDelta);
		void redraw();
		bool drawSelf();
		std::list<Primitive*> getGeometry(aabb2 bounds);
		void setShowNames(bool state);
		void setShowNamedPositions(bool state);
		void addNamedPosition(vector2 coords, const std::string& string);
		void delNamedPosition(int index);
		int getNamedPositionsN();
		std::string getNamedPosString(int index);
		std::string getNamedPosValue(int index, const std::string& var_n);
		vector2 getNamedPosPosition(int index);
		SceneNode* getObjectByName(const std::string& name);
		int checkNamedPosArea(aabb2 bb);
		int checkNamedPosTile(int tile_x, int tile_y);
		int findNamedPosition(vector2 coords);
		void computeSlanted();
		void removeSlanted();
		void populateObjects(bool setActive=false);
		void depopulateObjects();
		void clear();
		void getTileByXY(float x, float y, int* result_x, int* result_y);
		vector2 getTileByXY(vector2 coords);
		void fillTile(int x, int y);
		void fillTile(vector2 mousecoords);
		void clearTile(int x, int y);
		void clearTile(vector2 mousecoords);
		void invertTileRegion(int start_x, int end_x, int start_y, int end_y);
		bool isSlope(float x, float y);
		int getTile(int x, int y);
		int getTile(vector2 coords);
		PlayableCharacter* getPlayableCharacter();
		TheRock* getTheRock();
		float getTime();
		void addCheckpoint(int id, vector2 position);
		void setLastVisitedCheckpoint(int id);
		int getLastVisitedCheckpoint();
		vector2 getCheckpointPosition(int id);
		
		bool receiveCollisionEvent(CollisionEvent event);
		void populate_rand();
		void exportLevelInfo(const std::string& filename);
};

#endif

