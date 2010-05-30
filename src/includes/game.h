/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef GAME_H
#define GAME_H

enum GAME_STATE {
	STATE_INIT_GFX,
	STATE_INTRO,
	STATE_MAIN_MENU,
	STATE_LEVEL_EDITOR,
	STATE_CUSTOM_LEVEL,
	STATE_EXIT,
	STATE_DATABASE_READ,
	STATE_FIRST_RUN,
	STATE_TUTORIAL
};

struct DatabaseFile {
	int story_progress;
	int res_x;
	int res_y;
	bool fullscreen;
	int fsaa;
	bool mute_all;
	bool mute_music;	
};

class Game;

class GameEventHandler : public SceneNode {
	Game* game;
	
	public:
	GameEventHandler(Game* game_n);
	~GameEventHandler();
	void update(float frameDelta);
	void receiveKeyEvent(std::list<KeyEvent> events);
};

class Game {
	GAME_STATE state;
	
	GameEventHandler* eventhandler;
	
	bool firstRun;
	
	GfxMgr* gfx;
	UIImage* gameLogo;
	UIMenu* mainMenu;
	UIMenu* loadCustomMenu;
	UIMenu* settingsMenu;
	bool settingsMenuPolled;
	UIMenu* changeResMenu;
	UIMenu* exitMenu;
	UIInfoBox* infobox;
	UICutscene* introCutscene;
	
	int startFromCheckpoint;
	
	GLFWvidmode* video_modes;
	int video_mode_start;
	int video_modes_n;
	
	UILabel* title;
	LevelEditor* editor;
	char* level_filename;
	
	DatabaseFile* db;
	
	public:
	
	void saveDatabase();
	
	void showInfoBox(char* text, float timeOut=0.f, float textSize=24.f, vector2 position=vector2(100.f, 100.f), float margin=10.f, float transparency=1.f, char* delimiter=NULL);
	void hideInfoBox();
	void openMainMenu();
	void openLoadCustomMenu();
	void openSettingsMenu();
	void openChangeResMenu();
	
	void startIntroCutscene();
	void stopIntroCutscene();
		
	void pollMainMenu();
	void pollLoadCustomMenu();
	void pollSettingsMenu();
	void pollChangeResMenu();
	void pollExitMenu();
	
	void enterInitGfxState();
	void enterDatabaseReadState();
	void enterFirstRunState();
	void enterIntroState();
	void enterMainMenuState();
	void enterLevelEditorState();
	void enterCustomLevelState();
	void enterTutorialState();
	
	bool isRunning;
	
	Game();
	~Game();
	GfxMgr* getGfxMgr();
	void changeState(GAME_STATE state_n);
	void nextFrame(float frameDelta);
	bool running();
	GAME_STATE getState();
	void openExitMenu(bool death=false);
	GameEventHandler* getGameEventHandler();
		
};

#endif
