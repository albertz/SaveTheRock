/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "engine/headers.h"

using namespace std;

// GameEventHandler methods

GameEventHandler::GameEventHandler(Game* game_n) {
	game = game_n;
	gfx = game->getGfxMgr();
	game->getGfxMgr()->getInputHandler()->addReceiver(this);
}

GameEventHandler::~GameEventHandler() {}

void GameEventHandler::update(float frameDelta) {
	switch(game->getState()) {
	case STATE_TUTORIAL:
	case STATE_CUSTOM_LEVEL:
		if(gfx->getLevel()) {
			if(gfx->getLevel()->getTheRock()) {
				if(gfx->getLevel()->getTheRock()->getExitState()) {

					char* text = new char[1024];
					sprintf(text, "Time - %.3f seconds\nLevel complete!", gfx->getLevel()->getTime());
					vector2 wparams = gfx->getRenderer()->getWindowParams();
					game->showInfoBox(text, INFOBOX_DEFAULT_TIMEOUT+2.f, 24.f, wparams*0.5f-vector2(50.f, 50.f));
					delete text;
					gfx->reinitLevel();
					game->changeState(STATE_MAIN_MENU);
				}
			} else if(gfx->getLevel()->getPlayableCharacter()) {
				if(gfx->getLevel()->getPlayableCharacter()->getExitState()) {
					char* text = new char[1024];
					sprintf(text, "Time - %.3f seconds\nLevel complete!", gfx->getLevel()->getTime());
					vector2 wparams = gfx->getRenderer()->getWindowParams();
					game->showInfoBox(text, INFOBOX_DEFAULT_TIMEOUT+2.f, 24.f, wparams*0.5f-vector2(50.f, 50.f));
					delete text;
					gfx->reinitLevel();
					game->changeState(STATE_MAIN_MENU);
				}
			}
			if(gfx->getLevel()->getPlayableCharacter()) {
				if(!gfx->getLevel()->getPlayableCharacter()->isAlive()) {
					gfx->getLevel()->getPlayableCharacter()->setActive(false);
					game->openExitMenu(true);
				}
			}
		}
		break;
	}
}

void GameEventHandler::receiveKeyEvent(list<KeyEvent> events) {
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		switch(game->getState()) {
		case STATE_TUTORIAL:
		case STATE_CUSTOM_LEVEL:
			switch((*x).key) {
			case KEY_ESC:
				if((*x).state == GLFW_PRESS) {
					game->openExitMenu();
				}
				break;
			}
			break;
		case STATE_INTRO:
			switch((*x).key) {
			case KEY_SPACE:
				if((*x).state == GLFW_PRESS) {
					game->stopIntroCutscene();
					game->changeState(STATE_MAIN_MENU);
				}
			}
		}
	}
}

// game class methods
Game::Game() {
	isRunning = true;
	level_filename = NULL;
	gameLogo = NULL;
	mainMenu = NULL;
	loadCustomMenu = NULL;
	settingsMenu = NULL;
	changeResMenu = NULL;
	exitMenu = NULL;
	infobox = NULL;
	introCutscene = NULL;

	video_modes = NULL;
	video_modes_n = 0;
	video_mode_start = 0;
	title = NULL;
	editor = NULL;
	level_filename = NULL;
	db = NULL;
	firstRun = false;
	settingsMenuPolled = false;

	startFromCheckpoint = 0;
}

Game::~Game() {

}

void Game::saveDatabase() {
	ofstream fout("db", ios::binary);
	fout.write((char*)(db), sizeof(DatabaseFile));
}

void Game::showInfoBox(const char* text, float timeOut, float textSize, vector2 position, float margin, float transparency, const char* delimiter) {
	if(infobox) gfx->deleteSceneNode(infobox);
	
	if(!delimiter)
		delimiter = "\n";

	infobox = new UIInfoBox(gfx);
	infobox->allowClose(true);
	infobox->setColor(color(MENU_COLOR[0], MENU_COLOR[1], MENU_COLOR[2], transparency));
	infobox->setTimeout(timeOut);
	infobox->setText(margin, textSize, 0);

	char* buffer = new char[1024];
	for(int x=0; x<strlen(text); x++) buffer[x] = text[x];
	buffer[strlen(text)] = 0;
	char* line;
	line = strtok(buffer, delimiter);
	while(line != NULL) {
		infobox->addText(line);
		line = strtok(NULL, delimiter);
	}

	vector2 wparams = gfx->getRenderer()->getWindowParams();

	infobox->setPosition(position);
	gfx->getUI()->addWidget(infobox);
	infobox->fadeIn();
	delete buffer;
}

void Game::hideInfoBox() {
	if(infobox) infobox->fadeOut();
}

void Game::openMainMenu() {
	if(!mainMenu) {
		gfx->getSndMgr()->playRandMusic();
		mainMenu = new UIMenu(gfx);
		mainMenu->setColor(MENU_COLOR);
		mainMenu->setMenu(20.f, 36.f, 0);
		mainMenu->addMenuOption("exit", true);
		mainMenu->addMenuOption("level editor");
		mainMenu->addMenuOption("settings");
		mainMenu->addMenuOption("start level");
		mainMenu->addMenuOption("tutorial");
		gfx->getUI()->addWidget(mainMenu);
		mainMenu->setPosition(vector2(100.f, 100.f));
		mainMenu->fadeIn();
	}

	if(!gameLogo) {
		gameLogo = new UIImage(gfx);
		gameLogo->setImage("GAMELOGO");
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		gameLogo->setPosition(vector2(100.f, wparams[1]-gameLogo->getSize()[1]-50.f));
		gameLogo->setColor(color(1.f, 1.f, 1.f, 0.7f));
		gfx->getUI()->addWidget(gameLogo);
		gameLogo->fadeIn();
	}
}

void Game::openLoadCustomMenu() {
	if(!loadCustomMenu) {
		loadCustomMenu = new UIMenu(gfx);
		loadCustomMenu->setColor(MENU_COLOR);
		loadCustomMenu->setMenu(10.f, 24.f, 0);
		loadCustomMenu->addMenuOption("-back-", true);
		list<char*>* levelslist = gfx->getFilesystem()->getCustomLevels();
		for(list<char*>::iterator x = levelslist->begin(); x != levelslist->end(); x++) {
			loadCustomMenu->addMenuOption(*x);
		}
		gfx->getUI()->addWidget(loadCustomMenu);
		loadCustomMenu->setPosition(vector2(100.f, 100.f));
		loadCustomMenu->fadeIn();
	}
}

void Game::openSettingsMenu() {
	if(!settingsMenu) {
		settingsMenuPolled = false;
		settingsMenu = new UIMenu(gfx);
		settingsMenu->setColor(MENU_COLOR);
		settingsMenu->setMenu(10.f, 24.f, 0);
		settingsMenu->addMenuOption("-back-", true);
		if(db->mute_all)
			settingsMenu->addMenuOption("turn sound on");
		else
			settingsMenu->addMenuOption("turn sound off");
		if(db->mute_music)
			settingsMenu->addMenuOption("turn music on");
		else settingsMenu->addMenuOption("turn music off");
		settingsMenu->addMenuBreak();
		settingsMenu->addMenuOption("change display resolution");
		if(db->fullscreen)
			settingsMenu->addMenuOption("switch to windowed mode");
		else settingsMenu->addMenuOption("switch to fullscreen");
		settingsMenu->addMenuBreak();
		settingsMenu->addMenuOption("antialiasing off", (db->fsaa == 0));
		settingsMenu->addMenuOption("antialiasing x2", (db->fsaa == 2));
		settingsMenu->addMenuOption("antialiasing x4", (db->fsaa == 4));

		gfx->getUI()->addWidget(settingsMenu);
		settingsMenu->setPosition(vector2(100.f, 100.f));
		settingsMenu->fadeIn();
	}
}

void Game::openChangeResMenu() {
	if(!changeResMenu) {
		changeResMenu = new UIMenu(gfx);
		changeResMenu->setColor(MENU_COLOR);
		changeResMenu->setMenu(10.f, 24.f, 0);
		changeResMenu->addMenuOption("-back-", true);
		if(video_modes) delete [] video_modes;
		video_modes = new GLFWvidmode[MAX_VIDEO_MODES];
		video_modes_n = glfwGetVideoModes(video_modes, MAX_VIDEO_MODES);
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		video_mode_start = 0;
		char* str;
		int y;
		for(y=0; y<video_modes_n; y++) {
			if(video_modes[y].Width > 800 && video_modes[y].Height > 600) break;
			video_mode_start++;
		}
		y = 0;
		for(int x=video_mode_start; x<video_modes_n; x++) {
			if(y > MAX_DISPLAYED_VIDEO_MODES) break;
			str = new char[64];
			bool selected = (video_modes[x].Width == (int)wparams[0] && video_modes[x].Height == (int)wparams[1]);
			sprintf(str, "%ix%i", video_modes[x].Width, video_modes[x].Height);
			changeResMenu->addMenuOption(str, selected);
			delete [] str;
			y++;
		}

	//	delete [] video_modes;
		gfx->getUI()->addWidget(changeResMenu);
		changeResMenu->setPosition(vector2(100.f, 100.f));
		changeResMenu->fadeIn();
	}
}

void Game::openExitMenu(bool death) {
	if(!exitMenu) {
		gfx->setCursor(false);
		exitMenu = new UIMenu(gfx);
		exitMenu->setColor(MENU_COLOR);
		exitMenu->setMenu(20.f, 32.f, 0, false);
		switch(state) {
		case STATE_TUTORIAL:
		case STATE_CUSTOM_LEVEL:
			exitMenu->addMenuOption("exit");
			exitMenu->addMenuOption("restart");
			exitMenu->addMenuOption("restart from checkpoint");
			if(!death)
				exitMenu->addMenuOption("resume");
			break;
		}

		gfx->getUI()->addWidget(exitMenu);
		vector2 wcenter = gfx->getRenderer()->getWindowParams() * 0.5f;
		vector2 position = wcenter - exitMenu->getSize()*0.5f;

		exitMenu->setPosition(position);
		exitMenu->fadeIn();
		if(death) showInfoBox("You are dead.", INFOBOX_DEFAULT_TIMEOUT+100.f, 32.f, position + vector2(-10.f, 150.f));
	}
}

void Game::startIntroCutscene() {
	gfx->recomputeBackground();
	gfx->getUI()->darkenScreen(1.f);
	if(introCutscene) gfx->deleteSceneNode(introCutscene);
	introCutscene = new UICutscene(gfx);

	vector2 wparams = gfx->getRenderer()->getWindowParams();
	vector2 center = wparams*0.5f;

	/*** FRAME 0 ***/
	introCutscene->startFrame(3.f);

	UILabel* text = new UILabel(gfx);
	text->setColor(color(0.7f, 0.7f, 0.7f, 0.f));
	text->setText("powered by", 0, 32.f);
	text->setPosition(center+vector2(-150.f, 100.f));
//	text->fadeOut();

	UIImage* gllogo = new UIImage(gfx);
	gllogo->setImage("LOGOS");
	gllogo->setPosition(vector2(center[0]-100.f, center[1]-gllogo->getSize()[1]+100.f));
//	gllogo->fadeOut();
	introCutscene->addWidgetToFrame(text);
	introCutscene->addWidgetToFrame(gllogo);

	introCutscene->endFrame();

	/*** FRAME 1 ***/

	introCutscene->startFrame(6.f);

	text = new UILabel(gfx);
	text->setColor(color(0.7f, 0.7f, 0.7f, 0.f));
//	text->setText("Copyright 2010 Alexander Dzhoganov", 0, 32.f);
	text->setText(GNU_COPYRIGHT_NOTICE, 0, 20.f);
	text->setPosition(vector2(center[0]-400.f, center[1]));
	text->fadeOut();
	introCutscene->addWidgetToFrame(text);

	UILabel* skiplabel = new UILabel(gfx);
	skiplabel->setColor(color(0.4f, 0.4f, 0.4f, 0.f));
	skiplabel->setText("press  [space ] to skip intro", 0, 16.f);
	skiplabel->setPosition(vector2(wparams[0]-320.f, 50.f));

	introCutscene->addWidgetToFrame(skiplabel);

	introCutscene->endFrame();

	/*** FRAME 2 ***/

	introCutscene->startFrame(3.f);

	text = new UILabel(gfx);
	text->setColor(color(0.7f, 0.7f, 0.7f, 0.f));
	text->setText("a game by Alexander Dzhoganov", 0, 32.f);
	text->setPosition(center);
	text->fadeOut();
	introCutscene->addWidgetToFrame(text);

	introCutscene->endFrame();

	/*** FRAME 3 ***/

	introCutscene->startFrame(3.f);

	text = new UILabel(gfx);
	text->setColor(color(0.7f, 0.7f, 0.7f, 0.f));
	text->setText("programming by:\nAlexander Dzhoganov\n\nart by:\nViktor Popov", 0, 32.f);
	text->setPosition(center);
	text->fadeOut();
	introCutscene->addWidgetToFrame(text);

	introCutscene->endFrame();


	gfx->getUI()->addWidget(introCutscene);
}

void Game::stopIntroCutscene() {
	if(introCutscene) gfx->deleteSceneNode(introCutscene);
	introCutscene = NULL;
	gfx->getUI()->darkenScreenFadeOut();
}

void Game::pollMainMenu() {
	int status;
	if(mainMenu) status = mainMenu->pollStatus();
	else status = 0;
	switch(status) {
	case -1:
		gfx->deleteSceneNode(mainMenu);
		mainMenu = NULL;
		gfx->deleteSceneNode(gameLogo);
		gameLogo = NULL;
		break;
	case 0:
		break;
	case 1:
		// exit
		changeState(STATE_EXIT);
		break;
	case 2:
		// level editor
		gfx->deleteSceneNode(mainMenu);
		mainMenu = NULL;
		gfx->deleteSceneNode(gameLogo);
		gameLogo = NULL;
		changeState(STATE_LEVEL_EDITOR);
		break;
	case 3:
		// settings
		openSettingsMenu();
		break;
	case 4:
		// custom levels
		openLoadCustomMenu();
		break;
	case 5:
		// tutorial
		gfx->deleteSceneNode(mainMenu);
		mainMenu = NULL;
		gfx->deleteSceneNode(gameLogo);
		gameLogo = NULL;
		changeState(STATE_TUTORIAL);
		break;
	}
}

void Game::pollLoadCustomMenu() {
	int status;
	if(loadCustomMenu) status = loadCustomMenu->pollStatus();
	else status = 0;
	char* filename;
	if(status == -1) {
		gfx->deleteSceneNode(loadCustomMenu);
		loadCustomMenu = NULL;
	} else if(status == 1) {
		changeState(STATE_MAIN_MENU);
	} else {
		int i = 1;
		list<char*>* levels = gfx->getFilesystem()->getCustomLevels();
		for(list<char*>::iterator x = levels->begin(); x != levels->end(); x++) {
			if(i == status-1) {
				filename = new char[strlen(*x)];
				for(int p=0; p<strlen(*x); p++) filename[p] = (*x)[p];
				filename[strlen(*x)] = 0;
				level_filename = filename;
				changeState(STATE_CUSTOM_LEVEL);
				return;
			}
			i++;
		}
	}
}

void Game::pollSettingsMenu() {
	int status;
	if(settingsMenu) status = settingsMenu->pollStatus();
	else status = 0;
	char* filename;

	if(status == -1) {
		gfx->deleteSceneNode(settingsMenu);
		settingsMenu = NULL;
		if(!changeResMenu)
			changeState(STATE_MAIN_MENU);
	}

	if(settingsMenuPolled) return;

	if(status != 0) settingsMenuPolled = true;

	switch(status) {
	case 1:
		changeState(STATE_MAIN_MENU);
		break;
	case 2:
		db->mute_all = !db->mute_all;
		gfx->getSndMgr()->muteAll(db->mute_all);
		gfx->getSndMgr()->playRandMusic();
		saveDatabase();
		break;
	case 3:
		db->mute_music = !db->mute_music;
		gfx->getSndMgr()->muteMusic(db->mute_music);
		gfx->getSndMgr()->playRandMusic();
		saveDatabase();
		break;
	case 5:
		openChangeResMenu();
		break;
	case 6:
		db->fullscreen = !db->fullscreen;
		saveDatabase();
		showInfoBox("You must restart the game for the changes to take effect.", 2.f, 24.f, vector2(100.f, 400.f), 5.f);
		settingsMenuPolled = true;
		break;
	case 8:
		db->fsaa = 0;
		saveDatabase();
		showInfoBox("You must restart the game for the changes to take effect.", 2.f, 24.f, vector2(100.f, 400.f), 5.f);
		break;
	case 9:
		db->fsaa = 2;
		saveDatabase();
		showInfoBox("You must restart the game for the changes to take effect.", 2.f, 24.f, vector2(100.f, 400.f), 5.f);
		break;
	case 10:
		db->fsaa = 4;
		saveDatabase();
		showInfoBox("You must restart the game for the changes to take effect.", 2.f, 24.f, vector2(100.f, 400.f), 5.f);
		break;
	}
}

void Game::pollChangeResMenu() {
	int status;
	if(changeResMenu) status = changeResMenu->pollStatus();
	else status = 0;
	if(status == -1) {
		gfx->deleteSceneNode(changeResMenu);
		changeResMenu = NULL;
	} else if(status == 1) {
		changeState(STATE_MAIN_MENU);
	} else if(status > 1) {
		db->res_x = video_modes[status+video_mode_start-2].Width;
		db->res_y = video_modes[status+video_mode_start-2].Height;
		saveDatabase();
		changeState(STATE_MAIN_MENU);
		showInfoBox("You must restart the game for the changes to take effect.", 2.f, 24.f, vector2(100.f, 400.f), 5.f);
	}
}

void Game::pollExitMenu() {
	int status;
	if(exitMenu) status = exitMenu->pollStatus();
	else status = 0;

	if(status == -1) {
		gfx->deleteSceneNode(exitMenu);
		exitMenu = NULL;
	//	gfx->setCursor(true);
	} else if(status == 1) {
		gfx->reinitLevel();
		changeState(STATE_MAIN_MENU);
	} else if(status == 2) {
		if(state == STATE_CUSTOM_LEVEL)
			changeState(STATE_CUSTOM_LEVEL);
		else if(state == STATE_TUTORIAL) {
			changeState(STATE_TUTORIAL);
		}
	} else if(status == 3) {
		if(state == STATE_CUSTOM_LEVEL) {
			if(startFromCheckpoint == 0)
				startFromCheckpoint = gfx->getLevel()->getLastVisitedCheckpoint();
			changeState(STATE_CUSTOM_LEVEL);
		} else if(state == STATE_TUTORIAL) {
			if(startFromCheckpoint == 0)
				startFromCheckpoint = gfx->getLevel()->getLastVisitedCheckpoint();
			changeState(STATE_TUTORIAL);
		}
	} else if(status == 4) {
		gfx->setCursor(false);
	}

	if(status != 0) hideInfoBox();
}

void Game::enterInitGfxState() {
	gfx = new GfxMgr(this);
	gfx->init(db->res_x, db->res_y, db->fullscreen, false, db->fsaa);
	gfx->getSndMgr()->muteAll(db->mute_all);
	gfx->getSndMgr()->muteMusic(db->mute_music);

	eventhandler = new GameEventHandler(this);
	gfx->addSceneNode(eventhandler, gfx->getScene());
	changeState(STATE_INTRO);
}

void Game::enterIntroState() {
//	gfx->getSndMgr()->playRandMusic();
	startIntroCutscene();
//	changeState(STATE_MAIN_MENU);
}

void Game::enterDatabaseReadState() {
	ifstream fin("db", ios::binary);
	if(!fin) {
		firstRun = true;
		changeState(STATE_FIRST_RUN);
		return;
	}
	db = new DatabaseFile;
	fin.read((char*)(db), sizeof(DatabaseFile));
	changeState(STATE_INIT_GFX);
}

void Game::enterFirstRunState() {
	ofstream fout("db", ios::binary);

	db = new DatabaseFile;
#ifdef _WIN32
	db->res_x = GetSystemMetrics(SM_CXSCREEN);
	db->res_y = GetSystemMetrics(SM_CYSCREEN);
#else
	db->res_x = 1024;
	db->res_y = 768;
#endif
#ifdef __APPLE__
	db->fullscreen = false;
#else
	db->fullscreen = true;
#endif
	db->fsaa = 0;
	db->story_progress = 0;
	db->mute_all = false;
	db->mute_music = false;

	fout.write((char*)(db), sizeof(DatabaseFile));
	changeState(STATE_INIT_GFX);
}

void Game::enterMainMenuState() {
	gfx->getUI()->darkenScreenFadeOut();
	gfx->setCursor(true);
	gfx->getCamera()->setMovement(vector2(1.f, 0.f), 100.f);
	gfx->getCamera()->startMovement();
	startFromCheckpoint = 0;
	openMainMenu();
}

void Game::enterLevelEditorState() {
	gfx->getCamera()->setTranslation(vector2(1.f, 1.f));
	gfx->getCamera()->stopMovement();
	gfx->recomputeBackground();
	gfx->reinitLevel();
	gfx->getPhysMgr()->stopSimulation();
	editor = new LevelEditor(gfx);
	editor->init();
	editor->setActive(true);
	gfx->addSceneNode(editor, gfx->getCamera());
	gfx->getInputHandler()->addReceiver(gfx->getCamera());
	gfx->getCamera()->setKeyboardControl(true);
	gfx->getLevel()->setShowNamedPositions(true);
	gfx->getLevel()->setActive(true);
}

void Game::enterCustomLevelState() {
	gfx->getUI()->darkenScreen(1.f);
	gfx->getCamera()->setTranslation(vector2(1.f, 1.f));
	gfx->getCamera()->stopMovement();
	gfx->setCursor(false);
	gfx->recomputeBackground();
	gfx->reinitLevel();
	gfx->getPhysMgr()->setIterations(MIN_PHYS_ITERATIONS);
	gfx->getLevel()->loadFromFile(gfx->getFilesystem()->getLevelFilename(level_filename), true);
	gfx->getLevel()->setActive(true);
	gfx->getCamera()->setKeyboardControl(false);
	gfx->getPhysMgr()->startSimulation();
	if(startFromCheckpoint > 0) {
		Primitive* hero = gfx->getLevel()->getPlayableCharacter()->getCollisionPrimitive();
		hero->vertices[0] = gfx->getLevel()->getCheckpointPosition(startFromCheckpoint);
		hero->vertices_oldPos[0] = hero->vertices[0];
		hero->center = hero->vertices[0];
		startFromCheckpoint = 0;
	}
}

void Game::enterTutorialState() {
	gfx->getUI()->darkenScreen(1.f);
	gfx->setCursor(false);
	gfx->recomputeBackground();
	gfx->reinitLevel();
	gfx->getLevel()->loadFromFile(gfx->getFilesystem()->getLevelFilename("tutorial", true), true);
	gfx->getLevel()->setActive(true);
	gfx->getCamera()->setKeyboardControl(false);
	gfx->getPhysMgr()->startSimulation();
	if(startFromCheckpoint > 0) {
		Primitive* hero = gfx->getLevel()->getPlayableCharacter()->getCollisionPrimitive();
		hero->vertices[0] = gfx->getLevel()->getCheckpointPosition(startFromCheckpoint);
		hero->vertices_oldPos[0] = hero->vertices[0];
		hero->center = hero->vertices[0];
		startFromCheckpoint = 0;
	}
}

GfxMgr* Game::getGfxMgr() {
	return gfx;
}

void Game::changeState(GAME_STATE state_n) {
	state = state_n;
	switch(state) {
	case STATE_INIT_GFX:
		enterInitGfxState();
		break;
	case STATE_INTRO:
		enterIntroState();
		break;
	case STATE_MAIN_MENU:
		enterMainMenuState();
		break;
	case STATE_LEVEL_EDITOR:
		enterLevelEditorState();
		break;
	case STATE_CUSTOM_LEVEL:
		enterCustomLevelState();
		break;
	case STATE_EXIT:
		exit(0);
		break;
	case STATE_DATABASE_READ:
		enterDatabaseReadState();
		break;
	case STATE_FIRST_RUN:
		enterFirstRunState();
		break;
	case STATE_TUTORIAL:
		enterTutorialState();
		break;
	}
}

void Game::nextFrame(float frameDelta) {
	if(state == STATE_MAIN_MENU) {
		pollMainMenu();
		pollLoadCustomMenu();
		pollSettingsMenu();
		pollChangeResMenu();
	} else if(state == STATE_LEVEL_EDITOR) {
		if(!editor->isActive()) {
			gfx->getLevel()->setActive(false);
			gfx->deleteSceneNode(editor);
			editor = NULL;
			changeState(STATE_MAIN_MENU);
		}
	} else if(state == STATE_INTRO) {
		if(introCutscene)
			if(introCutscene->pollStatus()) {
				stopIntroCutscene();
				gfx->getUI()->darkenScreen(1.f);
				changeState(STATE_MAIN_MENU);
			}
	}

	pollExitMenu();

	gfx->frameStart();
	gfx->updateScene(frameDelta);
	gfx->frameEnd();

}

bool Game::running() {
	return isRunning;
}

GAME_STATE Game::getState() {
	return state;
}

GameEventHandler* Game::getGameEventHandler() {
	return eventhandler;
}

// TODO move out once we overtake OLX utils code
static size_t findLastPathSep(const std::string& path) {
	size_t slash = path.rfind('\\');
	size_t slash2 = path.rfind('/');
	if(slash == std::string::npos)
		slash = slash2;
	else if(slash2 != std::string::npos)
		slash = std::max(slash, slash2);
	return slash;
}

int main(int argc, char** argv) {
#ifdef __APPLE__
	std::string binary_dir;
	if(argc >= 1) {
		binary_dir = argv[0];
		size_t slashpos = findLastPathSep(binary_dir);
		if(slashpos != std::string::npos)  {
			binary_dir.erase(slashpos);
			//binary_dir = SystemNativeToUtf8(binary_dir);
		} else
			binary_dir = ".";
	} else {
		std::cout << "Warning: Binary-argument not given, assuming current dir" << std::endl;
		binary_dir = ".";
	}
	
	std::string mediadir = binary_dir + "/../Resources/media";
	std::cout << "Media dir: " << mediadir << std::endl;
	chdir(mediadir.c_str());
#endif
	
	srand(time(NULL));

	float frameDelta = 0.f;
	double lastFrameTime = 0.f;
	double currentTime = 0.f;
	int fps = 0, lastfps = 0, framecount = 0, fpsdiff = 0;
	double fpstest = 0.f;

	Game* game = new Game();
	game->changeState(STATE_DATABASE_READ);
	char title[40];
	char fpstext[10];

	int phys_iterations = DEFAULT_PHYS_ITERATIONS;
	int phys_threshold = phys_iterations * 10;

	glfwSetTime(0.f);
	// MAIN GAME LOOP
	bool running = GL_TRUE;
	while(game->running() && running) {
		game->getGfxMgr()->setWindowTitle(title);
		currentTime = glfwGetTime();
		frameDelta = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		game->nextFrame(frameDelta);

		// FPS COUNT
		framecount++;
		if(fpstest < currentTime) {
			fpstest = currentTime + 1.f;
			lastfps = fps;
			fps = framecount;
			fpsdiff = fps - lastfps;

			if(fpsdiff < 0)
				phys_threshold += fpsdiff/5;
			else if(fpsdiff > 10)
				phys_threshold += 10;

			/* adjust physics iterations based on current fps */
			if(phys_threshold < MIN_PHYS_ITERATIONS * 10) phys_threshold = MIN_PHYS_ITERATIONS * 10;
			if(phys_threshold > MAX_PHYS_ITERATIONS * 10) phys_threshold = MAX_PHYS_ITERATIONS * 10;
			phys_iterations = phys_threshold / 10;
			game->getGfxMgr()->getPhysMgr()->setIterations(phys_iterations);

			sprintf(fpstext, "%i", fps);
			for(int x=0;x<10;x++) title[x]=0;
			strcat(title, "SaveThisRock! ");
			strcat(title, "FPS: ");
			strcat(title, fpstext);
			framecount = 0;
		}
		running = running && glfwGetWindowParam(GLFW_OPENED);
	}
}

