/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __LEVEL_EDITOR_H
#define __LEVEL_EDITOR_H

enum BRUSH {
	BRUSH_NONE,
	BRUSH_FILL_TILE,
	BRUSH_CLEAR_TILE	
};

enum LEVEL_EDITOR_STATE {
	STATE_NONE,
	STATE_ADDING_NAMED_POS,
	STATE_DELETING_NAMED_POS,
	STATE_MOVING_NAMED_POS,
	STATE_SAVING_LEVEL,
	STATE_LOADING_LEVEL,
	STATE_PREVIEW_LEVEL,
	STATE_INVERTING_ZONE
};

class LevelEditor : public SceneNode {
	GfxMgr* gfx;
	
	LEVEL_EDITOR_STATE state;
	BRUSH currentBrush;

	std::string filename; // current opened level filename
	
	bool settingDestination;
	bool settingName;
	
	int numObjects;
	int nameVarLen;
	
	bool showSlanted;
	bool showingSlanted;
	
	bool drawing;
/*	bool addingNamedPos;
	bool deletingNamedPos;*/
	
	GAME_OBJECT_TYPE namedPosType;
	std::string namedPosName;
	std::string namedPosString;
	int namedPos_sprite_texid;
	Primitive* namedPos_primitive;
	bool namedPos_wallmount;
	int playableChar;
	
	vector2 invertZoneStart;
	
	/* switches */
	bool laserdoorswitch;
	/**/
	
	UIMenu* editMenu;
	UIMenu* loadMenu;
	UIMenu* objectsMenu;
	UIInputBox* inputBox;
	UIInfoBox* infobox;
	
	bool infobox_nps;
	int nps_lastindex;
	
	UILabel* label;
	
	bool saveInputBox;
	
	void showLabel(const std::string& text);
	void hideLabel();
	 
	void showInfoBox(const std::string& text, float timeOut=0.f, float textSize=24.f, vector2 position=vector2(100.f, 100.f), float margin=10.f, float transparency=1.f, const char* delimiter=NULL);
	void hideInfoBox();
	void openInputBox(const std::string& title, const std::string& text=NULL, bool alphanumeric=true);
	
	void openEditMenu(vector2 position);
	void openLoadMenu(vector2 position);
	void openObjectsMenu(vector2 position);
	
	void openLaserAddMenu();
	
	void pollEditMenu(float frameDelta);
	void pollLoadMenu(float frameDelta);
	void pollObjectsMenu(float frameDelta);
	void pollInputBox(float frameDelta);
	void pollInputBoxSaveLevel(const std::string& input_string);
	void pollInputBoxAddingNamedPos(const std::string& input_string);
	void pollInputBoxExportingLevelInfo(std::string input_string);
	
	void doDrawing(float frameDelta);
	void drawCursor(float frameDelta);
	bool checkNamedPosCollision(vector2 displacement, bool doLevelTest=true, bool doObjectsTest=true);
	bool checkTileCollision(vector2 displacement);
	void startAddNamedPos(const std::string& name);
	void addNamedPosVar(const std::string& var, const std::string& value);

	
	public:
	LevelEditor(GfxMgr* gfx_n);
	~LevelEditor();
	void init();
	void update(float frameDelta);
	void receiveKeyEvent(std::list<KeyEvent> events);
	void receiveKeyEvent_moveNamedPos(KeyEvent event);
	void receiveKeyEvent_addNamedPos(KeyEvent event);
	LevelSceneNode* getLevel();
};

#endif
