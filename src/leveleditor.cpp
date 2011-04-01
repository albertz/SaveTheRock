/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


/* Level Editor
A simple level editor for the game.
*/

#include "engine/headers.h"

using namespace std;

LevelEditor::LevelEditor(GfxMgr* gfx_n) {
	type = SN_TYPE_LEDITOR;
	gfx = gfx_n;
	filename = NULL;
	currentBrush = BRUSH_NONE;	
	showSlanted = true;
	showingSlanted = true;
	settingDestination = false;
	settingName = false;
	drawing = false;
	state = STATE_NONE;
	
	/*addingNamedPos = false;
	deletingNamedPos = false;*/
	namedPosType = GO_TYPE_NONE;
	namedPosString = NULL;
	namedPos_sprite_texid = -1;
	namedPos_primitive = NULL;
	namedPos_wallmount = false;
	nameVarLen = 0;
	
	
	editMenu = NULL;
	inputBox = NULL;
	saveInputBox = false;
	loadMenu = NULL;
	label = NULL;
	objectsMenu = NULL;
	infobox = NULL;
	gfx->reinitLevel();
	playableChar = -1;
	
	// switches
	laserdoorswitch = false;
	
	state = STATE_NONE;
	_markForDeletion = 0;
	infobox_nps = false;
	nps_lastindex = -1;
}

LevelEditor::~LevelEditor() {
	gfx->getInputHandler()->deleteReceiver(this);	
}

void LevelEditor::showLabel(const char* text) {
	if(label) gfx->deleteSceneNode(label);
	label = new UILabel(gfx);
	label->setColor(MENU_TEXT_COLOR);
	label->setText(text, 0, 26.f);
	vector2 wparams = gfx->getRenderer()->getWindowParams();
	vector2 position = vector2(50.f, wparams[1] - 100.f);
	label->setPosition(position);
	label->setActive(true);
	gfx->addSceneNode(label, this);
}

void LevelEditor::hideLabel() {
	if(label) {
		gfx->deleteSceneNode(label);
		label = NULL;
	}
}

void LevelEditor::showInfoBox(const char* text, float timeOut, float textSize, vector2 position, float margin, float transparency, const char* delimiter) {
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
	
//	position -= gfx->getCamera()->getAbsoluteTranslation();
	
	if(timeOut > 0) 
		infobox->setPosition(vector2(position[0], wparams[1]-position[1]));
	else infobox->setPosition(position);
	gfx->getUI()->addWidget(infobox);
	infobox->fadeIn();
	delete [] buffer;
}

void LevelEditor::hideInfoBox() {
//	if(infobox) gfx->deleteSceneNode(infobox);
	//infobox = NULL;
	infobox->fadeOut();	
}

void LevelEditor::openInputBox(const char* title, const char* text, bool alphanumeric) {
	if(inputBox) delete inputBox;
	inputBox = new UIInputBox(gfx);
	inputBox->setPosition(vector2(100.f, 100.f));
	inputBox->setInputBox(title, alphanumeric);
	if(text)
		inputBox->setText(text);
	inputBox->setColor(MENU_COLOR);
	gfx->getUI()->addWidget(inputBox);
	inputBox->fadeIn();
}

void LevelEditor::openEditMenu(vector2 position) {
	if(!editMenu) {
		editMenu = new UIMenu(gfx);
		editMenu->allowClose(true);
		editMenu->setColor(MENU_COLOR);
		editMenu->setMenu(10.f, 24.f, 0);

		editMenu->addMenuOption("invert zone <Z>");
		editMenu->addMenuOption("clear <C>", (currentBrush == BRUSH_CLEAR_TILE));
		editMenu->addMenuOption("draw <D>", (currentBrush == BRUSH_FILL_TILE));
		editMenu->addMenuBreak();
		editMenu->addMenuOption("move/copy object", (state == STATE_MOVING_NAMED_POS));
		editMenu->addMenuOption("delete object <S>", (state == STATE_DELETING_NAMED_POS));
		editMenu->addMenuOption("add object <A>", (state == STATE_ADDING_NAMED_POS));
		editMenu->addMenuOption("preview level <P>");
		editMenu->addMenuBreak();
		editMenu->addMenuOption("show/hide slanted");
		editMenu->addMenuOption("clear level");
		editMenu->addMenuBreak();
		editMenu->addMenuOption("exit editor");
		editMenu->addMenuBreak();
		editMenu->addMenuOption("load");
		editMenu->addMenuOption("save <Q>");
		editMenu->addMenuOption("export level info");

		vector2 size = editMenu->getSize();
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		
		if(position[0] + size[0] > wparams[0]) {
			position.set(position[0]-size[0], position[1]);	
		}
		if(position[1] + size[1] > wparams[1]) {
			position.set(position[0], position[1]-size[1]);
		}

		editMenu->setPosition(position);
		gfx->getUI()->addWidget(editMenu);
		editMenu->fadeIn();
	}
}

void LevelEditor::openLoadMenu(vector2 position) {
	if(!loadMenu) {
		loadMenu = new UIMenu(gfx);
		loadMenu->allowClose(true);
		loadMenu->setColor(MENU_COLOR);
		loadMenu->setMenu(10.f, 24.f, 0);
		
		list<std::string> levelslist = gfx->getFilesystem()->getCustomLevels();
		for(list<std::string>::iterator x = levelslist.begin(); x != levelslist.end(); x++) {
			loadMenu->addMenuOption(x->c_str());	
		}
		
		vector2 size = loadMenu->getSize();
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		
		if(position[0] + size[0] > wparams[0]) {
			position.set(position[0]-size[0], position[1]);	
		}
		if(position[1] + size[1] > wparams[1]) {
			position.set(position[0], position[1]-size[1]);
		}

		loadMenu->setPosition(position);
		gfx->getUI()->addWidget(loadMenu);
		loadMenu->fadeIn();
	}
}

void LevelEditor::openObjectsMenu(vector2 position) {
	if(!objectsMenu) {
		objectsMenu = new UIMenu(gfx);
		objectsMenu->allowClose(true);
		objectsMenu->setColor(MENU_COLOR);
		objectsMenu->setMenu(10.f, 24.f, 0);
		objectsMenu->addMenuOption("crate");
		objectsMenu->addMenuOption("hero");
		objectsMenu->addMenuOption("wall laser");
		objectsMenu->addMenuOption("robot guard");
		objectsMenu->addMenuOption("switch");
		objectsMenu->addMenuOption("pressure pad");
		objectsMenu->addMenuOption("proximity mine");
		objectsMenu->addMenuOption("level exit");
		objectsMenu->addMenuOption("text");
		objectsMenu->addMenuOption("gravity lift");
		objectsMenu->addMenuOption("the rock");
		objectsMenu->addMenuOption("checkpoint");
		
		vector2 size = objectsMenu->getSize();
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		
		if(position[0] + size[0] > wparams[0]) {
			position.set(position[0]-size[0], position[1]);	
		}
		if(position[1] + size[1] > wparams[1]) {
			position.set(position[0], position[1]-size[1]);
		}

		objectsMenu->setPosition(position);
		gfx->getUI()->addWidget(objectsMenu);
		objectsMenu->fadeIn();
	}
}

void openLaserAddMenu(vector2 position) {
	
}

void LevelEditor::pollEditMenu(float frameDelta) {
	float mouse_x, mouse_y;
	int menuStatus;
	if(!editMenu) menuStatus = 0;
	else menuStatus = editMenu->pollStatus();
	if(menuStatus != 0) {
		switch(menuStatus) {
		case -1:
			gfx->deleteSceneNode(editMenu);
			editMenu = NULL;
			break;
		case 1:
			// invert zone
			state = STATE_INVERTING_ZONE;
			settingDestination = false;
			showInfoBox("Please, drag along the zone you wish to invert", INFOBOX_DEFAULT_TIMEOUT);
			break;
		case 2:
			// clear tiles
			currentBrush = BRUSH_CLEAR_TILE;
			state = STATE_NONE;
			showInfoBox("right click - cancel\nleft click - clear\nset brush: clear tiles", INFOBOX_DEFAULT_TIMEOUT);
			break;
		case 3:
			// draw tiles
			state = STATE_NONE;
			currentBrush = BRUSH_FILL_TILE;	
			showInfoBox("right click - cancel\nleft click - draw\nset brush: draw tiles", INFOBOX_DEFAULT_TIMEOUT);
			break;
		case 4:
			break;
		case 5:
			// move object
			showInfoBox("Sorry, not yet implemented!", INFOBOX_DEFAULT_TIMEOUT);
		/*	state = STATE_MOVING_NAMED_POS;
			showInfoBox("right click - cancel\nleft click - move object\nset brush: move object", INFOBOX_DEFAULT_TIMEOUT);
		*/
			break;
		case 6:
			// del object
			state = STATE_DELETING_NAMED_POS;
			currentBrush = BRUSH_NONE;
			showInfoBox("right click - cancel\nleft click - delete object\nset brush: delete object", INFOBOX_DEFAULT_TIMEOUT);
			break;
		case 7:
			// add object;
			state = STATE_NONE;
			currentBrush = BRUSH_NONE;
			openObjectsMenu(gfx->getMousePos(true));
			break;
		case 8:
			// preview level
			hideInfoBox();
			if(!gfx->getLevel()->getPlayableCharacter()) {
				showInfoBox("Use \"add object\" to add new objects.\nCan't preview the level without a hero in it.\nSorry!", INFOBOX_DEFAULT_TIMEOUT+3.f);	
				break;
			}
			state = STATE_PREVIEW_LEVEL;
			gfx->getLevel()->populateObjects(true);
			gfx->getLevel()->computeSlanted();
			gfx->getLevel()->setShowNamedPositions(false);
			gfx->getCamera()->setKeyboardControl(false);
			gfx->getPhysMgr()->startSimulation();
			showLabel("Previewing level - press <P> to return");
			break;
		case 9:
			break;
		case 10:
			// show/hide slanted
			if(!showingSlanted) {	
				gfx->getLevel()->computeSlanted();
				showingSlanted = true;
			} else {
				gfx->getLevel()->removeSlanted();
				showingSlanted = false;	
			}
			break;
		case 11:
			// clear level
			gfx->getLevel()->clear();
			break;
		case 12:
		break;
		case 13:
			// exit editor
			gfx->reinitLevel();
			setActive(false);
			break;
		case 14:
			break;
		case 15:
			// load level
			state = STATE_LOADING_LEVEL;
			openLoadMenu(gfx->getMousePos(true));
			break;	
		case 16:
			// save level
			if(!gfx->getLevel()->getPlayableCharacter()) {
				showInfoBox("Use \"add object\" to add new objects.\nCan't save the level without a hero in it.\nSorry!", INFOBOX_DEFAULT_TIMEOUT+3.f);	
				break;
			}
			state = STATE_SAVING_LEVEL;
			openInputBox("Enter level name:", filename);
			saveInputBox = true;
			break;
		case 17:
			// export level info
			if(!filename) break;
			gfx->getLevel()->exportLevelInfo(filename);	
			showInfoBox("Level info exported!", INFOBOX_DEFAULT_TIMEOUT);
			break;
		}
		if(editMenu) editMenu->resetStatus();
	}	
}

void LevelEditor::pollLoadMenu(float frameDelta) {
	int loadMenuStatus;
	if(!loadMenu) loadMenuStatus = 0;
	else loadMenuStatus = loadMenu->pollStatus();
	if(loadMenuStatus != 0) {
		if(loadMenuStatus == -1) {
			gfx->deleteSceneNode(loadMenu);
			loadMenu = NULL;	
		} else {
			int i = 1;
			std::list<std::string> levels = gfx->getFilesystem()->getCustomLevels();
			for(list<std::string>::iterator x = levels.begin(); x != levels.end(); x++) {
				if(i == loadMenuStatus) {
					filename = new char[x->size()];
					for(int p=0; p<x->size(); p++) filename[p] = (*x)[p];
					filename[x->size()] = 0;
			//		gfx->getLevel()->setShowNames(true);
					gfx->getLevel()->loadFromFile(gfx->getFilesystem()->getLevelFilename(filename).c_str());
					char* buffer = new char[MAX_TEXT_BUFFER];
					sprintf(buffer, "Loaded \"%s\"..", filename);
					showInfoBox(buffer, INFOBOX_DEFAULT_TIMEOUT);
					delete [] buffer;	
					state = STATE_NONE;
				}
				i++;	
			}
		}
		if(loadMenu) loadMenu->resetStatus();	
	}
}

void LevelEditor::pollObjectsMenu(float frameDelta) {
	bool infobox_used = false;
	int objectsMenuStatus;
	if(!objectsMenu) objectsMenuStatus = 0;
	else objectsMenuStatus = objectsMenu->pollStatus();
	if(objectsMenuStatus != 0) {
		if(objectsMenuStatus == -1) {
			gfx->deleteSceneNode(objectsMenu);
			objectsMenu = NULL;	
		} else {
			namedPos_wallmount = false;
			if(!namedPos_primitive) namedPos_primitive = new Primitive;
			if(objectsMenuStatus == 1) {
				namedPosType = GO_TYPE_RAGDOLL;
				namedPosString = "OBJ_CRATE";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				startAddNamedPos("CRATE");
			} else if(objectsMenuStatus == 2) {
				if(gfx->getLevel()->getPlayableCharacter()) {
					vector2 coords = gfx->getLevel()->getPlayableCharacter()->getCollisionPrimitive()->center;
					gfx->getLevel()->delNamedPosition(
						gfx->getLevel()->findNamedPosition(coords));
				} 
				namedPosType = GO_TYPE_CHAR;
				namedPosString = "OBJ_HERO";
				namedPos_primitive->type = PRIM_TYPE_CIRCLE;
				namedPos_primitive->radius = 16.f;
				startAddNamedPos("HERO");
			} else if(objectsMenuStatus == 3) {
				namedPosType = GO_TYPE_WALLLASER;
				namedPosString = "OBJ_WALLLASER";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				namedPos_wallmount = true;
				startAddNamedPos("WALLLASER");
				openInputBox("Enter laser type\n0 - laser door (stops objects, doesn't destroy them)\n1 - laser weapon (destroys objects)", "", false);
			} else if(objectsMenuStatus == 4) {
				namedPosType = GO_TYPE_ROBOTGUARD;
				namedPosString = "OBJ_ROBOTGUARD";
				namedPos_primitive->type = PRIM_TYPE_CIRCLE;
				namedPos_primitive->radius = 32.f;
				startAddNamedPos("ROBOTGUARD");
				openInputBox("Enter robot acceleration in pixels per frame (float)\n0 means the robot is static\n50 is pretty slow, 1000 is very fast", "", false);
			} else if(objectsMenuStatus == 5) {
				namedPosType = GO_TYPE_SWITCH;
				namedPosString = "OBJ_SWITCH";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				startAddNamedPos("SWITCH");	
				openInputBox("Please,\nset how many times this switch can be used\n-1 - infinite", "", false);
				gfx->setCursor(true);
			} else if(objectsMenuStatus == 6) {
				namedPosType = GO_TYPE_PRESSUREPAD;
				namedPosString = "OBJ_PRESSUREPAD";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				startAddNamedPos("PRESSUREPAD");
				settingDestination = true;	
				gfx->setCursor(true);
				infobox_used = true;
				showInfoBox("you want to bind the pressure pad to\nleft click on the object\nPlease,", INFOBOX_DEFAULT_TIMEOUT+2.f);
			} else if(objectsMenuStatus == 7) {
				namedPosType = GO_TYPE_MINE;
				namedPosString = "OBJ_MINE";
				namedPos_primitive->type = PRIM_TYPE_CIRCLE;
				namedPos_primitive->radius = 8.f;
				startAddNamedPos("MINE");	
			} else if(objectsMenuStatus == 8) {
				namedPosType = GO_TYPE_EXIT;
				namedPosString = "OBJ_EXIT";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				startAddNamedPos("EXIT");	
			} else if(objectsMenuStatus == 9) {
				namedPosType = GO_TYPE_TEXT;
				namedPosString = "OBJ_TEXT";
				namedPos_primitive->type = PRIM_TYPE_QUAD;
				startAddNamedPos("TEXT");
				openInputBox("Please,\nenter text size (character width in pixels)", "", false);
				gfx->setCursor(true);
			} else if(objectsMenuStatus == 10) {
				namedPosType = GO_TYPE_GRAVITYLIFT;
				namedPosString = "OBJ_GRAVITYLIFT";
				startAddNamedPos("GRAVITYLIFT");
				openInputBox("Enter gravity lift force in pixels per frame per frame (float)\n10 is low, 500 is very high", "", false);
			} else if(objectsMenuStatus == 11) {
				if(gfx->getLevel()->getTheRock()) {
					showInfoBox("Can't add more than one rock.\nSorry!", INFOBOX_DEFAULT_TIMEOUT);
					infobox_used = true;
				} else {
					namedPosType = GO_TYPE_ROCK;
					namedPosString = "OBJ_ROCK";
					startAddNamedPos("ROCK");
				}
			} else if(objectsMenuStatus == 12) {
				namedPosType = GO_TYPE_CHECKPOINT;
				namedPosString = "OBJ_CHECKPOINT";
				startAddNamedPos("CHECKPOINT");
				char* tmp = new char[32];
				sprintf(tmp, "%i", rand() % 100000 + 1);
				addNamedPosVar("ID", tmp);
				delete [] tmp;
			}
			if(!infobox_used)
				showInfoBox("right click - cancel\nleft click - place object\nset brush: add object", INFOBOX_DEFAULT_TIMEOUT);
		}
		if(objectsMenu) objectsMenu->resetStatus();
	}		
}

void LevelEditor::pollInputBox(float frameDelta) {
	int inputStatus;
	if(!inputBox) 
		inputStatus = 0;
	else
		inputStatus = inputBox->pollStatus();

	if(inputStatus != 0) {
		if(inputStatus > 0) {
			char* str = inputBox->getReturnString();
			int len = strlen(str);
			if(len != 0) {
				if(state == STATE_SAVING_LEVEL)
					pollInputBoxSaveLevel(str);
				if(state == STATE_ADDING_NAMED_POS) {
					pollInputBoxAddingNamedPos(str);
				}
				saveInputBox = false;
			} else {
				state = STATE_NONE;
				gfx->setCursor(true);
			}
		} else if(inputStatus == -1) {
			gfx->deleteSceneNode(inputBox);
			inputBox = NULL;
			
			if(namedPosString)
			if(namedPosType == GO_TYPE_ROBOTGUARD && !strstr(namedPosString, "PROJECTILESPEED") && strstr(namedPosString, "FIRINGSPEED")) {
				openInputBox("Enter projectile speed in pixels per frame (float)\n1 is slow\n20 is very fast\nvalues greater than 20 may present bugs on slow systems", "", false);
			}
			
			if(namedPosString)
			if(namedPosType == GO_TYPE_WALLLASER && laserdoorswitch && settingDestination) {
				openInputBox("Enter laser switch on interval in seconds (float)\n0 means always switched off\n-1 means always switched on", "", false);	
				settingDestination = false;
			} else if(namedPosType == GO_TYPE_WALLLASER && !laserdoorswitch && settingDestination) {
				openInputBox("Enter laser door initial state (float)\n-1 - on\n0 - off", "", false);
				settingDestination = false;
			}
			
			if(namedPosString)
			if(namedPosType == GO_TYPE_TEXT && settingDestination) {
				openInputBox("Please,\nenter text to be displayed", "", true);
				settingDestination = false;
			}
		}
		if(inputBox)
			inputBox->resetStatus();
	}
}

void LevelEditor::pollInputBoxSaveLevel(const char* input_string) {
	int len = strlen(input_string);
	if(filename) delete [] filename;
	filename = new char[len+1];
	for(int x=0; x<len; x++) filename[x] = input_string[x];
	filename[len] = 0;
	gfx->getLevel()->writeToFile(gfx->getFilesystem()->getLevelFilename(filename).c_str());
	char* buffer = new char[MAX_TEXT_BUFFER];
	sprintf(buffer, "Saved \"%s\"..", filename);
	showInfoBox(buffer, INFOBOX_DEFAULT_TIMEOUT);
	delete [] buffer;
}

void LevelEditor::pollInputBoxAddingNamedPos(const char* input_string) {
	int len = strlen(input_string);
	if(namedPosType == GO_TYPE_WALLLASER) {
		if(!strstr(namedPosString, "TYPE")) {
			if(atof(input_string) >= 1) {
				addNamedPosVar("TYPE", "1");
				laserdoorswitch = true;
				settingDestination = true;
				namedPos_sprite_texid = gfx->getTexMgr()->getId("WALLLASER");
			} else {
				addNamedPosVar("TYPE", "0");
				laserdoorswitch = false;
				settingDestination = true;
				namedPos_sprite_texid = gfx->getTexMgr()->getId("WALLDOOR");	
			}
		} else {
			if(atof(input_string) >= 0)
				if(laserdoorswitch)
					addNamedPosVar("INTERVAL", input_string);
				else addNamedPosVar("INTERVAL", "0");
			else addNamedPosVar("INTERVAL", "-1");
		}
	} else if(namedPosType == GO_TYPE_ROBOTGUARD) {
		if(!strstr(namedPosString, "MOVESPEED")) {
			if(atof(input_string) > 0) addNamedPosVar("MOVESPEED", input_string);
			else addNamedPosVar("MOVESPEED", "0");	
			showInfoBox("left click on the movement destination!\nPlease,", INFOBOX_DEFAULT_TIMEOUT+2.f);
			gfx->setCursor(true);	
			settingDestination = true;	
		} else if(!strstr(namedPosString, "FIRINGSPEED")) {
			if(atof(input_string) > 0) addNamedPosVar("FIRINGSPEED", input_string);	
			else addNamedPosVar("FIRINGSPEED", "0");	
		} else if(!strstr(namedPosString, "PROJECTILESPEED")) {
			if(atof(input_string) > 0) addNamedPosVar("PROJECTILESPEED", input_string);
			else addNamedPosVar("PROJECTILESPEED", "1");		
		}
	} else if(namedPosType == GO_TYPE_SWITCH) {
		if(!strstr(namedPosString, "SWITCHTIMES")) {
			if(atof(input_string) >= 0) addNamedPosVar("SWITCHTIMES", input_string);
			else addNamedPosVar("SWITCHTIMES", "-1");
			showInfoBox("you want to bind the switch to\nleft click on the object\nPlease,", INFOBOX_DEFAULT_TIMEOUT+2.f);
			settingDestination = true;	
		}	
	} else if(namedPosType == GO_TYPE_TEXT) {
		if(!strstr(namedPosString, "SIZE")) {
			addNamedPosVar("SIZE", input_string);
			settingDestination = true;	
		} else {
			addNamedPosVar("TEXT", input_string);	
		}
	} else if(namedPosType == GO_TYPE_GRAVITYLIFT) {
		if(!strstr(namedPosString, "FORCE")) {
			addNamedPosVar("FORCE", input_string);	
		}	
	}
}

void LevelEditor::doDrawing(float frameDelta) {
	if(drawing) {
		vector2 mousepos = gfx->getMousePos();
		if(currentBrush == BRUSH_FILL_TILE) {
			vector2 tilepos = gfx->getLevel()->getTileByXY(mousepos);
			tilepos[0] *= PPT_X;
			tilepos[1] *= PPT_Y;
			tilepos[0] += PPT_X/2;
			tilepos[1] += PPT_Y/2;
			if(!checkTileCollision(tilepos))
				gfx->getLevel()->fillTile(tilepos);
			else showInfoBox("(something is in the way)\nCan't draw here!", INFOBOX_DEFAULT_TIMEOUT);
		} else if(currentBrush == BRUSH_CLEAR_TILE)
			gfx->getLevel()->clearTile(mousepos);	
	}	
}

bool LevelEditor::checkNamedPosCollision(vector2 displacement, bool doLevelTest, bool doObjectsTest) {
	Primitive* tmp;		
	tmp = new Primitive;
	if(namedPos_primitive) {
		tmp->type = namedPos_primitive->type;
		if(tmp->type == PRIM_TYPE_CIRCLE) { 
			tmp->radius = namedPos_primitive->radius;
		}
		delete namedPos_primitive;
	} else {
		tmp->type = PRIM_TYPE_QUAD;	
	}
	namedPos_primitive = tmp;
	vector2 size = gfx->getTexMgr()->getSize(namedPos_sprite_texid);
	switch(namedPos_primitive->type) {
	case PRIM_TYPE_QUAD:
		namedPos_primitive->vertices[0].set(-size[0]/2.f, -size[1]/2.f+1.f);
		namedPos_primitive->vertices[1].set(size[0]/2.f, -size[1]/2.f+1.f);
		namedPos_primitive->vertices[2].set(size[0]/2.f, size[1]/2.f);
		namedPos_primitive->vertices[3].set(-size[0]/2.f, size[1]/2.f);
		for(int x=0; x<4; x++) namedPos_primitive->vertices[x] += displacement;
		gfx->getPhysMgr()->setQuadEdgesAndConstraints(namedPos_primitive);
		gfx->getPhysMgr()->calculatePolyCenter(namedPos_primitive);
		break;
	case PRIM_TYPE_CIRCLE:
		namedPos_primitive->center = displacement;
		namedPos_primitive->vertices[0] = displacement;
		break;
	}
	
	CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(namedPos_primitive, doLevelTest, doObjectsTest);
	return event.doIntersect;
}

bool LevelEditor::checkTileCollision(vector2 displacement) {
	Primitive* tile;
	tile = new Primitive;
	tile->type = PRIM_TYPE_QUAD;
	vector2 size(PPT_X, PPT_Y);
	tile->vertices[0].set(-size[0]/2, -size[1]/2);
	tile->vertices[1].set(size[0]/2, -size[1]/2);
	tile->vertices[2].set(size[0]/2, size[1]/2);
	tile->vertices[3].set(-size[0]/2, size[1]/2);
	for(int x=0; x<4; x++) tile->vertices[x] += displacement;
	gfx->getPhysMgr()->setQuadEdgesAndConstraints(tile);
	CollisionEvent event = gfx->getPhysMgr()->primitiveCollisionTest(tile, false);
	return event.doIntersect;
}

void LevelEditor::startAddNamedPos(const char* name) {
	if(namedPosName) delete [] namedPosName;
	namedPosName = new char[MAX_TEXT_BUFFER];
	int namelen = strlen(name);
	for(int x=0; x<namelen; x++) namedPosName[x] = name[x];
	namedPosName[namelen] = 0;
	
	state = STATE_ADDING_NAMED_POS;
	if(namedPosString) delete [] namedPosString;
	namedPosString = new char[MAX_TEXT_BUFFER];
	namedPosString[0] = 0;
	sprintf(namedPosString, "OBJ=%s", name);
	namedPosString[4+strlen(name)] = 0;
	if(!namedPos_wallmount)
		gfx->setCursor(false);
	namedPos_sprite_texid = gfx->getTexMgr()->getId(name);
	nameVarLen = 0;
}

void LevelEditor::addNamedPosVar(const char* var, const char* value) {
	if(state == STATE_ADDING_NAMED_POS) {
		if(!namedPosString) return;
		if(!var || !value) return;
		sprintf(namedPosString, "%s_%s=%s", namedPosString, var, value);
	}
}

void LevelEditor::drawCursor(float frameDelta) {
	if(state == STATE_NONE) {
		vector2 mousepos = gfx->getMousePos(false);
		int index = gfx->getLevel()->findNamedPosition(mousepos);
		
		if(index >= 0) {
			char* objType = gfx->getLevel()->getNamedPosValue(index, "OBJ");
			if(!strcmp("SWITCH", objType) || !strcmp("PRESSUREPAD", objType)) {
				char* targetName = gfx->getLevel()->getNamedPosValue(index, "TARGET");
				if(targetName) {
					SceneNode* obj = gfx->getLevel()->getObjectByName(targetName);
					if(obj) {
						Primitive* x = obj->getCollisionPrimitive();
						vector2 switchpos = gfx->getLevel()->getNamedPosPosition(index);
						color clr(0.5f, 0.5f, 1.f, 0.7f);
						if(x)
							gfx->getRenderer()->drawLine(x->center, switchpos, clr, clr, false);
					}
					delete [] targetName;
				}
			}
		}
		
		if(index >= 0 && !infobox_nps) {
			showInfoBox(gfx->getLevel()->getNamedPosString(index), INFOBOX_NO_TIMEOUT, 16.f, gfx->getMousePos(true), 2.f, 0.5f, "_");
			nps_lastindex = index;
			infobox_nps = true;	
			
		} else if(index != nps_lastindex && infobox_nps && index >= 0) {
			showInfoBox(gfx->getLevel()->getNamedPosString(index), INFOBOX_NO_TIMEOUT, 16.f, gfx->getMousePos(true), 2.f, 0.5f, "_");
			nps_lastindex = index;
			infobox_nps = true;
		} else if(index < 0 && infobox_nps) {
			hideInfoBox();
			infobox_nps = false;
		}
	} else if(state == STATE_ADDING_NAMED_POS) {
		if(settingDestination) {
			// pass
		} else if(!namedPos_wallmount && namedPosType != GO_TYPE_TEXT) {
			DrawPrimitive* quad = new DrawPrimitive;
			for(int x=0; x<4; x++) quad->vertices_colors[x] = color(1.f, 1.f, 1.f, 0.75f);
			vector2 size = gfx->getTexMgr()->getSize(namedPos_sprite_texid);
			quad->size = size;
			quad->tex_size = gfx->getTexMgr()->getTexSize(namedPos_sprite_texid);
			
			quad->vertices[0].set(0.f, 0.f);
			quad->vertices[1].set(size[0], 0.f);
			quad->vertices[2].set(size[0], size[1]);
			quad->vertices[3].set(0.f, size[1]);
			for(int x=0; x<4; x++) quad->vertices[x] += gfx->getMousePos(false) - size*0.5f;

			if(namedPosType != GO_TYPE_TEXT) {
				unsigned int* textures = gfx->getTexMgr()->getTexture(namedPos_sprite_texid);
				gfx->getRenderer()->drawSprite(quad, textures[0]);
			}	
			delete quad;
			
		} else if (gfx->getLevel()->getTile(gfx->getMousePos(false)) != 0 && namedPosType != GO_TYPE_TEXT){
			gfx->setCursor(true);
			DrawPrimitive* quad = new DrawPrimitive;
			for(int x=0; x<4; x++) quad->vertices_colors[x] = color(1.f, 1.f, 1.f, 0.75f);
			vector2 size = gfx->getTexMgr()->getSize(namedPos_sprite_texid);
			quad->size = size;
			quad->tex_size = gfx->getTexMgr()->getTexSize(namedPos_sprite_texid);
						
			vector2 mousecoords = gfx->getMousePos(false);
			vector2 tilecoords = gfx->getLevel()->getTileByXY(mousecoords);
			int tile = gfx->getLevel()->getTile(gfx->getMousePos(false));
			
			vector2 tile_center;
			tile_center.set(tilecoords[0]*PPT_X+PPT_X/2, tilecoords[1]*PPT_Y+PPT_Y/2);
			
			vector2 dfc;
			dfc = tile_center - mousecoords; // distance from center of tile
			
			if(tile == 15) {
				// implicit tile grid
				if(dfc[0] > -6 && dfc[0] < 6 && dfc[1] > - 12 && dfc[1] < 0) {
					quad->vertices[0].set(0.f, 0.f);
					quad->vertices[1].set(size[0], 0.f);
					quad->vertices[2].set(size[0], size[1]);
					quad->vertices[3].set(0.f, size[1]);
					for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(size[0]/2, 0.f);
				} else if(dfc[0] > -6 && dfc[0] < 6 && dfc[1] < 12 && dfc[1] > 0) { 
					quad->vertices[3].set(0.f, 0.f);
					quad->vertices[2].set(size[0], 0.f);
					quad->vertices[1].set(size[0], size[1]);
					quad->vertices[0].set(0.f, size[1]);
					for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(size[0]/2, PPT_Y/2);
				} else if(dfc[0] < 0 && dfc[0] > -12 && dfc[1] > - 6 && dfc[1] < 6) {
					quad->vertices[0].set(0.f, 0.f);
					quad->vertices[3].set(size[0], 0.f);
					quad->vertices[2].set(size[0], size[1]);
					quad->vertices[1].set(0.f, size[1]);
					for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(0.f, size[1]/2);	
				} else if(dfc[0] > 0 && dfc[0] < 12 && dfc[1] > -6 && dfc[1] < 6) {
					quad->vertices[3].set(0.f, 0.f);
					quad->vertices[0].set(size[0], 0.f);
					quad->vertices[1].set(size[0], size[1]);
					quad->vertices[2].set(0.f, size[1]);
					for(int x=0; x<4; x++) quad->vertices[x] += tile_center - vector2(PPT_X/2, size[1]/2);	
				}
				unsigned int* textures = gfx->getTexMgr()->getTexture(namedPos_sprite_texid);
				gfx->getRenderer()->drawSprite(quad, textures[0]);
			}	
			delete quad;
		} else {
			gfx->setCursor(true);
		}
	} else if(state == STATE_INVERTING_ZONE && settingDestination) {
		vector2 mousepos = gfx->getMousePos(false);
		DrawPrimitive* quad = new DrawPrimitive;
		quad->type = PRIM_TYPE_QUAD;
		
		for(int x=0; x<4; x++)
			quad->vertices_colors[x] = color(1.f, 1.f, 1.f, 0.3f);
			
		int start_x, end_x, start_y, end_y, tmp;
		gfx->getLevel()->getTileByXY(invertZoneStart[0], invertZoneStart[1], &start_x, &start_y);
		gfx->getLevel()->getTileByXY(mousepos[0], mousepos[1], &end_x, &end_y);
		
		if(start_x > end_x) { 
			tmp = start_x;
			start_x = end_x;
			end_x = tmp;
		}
		if(start_y > end_y) {
			tmp = start_y;
			start_y = end_y;
			end_y = tmp;	
		}
		
		quad->vertices[0].set(start_x*PPT_X, start_y*PPT_Y);
		quad->vertices[1].set(end_x*PPT_X, start_y*PPT_Y);
		quad->vertices[2].set(end_x*PPT_X, end_y*PPT_Y);
		quad->vertices[3].set(start_x*PPT_X, end_y*PPT_Y);
		
		gfx->getRenderer()->drawQuad(quad);
		
		delete quad;
	}	
}

void LevelEditor::init() {
	gfx->getLevel()->clear();
	gfx->getLevel()->computeSlanted();
	gfx->getLevel()->populateObjects();
	gfx->getInputHandler()->addReceiver(this);
	gfx->getCamera()->setKeyboardControl(true);
	showInfoBox(LEVELEDITOR_WELCOME_MSG);
}

void LevelEditor::update(float frameDelta) {
	if(_isActive) {
		pollEditMenu(frameDelta);
		pollLoadMenu(frameDelta);
		pollObjectsMenu(frameDelta);
		pollInputBox(frameDelta);
		
		doDrawing(frameDelta);	
		drawCursor(frameDelta);

		SceneNode::update(frameDelta);
	}
}

void LevelEditor::receiveKeyEvent(list<KeyEvent> events) {
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		if(state == STATE_PREVIEW_LEVEL && (*x).state == GLFW_PRESS) {
			switch((*x).key) {
			case KEY_ESC:
			case KEY_ENTER:
				(*x).chr = 80;
			case KEY_CHR:
				if((*x).chr == 80) {
					state = STATE_NONE;
					gfx->getLevel()->populateObjects(false);
					gfx->getLevel()->setShowNamedPositions(true);
					gfx->getCamera()->setKeyboardControl(true);
					gfx->getPhysMgr()->stopSimulation();
					gfx->setCursor(true);
					hideLabel();
				}
				break;
			}
			return;
		}
		switch((*x).key) {
		case KEY_LEFT_MB:
			if((*x).state == GLFW_PRESS) {
				if(state == STATE_MOVING_NAMED_POS) {
					receiveKeyEvent_moveNamedPos(*x);
				} else if(state == STATE_ADDING_NAMED_POS) {
					receiveKeyEvent_addNamedPos(*x);
				} else if(state == STATE_DELETING_NAMED_POS) {		
					vector2 coords = gfx->getMousePos(false);
					gfx->getLevel()->delNamedPosition(
						gfx->getLevel()->findNamedPosition(coords));
				} else if(state == STATE_INVERTING_ZONE) {
					if(!settingDestination) {
						invertZoneStart = gfx->getMousePos(false);
						settingDestination = true;	
					}
				} else {
					drawing = true;
				}
			} else {
				if(state == STATE_INVERTING_ZONE && settingDestination) {
					vector2 invertZoneEnd = gfx->getMousePos(false);
					int start_x, end_x, start_y, end_y, tmp;
					gfx->getLevel()->getTileByXY(invertZoneStart[0], invertZoneStart[1], &start_x, &start_y);
					gfx->getLevel()->getTileByXY(invertZoneEnd[0], invertZoneEnd[1], &end_x, &end_y);
					if(start_x > end_x) {
						tmp = start_x;
						start_x = end_x;
						end_x = tmp;	
					}
					if(start_y > end_y) {
						tmp = start_y;
						start_y = end_y;
						end_y = tmp;	
					}
					gfx->getLevel()->invertTileRegion(start_x, end_x, start_y, end_y);
					settingDestination = false;
					hideInfoBox();	
				}
				drawing = false;
			}
			break;
		case KEY_RIGHT_MB:
			if((*x).state == GLFW_PRESS) {
				if(state == STATE_ADDING_NAMED_POS || state == STATE_DELETING_NAMED_POS || state == STATE_MOVING_NAMED_POS || state == STATE_INVERTING_ZONE	) {
					state = STATE_NONE;
					settingDestination = false;
					gfx->setCursor(true);
				} else {
					openEditMenu(gfx->getMousePos(true));
				}
			}	
			break;
		case KEY_CHR:
			if((*x).state == GLFW_PRESS && state != STATE_ADDING_NAMED_POS) {
				switch((*x).chr) {
				case 68: // d
					state = STATE_NONE;
					currentBrush = BRUSH_FILL_TILE;	
					showInfoBox("right click - cancel\nleft click - draw\nset brush: draw tiles", INFOBOX_DEFAULT_TIMEOUT);
					gfx->setCursor(true);
					break;
				case 67: // c
					state = STATE_NONE;
					currentBrush = BRUSH_CLEAR_TILE;
					showInfoBox("right click - cancel\nleft click - clear\nset brush: clear tiles", INFOBOX_DEFAULT_TIMEOUT);
					gfx->setCursor(true);
					break;
				case 65: // a
					state = STATE_NONE;
					currentBrush = BRUSH_NONE;
					openObjectsMenu(gfx->getMousePos(true));
					gfx->setCursor(true);
					break;
				case 80: // p
					hideInfoBox();
					if(!gfx->getLevel()->getPlayableCharacter()) {
						showInfoBox("Use \"add object\" to add new objects.\nCan't preview the level without a hero in it.\nSorry!", INFOBOX_DEFAULT_TIMEOUT+3.f);	
						break;
					}
					state = STATE_PREVIEW_LEVEL;
					gfx->getLevel()->populateObjects(true);
					gfx->getLevel()->computeSlanted();
					gfx->getLevel()->setShowNamedPositions(false);
					gfx->getCamera()->setKeyboardControl(false);
					gfx->getPhysMgr()->startSimulation();
					showLabel("Previewing level - press <P> to return");
					gfx->setCursor(false);
					break;
				case 81: // q
					// save level
					if(!gfx->getLevel()->getPlayableCharacter()) {
						showInfoBox("Use \"add object\" to add new objects.\nCan't save the level without a hero in it.\nSorry!", INFOBOX_DEFAULT_TIMEOUT+3.f);	
						break;
						}
					state = STATE_SAVING_LEVEL;
					if(!filename) {
						openInputBox("Enter level name:", filename);
						saveInputBox = true;
					} else {
						gfx->getLevel()->writeToFile(gfx->getFilesystem()->getLevelFilename(filename).c_str());
						char* buffer = new char[MAX_TEXT_BUFFER];
						sprintf(buffer, "Saved \"%s\"..", filename);
						showInfoBox(buffer, INFOBOX_DEFAULT_TIMEOUT);
						delete [] buffer;
					}
					break;
				case 83: // s
					state = STATE_DELETING_NAMED_POS;
					currentBrush = BRUSH_NONE;
					showInfoBox("right click - cancel\nleft click - delete object\nset brush: delete object", INFOBOX_DEFAULT_TIMEOUT);
					break;
				case 90:
					state = STATE_INVERTING_ZONE;
					settingDestination = false;
					showInfoBox("Please, drag along the zone you wish to invert", INFOBOX_DEFAULT_TIMEOUT);
					break;
				}	
			}
		}
	}
}

void LevelEditor::receiveKeyEvent_moveNamedPos(KeyEvent event) {
	
/*	vector2 coords = gfx->getMousePos(false);
	int index = gfx->getLevel()->findNamedPosition(coords);
	if(index >= 0){
		namedPosString = new char[NAMED_POSITION_LEN];
		char* str = gfx->getLevel()->getNamedPosString(index);
		for(int pp=0; pp<strlen(str); pp++) {
			namedPosString[pp] = str[pp];
		}
		namedPosString[strlen(str)] = 0;
		char* type = gfx->getLevel()->getNamedPosValue(index, "OBJ");
		namedPos_sprite_texid = gfx->getTexMgr()->getId(type);
		state = STATE_ADDING_NAMED_POS;
		settingDestination = false;
		gfx->getLevel()->delNamedPosition(index);
		
		
	}*/
}

void LevelEditor::receiveKeyEvent_addNamedPos(KeyEvent event) {
	vector2 coords = gfx->getMousePos(false);
	if(gfx->getLevel()->named_positions_n + 1 > MAX_PHYS_OBJECTS)
			showInfoBox("You've reached the objects limit.\nCan't add object!", INFOBOX_DEFAULT_TIMEOUT);
	else {
		if(namedPosType == GO_TYPE_ROBOTGUARD && settingDestination) {
			char* dest_x = new char[25];
			char* dest_y = new char[25];
			sprintf(dest_x, "%.1f", coords[0]);
			sprintf(dest_y, "%.1f", coords[1]);
			addNamedPosVar("DESTX", dest_x);
			addNamedPosVar("DESTY", dest_y);
			delete [] dest_x;
			delete [] dest_y;	
			settingDestination = false;
			openInputBox("Enter robot firing speed in shots per second (float)\n0 means the robot can't fire", "", false);
			gfx->setCursor(false);
		} else if((namedPosType == GO_TYPE_SWITCH || namedPosType == GO_TYPE_PRESSUREPAD) && settingDestination) {
			int index = gfx->getLevel()->findNamedPosition(coords);
			if(index < 0) return;
			
			char* targetType = gfx->getLevel()->getNamedPosValue(index, "OBJ");
			if(strcmp(targetType, "WALLLASER") != 0 && strcmp(targetType, "WALLDOOR") != 0 && strcmp(targetType, "ROBOTGUARD") != 0) {
				showInfoBox("lasers and robot guards\nyou can only bind switches and pressure pads to\nSorry,", INFOBOX_DEFAULT_TIMEOUT);
				return;	
			}
				
			char* targetName = gfx->getLevel()->getNamedPosValue(index, "NAME");
			addNamedPosVar("TARGET", targetName);
			SceneNode* obj = gfx->getLevel()->getObjectByName(targetName);
			if(!obj) {
				state = STATE_NONE;
				settingDestination = false;
				gfx->setCursor(true);
				return;
			}
			
			if(obj->getSwitch())
				addNamedPosVar("STATE", "1");
			else addNamedPosVar("STATE", "0");
			delete [] targetName;
			settingDestination = false;
			gfx->setCursor(false);
		} else if(namedPos_wallmount && gfx->getLevel()->getTile(coords) == 15) {
			vector2 tile_coords = gfx->getLevel()->getTileByXY(coords);
			tile_coords[0] *= PPT_X;
			tile_coords[1] *= PPT_Y;
			aabb2 bounding_box;
			bounding_box.bottom_left = tile_coords;
			bounding_box.top_right = tile_coords + vector2(PPT_X, PPT_Y);
			
			if(gfx->getLevel()->checkNamedPosArea(bounding_box) == -1) {
				if(nameVarLen) {
					namedPosString[strlen(namedPosString)-nameVarLen-6] = 0;	
				}

				char* tmp = new char[MAX_TEXT_BUFFER];
				tmp[0] = 0;
				sprintf(tmp, "%s%i.%i", namedPosName, gfx->getLevel()->getNamedPositionsN(), rand() % 10000);
				nameVarLen = strlen(tmp);
				addNamedPosVar("NAME", tmp);
				delete [] tmp;
				gfx->getLevel()->addNamedPosition(coords, namedPosString);
			}
			else showInfoBox("(something is in the way)\nCan't place here!", INFOBOX_DEFAULT_TIMEOUT);
		} else if(namedPos_wallmount) {
			showInfoBox("(this object belongs on a wall)\nCan't place here!", INFOBOX_DEFAULT_TIMEOUT);
		} else if(!checkNamedPosCollision(coords)) {
			if(nameVarLen) {
				namedPosString[strlen(namedPosString)-nameVarLen-6] = 0;	
			}

			char* tmp = new char[MAX_TEXT_BUFFER];
			tmp[0] = 0;
			sprintf(tmp, "%s%i.%i", namedPosName, gfx->getLevel()->getNamedPositionsN(), rand() % 100);
			nameVarLen = strlen(tmp);
			addNamedPosVar("NAME", tmp);
			delete [] tmp;
			
			if(namedPosType == GO_TYPE_CHECKPOINT || namedPosType == GO_TYPE_CHAR || namedPosType == GO_TYPE_ROCK) {
				state = STATE_NONE;
				gfx->setCursor(true);	
			}
			
			gfx->getLevel()->addNamedPosition(coords, namedPosString);
		} else if(namedPosType == GO_TYPE_TEXT) {
			if(nameVarLen) {
				namedPosString[strlen(namedPosString)-nameVarLen-6] = 0;	
			}

			char* tmp = new char[MAX_TEXT_BUFFER];
			tmp[0] = 0;
			sprintf(tmp, "%s%i.%i", namedPosName, gfx->getLevel()->getNamedPositionsN(), rand() % 100);
			nameVarLen = strlen(tmp);
			addNamedPosVar("NAME", tmp);
			delete [] tmp;
			if(namedPosType == GO_TYPE_CHAR) {
				state = STATE_NONE;
				gfx->setCursor(true);	
			}
			gfx->getLevel()->addNamedPosition(coords, namedPosString);
		} else showInfoBox("(something is in the way)\nCan't place here!", INFOBOX_DEFAULT_TIMEOUT);
	}
}

LevelSceneNode* LevelEditor::getLevel() {
	return gfx->getLevel();	
}
