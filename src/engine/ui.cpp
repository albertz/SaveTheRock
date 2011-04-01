/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

// UIWidget class methods

UIWidget::UIWidget() {}

void UIWidget::update(float frameDelta) {}

void UIWidget::setColor(color clr_n) {
	clr = clr_n;
	if(!defaultColorSet) {
		defaultColor = clr;
		defaultColorSet = true;
	}
}

void UIWidget::setColor() {
	clr = defaultColor;	
}

void UIWidget::fadeIn() {
	clr[3] = 0.f;
	doFadeIn = true;
	setActive(true);
}

void UIWidget::fadeOut() {
	clr[3] = 1.f;
	doFadeOut = true;
//	setActive(false);
}

// UILabel class methods

UILabel::UILabel(GfxMgr* gfx_n){
	gfx = gfx_n;
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_LABEL;
	
	_isActive = false;
	level_label = false;

	doFadeIn = false;
	doFadeOut = false;
	defaultColorSet = false;
	_markForDeletion = 0;
}

void UILabel::update(float frameDelta) {
	if(level_label) {
		vector2 camerapos = gfx->getCamera()->getAbsoluteTranslation();
		vector2 wparams = gfx->getRenderer()->getWindowParams();
		aabb2 screen;
		screen.bottom_left = camerapos;
		screen.top_right = camerapos + wparams;
		
		vector2 center = position + vector2((text_size-text_size/2.f)*(text.size()+1.f)/2.f, text_size);
		bool isInsideScreen = screen.checkPointInside(center);
		if(isInsideScreen && clr[3] == 0.f) {
			fadeIn();
		} else if(!isInsideScreen && clr[3] == 1.f) {
			fadeOut();	
		}
	}
	
	if(_isActive) {
		if(doFadeIn && doFadeOut) {
			// can't fade in and out at the same time
			doFadeIn = false;
			doFadeOut = true;
		}
		if(doFadeIn) {
			clr[3] += frameDelta * FADEIN_MULTIPLIER;
			if(clr[3] >= 1.f) {
				clr[3] = 1.f;
				doFadeIn = false;
			}	
		}
		if(doFadeOut) {
			clr[3] -= frameDelta * FADEOUT_MULTIPLIER;
			if(clr[3] <= 0.f) {
				clr[3] = 0.f;
				doFadeOut = false;
				setActive(false);
			}
		}
		
		if(!level_label) 
			gfx->getRenderer()->drawText(text, text.size(), text_font, text_size, clr, position);
		else
			gfx->getRenderer()->drawText(text, text.size(), text_font, text_size, clr, position-gfx->getCamera()->getAbsoluteTranslation());
	}
}

void UILabel::setPosition(vector2 position_n) {
	position = position_n;
}

void UILabel::setText(const std::string& text_n, bool font_n, float size_n, bool level_label_n) {
	text = text_n;
	text_font = font_n;
	text_size = size_n;
	level_label = level_label_n;
}

aabb2 UILabel::getBoundingBox() {
	// works correctly only for one line labels (such as these used in menus),
	// this is because line wrap is done in the renderer, which allows for better rendering control,
	// but makes bounds calculation not trivial
	// we don't really need to detect events on large bodies of text, rather just menu labels, so it's ok (;
	bounding_box.bottom_left.set(position[0], position[1]);
	bounding_box.top_right.set(position[0]+(text_size-text_size/2)*(text.size()+1), position[1]+text_size);
	return bounding_box;
}

// UIImage class methods

UIImage::UIImage(GfxMgr* gfx_n) {
	gfx = gfx_n;
	sprite = NULL;
	_isActive = false;
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_IMAGE;
	doFadeIn = false;
	doFadeOut = false;
	_markForDeletion = false;
	transparency = 0.f;
}
void UIImage::update(float frameDelta) {
	if(_isActive) {
		SceneNode::update(frameDelta);
		DrawPrimitive* image = sprite->getPrimitive();
		
		if(doFadeIn) {
			if(transparency < 1.f) transparency += frameDelta * FADEIN_MULTIPLIER;
			else {
				doFadeIn = false;
				transparency = 1.f;
			}
		}
		if(doFadeOut) {
			if(transparency > 0.f) transparency -= frameDelta * FADEOUT_MULTIPLIER;
			else {
				doFadeOut = false;
				transparency = 0.f;
			}
		}
		
		image->vertices_colors[0][3] = transparency;
	}
}

void UIImage::setPosition(vector2 position_n) {
	position = position_n;	
	DrawPrimitive* image = sprite->getPrimitive();
	image->vertices[0] = position;
	image->vertices[1] = position + vector2(size[0], 0.f);
	image->vertices[2] = position + vector2(size[0], size[1]);
	image->vertices[3] = position + vector2(0.f, size[1]);
}

void UIImage::setImage(const std::string& texture_name) {
	sprite = new AnimatedSpriteSceneNode(gfx);
	int texid = gfx->getTexMgr()->getId(texture_name);
	size = gfx->getTexMgr()->getSize(texid);
	sprite->setSprite(texid);
	sprite->setActive(true);
	gfx->addSceneNode(sprite, this);
	_isActive = true;
}

vector2 UIImage::getSize() {
	return size;	
}


// UIInfoBox class methods

UIInfoBox::UIInfoBox(GfxMgr* gfx_n) {
	gfx = gfx_n;	
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_INFOBOX;
	
	labels_n = 0;
	quad = new DrawPrimitive;
	quad->type = PRIM_TYPE_QUAD;
	
	_isActive = false;
	doFadeIn = false;
	doFadeOut = false;
	doAllowClose = false;
	timeOut = 0.f;
	_markForDeletion = 0;
}

UIInfoBox::~UIInfoBox() {
	fadeOut();	
}

void UIInfoBox::setupPrimitive() {
	quad->vertices[0].set(position[0] - margin, position[1] - margin);
	quad->vertices[1].set(position[0] + size[0] + margin, position[1] - margin);
	quad->vertices[2].set(position[0] + size[0] + margin, position[1] + size[1] + margin);
	quad->vertices[3].set(position[0] - margin, position[1] + size[1] + margin);
	for(int x=0; x<4; x++) {
		quad->vertices_colors[x] = clr;	
	}	
}

void UIInfoBox::setActive(bool state) {
	if(state && doAllowClose && timeOut==0.f) {
		gfx->getInputHandler()->takeControl(this);
	}
	
	SceneNode::setActive(state);	
}

void UIInfoBox::update(float frameDelta) {
	if(_isActive) {		
		if(timeOut > 0) {
			timeOut -= frameDelta;
			if(timeOut <= 0) {
				timeOut = 0;
				fadeOut();	
			}	
		}
		
		if(doFadeIn && doFadeOut) {
			// can't fade in and out at the same time
			doFadeIn = false;
			doFadeOut = true;
		}
		if(doFadeIn) {
			clr[3] += frameDelta * FADEIN_MULTIPLIER;
			if(clr[3] >= 0.8f) {
				clr[3] = 0.8f;
				doFadeIn = false;
			}	
		}
		if(doFadeOut) {
			clr[3] -= frameDelta * FADEOUT_MULTIPLIER;
			if(clr[3] <= 0.f) {
				clr[3] = 0.f;
				doFadeOut = false;
				setActive(false);
			}
		}
		setupPrimitive();
		gfx->getRenderer()->drawQuad(quad);
		SceneNode::update(frameDelta);
	}
}

void UIInfoBox::setPosition(vector2 position_n) {
	position = position_n;
	int i = 0;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		vector2 newPos = position;
		newPos.set(newPos[0] - text_size/2, newPos[1] + text_size*i);
		(*x)->setPosition(newPos);
		i++;
	}	
}

void UIInfoBox::setText(float margin_n, float text_size_n, bool font_n) {
	size = vector2(0.f, 0.f);
	margin = margin_n;
	text_size = text_size_n;
	text_font = font_n;
	if(timeOut == 0.f) {
		addText("Press <enter> to dismiss", true);
		addText(" ");
		addText(" ");
	}
}

void UIInfoBox::setTimeout(float seconds) {
	timeOut = seconds;
}

void UIInfoBox::addText(const std::string& text, bool isGray) {
	UILabel* label = new UILabel(gfx);
	if(isGray)
		label->setColor(MENU_TEXT_COLOR_SELECTED);
	else label->setColor(MENU_TEXT_COLOR);
	label->setText(text, text_font, text_size);
	label->setPosition(vector2());
	label->setActive(false);
//	label->fadeIn();
	
	float minMenuSizeX = text.size()*(text_size-text_size/2);
	if(minMenuSizeX > size[0]) size[0] = minMenuSizeX;
	size[1] = (labels_n+1)*text_size;
	
	gfx->addSceneNode(label, this);
	labels.push_back(label);
	labels_n++;
}

void UIInfoBox::addBreak() {
	UILabel* label = new UILabel(gfx);
	label->setColor(MENU_TEXT_COLOR);
	label->setText("...", 0, text_size);
	label->setPosition(vector2());
	label->setActive(false);
//	label->fadeIn();
	
	//size[0] = 18;
	size[1] = (labels_n+1)*text_size;
	
	gfx->addSceneNode(label, this);
	labels.push_back(label);
	labels_n++;	
}

void UIInfoBox::receiveKeyEvent(list<KeyEvent> events) {
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		switch((*x).key) {
		case KEY_ENTER:
			if((*x).state == GLFW_PRESS) {
				if(doAllowClose) {
					fadeOut();
				}
			}
			break;
		}
	}
}

void UIInfoBox::fadeIn() {
	clr[3] = 0.f;
	doFadeIn = true;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		(*x)->fadeIn();	
	}
	setActive(true);
}

void UIInfoBox::fadeOut() {
	gfx->getInputHandler()->releaseControl(this);
	clr[3] = 0.8f;
	doFadeOut = true;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		(*x)->fadeOut();	
	}
	//setActive(false);
}

void UIInfoBox::allowClose(bool doAllow) {
	if(doAllow) {
		doAllowClose = true;	
	} else {
		doAllowClose = false;	
	}
}

vector2 UIInfoBox::getSize() {
	return size;
}

vector2 UIInfoBox::getPosition() {
	return position;
}

// UIInputBox class methods

UIInputBox::UIInputBox(GfxMgr* gfx_n) {
	gfx = gfx_n;
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_INPUTBOX;
	
	_isActive = false;
	doFadeIn = false;
	doFadeOut = false;
	
	quad = new DrawPrimitive;
	quad2 = new DrawPrimitive;
	quad->type = PRIM_TYPE_QUAD;
	quad2->type = PRIM_TYPE_QUAD;	
	
	returnState = false;
	
	alphanumeric = true;
	clr2 = INPUTBOX_COLOR;
	dotCount = false;
	_markForDeletion = 0;
}

UIInputBox::~UIInputBox() {
	//fadeOut();	
}

void UIInputBox::setupPrimitives() {
	quad->vertices[0].set(position[0] - margin, position[1] - margin);
	quad->vertices[1].set(position[0] + size[0] + margin, position[1] - margin);
	quad->vertices[2].set(position[0] + size[0] + margin, position[1] + size[1] + margin);
	quad->vertices[3].set(position[0] - margin, position[1] + size[1] + margin);
	for(int x=0; x<4; x++) {
		quad->vertices_colors[x] = clr;	
	}
	
	quad2->vertices[0].set(position[0] + margin, position[1] + margin);
	quad2->vertices[1].set(position[0] + size[0] - margin, position[1] + margin);
	quad2->vertices[2].set(position[0] + size[0] - margin, position[1] + 32.f);
	quad2->vertices[3].set(position[0] + margin, position[1] + 32.f);
	for(int x=0; x<4; x++) {
		quad2->vertices_colors[x] = clr2;	
	}		
}

void UIInputBox::setActive(bool state) {
	if(state) {
		gfx->getInputHandler()->takeControl(this);
	}
	SceneNode::setActive(state);	
}

void UIInputBox::update(float frameDelta) {
	if(_isActive) {
	//	if(!doFadeOut)
		//	updateMousePos();
		
		if(doFadeIn && doFadeOut) {
			// can't fade in and out at the same time
			doFadeIn = false;
			doFadeOut = true;
		}
		if(doFadeIn) {
			clr[3] += frameDelta * FADEIN_MULTIPLIER;
			clr2[3] += frameDelta * FADEIN_MULTIPLIER;
			if(clr[3] >= 0.8f) {
				clr[3] = 0.8f;
				clr2[3] = 0.8f;
				doFadeIn = false;
			}
		}
		if(doFadeOut) {
			clr[3] -= frameDelta * FADEOUT_MULTIPLIER;
			clr2[3] -= frameDelta * FADEOUT_MULTIPLIER;
			if(clr[3] <= 0.f) {
				clr[3] = 0.f;
				clr2[3] = 0.f;
				doFadeOut = false;
				setActive(false);
				returnState = -1;
			}
		}
		setupPrimitives();
		gfx->getRenderer()->drawQuad(quad);
		gfx->getRenderer()->drawQuad(quad2);
		
		SceneNode::update(frameDelta);	
	}	
}

void UIInputBox::setPosition(vector2 position_n) {
	position = position_n;
}

void UIInputBox::setInputBox(const std::string& label_text, bool alphanumeric_n) {
	size = vector2(800.f, 200.f);
	margin = 5.f;
	text_size = 23.f;
	text_font = MENU_FONT;
	alphanumeric = alphanumeric_n;

	std::string tmp = label_text;
	tmp += "\n\npress <enter> to continue or <escape> to cancel";

	label = new UILabel(gfx);
	label->setColor(MENU_TEXT_COLOR);
	label->setText(tmp, text_font, text_size);
	label->setPosition(vector2(position[0] + margin, position[1] + size[1] - margin - 20.f));
	gfx->addSceneNode(label, this);
	
	text = new UILabel(gfx);
	text->setColor(INPUTBOX_TEXT_COLOR);
	text->setText("", text_font, text_size);
	text->setPosition(vector2(position[0] + margin, position[1] + margin));
	gfx->addSceneNode(text, this);
}

void UIInputBox::setText(const std::string& text_n) {
	input = text_n;
}

int UIInputBox::pollStatus() {
	return returnState;	
}

std::string UIInputBox::getReturnString() {
	return returnString;
}

void UIInputBox::resetStatus() {
	returnState = false;
	returnString = "";
}

void UIInputBox::receiveKeyEvent(list<KeyEvent> events) {
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		switch((*x).key) {
		case KEY_ENTER:
			if((*x).state == GLFW_PRESS) {
				if(input.size() > 0) {
					returnState = true;
					returnString = input;
					fadeOut();
				} else {
					returnState = false;
					fadeOut();	
				}
			}
			break;
		case KEY_ESC:
			if((*x).state == GLFW_PRESS) {
				returnState = true;
				returnString = "";
				fadeOut();	
			}
			break;
		case KEY_BACKSPACE:
			if((*x).state == GLFW_PRESS) {
				if(input.size() > 0) {
					if(input[input.size()-1] == 46) dotCount = false;
					input = input.substr(0, input.size()-1);
				}
			}
			break;
		case KEY_SPACE:
			if((*x).state == GLFW_PRESS) {
				if(!alphanumeric && input.size() > INPUTBOX_CHAR_LIMIT) break;
				input += " ";
			}
			break;
		case KEY_CHR:
			if((*x).state == GLFW_PRESS && (*x).chr > 31 && (*x).chr < 160) {
				if(!alphanumeric) {					
					if(input.size() > INPUTBOX_CHAR_LIMIT) break;
					if((*x).chr < 45 || (*x).chr > 57 || (*x).chr == 47)
						break;
					else if((*x).chr == 46)
						if(!dotCount)
							dotCount = true;	
						else break;
					else if((*x).chr == 45)
						if(input.size() != 0)
							break;	
				}
				
				if((*x).chr > 64 && (*x).chr < 91)
					input += char((*x).chr + 32);
				else
					input += char((*x).chr);
			}
			break;
		}
	}

	text->setText(input, text_font, text_size);
}

void UIInputBox::fadeIn() {
	gfx->getUI()->darkenScreenFadeIn();
	clr[3] = 0.f;
	clr2[3] = 0.f;
	doFadeIn = true;
	label->fadeIn();
	text->fadeIn();
	setActive(true);
}

void UIInputBox::fadeOut() {
	gfx->getUI()->darkenScreenFadeOut();
	gfx->getInputHandler()->releaseControl(this);
	clr[3] = 0.8f;
	clr2[3] = 0.8f;
	doFadeOut = true;
	label->fadeOut();
	text->fadeOut();
}

// UIMenu class methods

UIMenu::UIMenu(GfxMgr* gfx_n) {
	gfx = gfx_n;	
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_MENU;
	
	labels_n = 0;
	quad = new DrawPrimitive;
	quad->type = PRIM_TYPE_QUAD;
	mouseControl = true;
	
	_isActive = false;
	doFadeIn = false;
	doFadeOut = false;
	returnItem = 0;
	currentSelection = 0;
	doAllowClose = false;
	_markForDeletion = 0;
}

UIMenu::~UIMenu() {
//	fadeOut();	
}

void UIMenu::setupPrimitive() {
	quad->vertices[0].set(position[0] - margin, position[1] - margin);
	quad->vertices[1].set(position[0] + size[0] + margin, position[1] - margin);
	quad->vertices[2].set(position[0] + size[0] + margin, position[1] + size[1] + margin);
	quad->vertices[3].set(position[0] - margin, position[1] + size[1] + margin);
	for(int x=0; x<4; x++) {
		quad->vertices_colors[x] = clr;	
	}	
}

void UIMenu::updateMousePos() {
	float mouse_x, mouse_y;
	vector2 mousePos = gfx->getMousePos(true);
	aabb2 bounding_box;
	int i = 1;
	currentSelection = 0;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		bounding_box = (*x)->getBoundingBox();
		if(bounding_box.checkPointInside(mousePos)) {
			(*x)->setColor(MENU_TEXT_COLOR_HOVER);
			currentSelection = i;
		} else {
			(*x)->setColor();	
		}
		i++;
	}	
}

void UIMenu::updateKeyboardSelection() {
	if(currentSelection == 0) currentSelection = labels_n;
	int y = 1;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		if(y == currentSelection) {
			(*x)->setColor(MENU_TEXT_COLOR_HOVER);
		} else {
			(*x)->setColor();	
		}
		y++;
	}	
}

void UIMenu::setActive(bool state) {
	if(state)
		gfx->getInputHandler()->takeControl(this);
	SceneNode::setActive(state);	
}

void UIMenu::update(float frameDelta) {
	if(_isActive) {
		if(!doFadeOut) {
			if(mouseControl)
				updateMousePos();
			else updateKeyboardSelection();
		}
		
		if(doFadeIn && doFadeOut) {
			// can't fade in and out at the same time
			doFadeIn = false;
			doFadeOut = true;
		}
		if(doFadeIn) {
			clr[3] += frameDelta * FADEIN_MULTIPLIER;
			if(clr[3] >= 0.8f) {
				clr[3] = 0.8f;
				doFadeIn = false;
			}	
		}
		if(doFadeOut) {
			clr[3] -= frameDelta * FADEOUT_MULTIPLIER;
			if(clr[3] <= 0.f) {
				clr[3] = 0.f;
				doFadeOut = false;
				setActive(false);
				returnItem = -1;
			}
		}
		setupPrimitive();
		gfx->getRenderer()->drawQuad(quad);
		SceneNode::update(frameDelta);
	}
}

void UIMenu::setPosition(vector2 position_n) {
	position = position_n;
	int i = 0;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		vector2 newPos = position;
		newPos.set(newPos[0] - text_size/2, newPos[1] + text_size*i);
		(*x)->setPosition(newPos);
		i++;
	}	
}

void UIMenu::setMenu(float margin_n, float text_size_n, bool font_n, bool mouseControl_n) {
	size = vector2(0.f, 0.f);
	margin = margin_n;
	text_size = text_size_n;
	text_font = font_n;
	mouseControl = mouseControl_n;
}

void UIMenu::addMenuOption(const std::string& text, bool isSelected) {
	UILabel* label = new UILabel(gfx);
	if(isSelected)
		label->setColor(MENU_TEXT_COLOR_SELECTED);
	else label->setColor(MENU_TEXT_COLOR);
	label->setText(text, text_font, text_size);
	label->setPosition(vector2());
	label->setActive(false);
//	label->fadeIn();
	
	float minMenuSizeX = text.size()*(text_size-text_size/2);
	if(minMenuSizeX > size[0]) size[0] = minMenuSizeX;
	size[1] = (labels_n+1)*text_size;
	
	gfx->addSceneNode(label, this);
	labels.push_back(label);
	labels_n++;
}

void UIMenu::addMenuBreak() {
	UILabel* label = new UILabel(gfx);
	label->setColor(MENU_TEXT_COLOR);
	label->setText("...", 0, text_size);
	label->setPosition(vector2());
	label->setActive(false);
//	label->fadeIn();
	
	//size[0] = 18;
	size[1] = (labels_n+1)*text_size;
	
	gfx->addSceneNode(label, this);
	labels.push_back(label);
	labels_n++;	
}

int UIMenu::pollStatus() {
	if(returnItem == 0)
		return 0;
	
	int tmp = returnItem;
	returnItem = 0;
	return tmp;	
}

void UIMenu::resetStatus() {
	returnItem = 0;
	currentSelection = 0;	
}

void UIMenu::receiveKeyEvent(list<KeyEvent> events) {
	if(mouseControl)
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		switch((*x).key) {
		case KEY_LEFT_MB:
			if((*x).state == GLFW_PRESS) {
				returnItem = currentSelection;
				if(currentSelection != 0)
					fadeOut();
				if(currentSelection == 0 && doAllowClose)
					fadeOut();
			}
			break;
		case KEY_RIGHT_MB:
			if((*x).state == GLFW_PRESS) {
				if(doAllowClose) {
					returnItem = 0;
					fadeOut();
				}
			}
			break;
		}
	}
	else
	for(list<KeyEvent>::iterator x = events.begin(); x != events.end(); x++) {
		if((*x).state == GLFW_PRESS)
			switch((*x).key) {
			case KEY_DOWN:
				if(currentSelection > 1)
					currentSelection--;
				break;
			case KEY_UP:
				if(currentSelection < labels_n)
					currentSelection++;
				break;
			case KEY_ENTER:
				returnItem = currentSelection;
				fadeOut();
				break;
			}	
	}
}

void UIMenu::fadeIn() {
	clr[3] = 0.f;
	doFadeIn = true;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		(*x)->fadeIn();	
	}
	setActive(true);
}

void UIMenu::fadeOut() {
	gfx->getInputHandler()->releaseControl(this);
	clr[3] = 0.8f;
	doFadeOut = true;
	for(list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		(*x)->fadeOut();	
	}
	//setActive(false);
}

void UIMenu::allowClose(bool doAllow) {
	if(doAllow) {
		doAllowClose = true;	
	} else {
		doAllowClose = false;	
	}
}

vector2 UIMenu::getSize() {
	return size;
}

vector2 UIMenu::getPosition() {
	return position;
}

// UICutscene methods

UICutscene::UICutscene(GfxMgr* gfx_n) {
	type = SN_TYPE_WIDGET;
	uiwidget_type = UIWIDGET_TYPE_CUTSCENE;
	gfx = gfx_n;
	frames = new ContainerSceneNode*[MAX_CUTSCENE_FRAMES];
	frames_n = 0;
	frame_timings = new float[MAX_CUTSCENE_FRAMES];
	currentFrame = -1;
	timeToNextFrame = 1.f;
	finished = false;
	_markForDeletion = 0;
}

void UICutscene::startFrame(float frameTime) {
	frame_timings[frames_n] = frameTime;
	frames[frames_n] = new ContainerSceneNode(gfx);
}

void UICutscene::addWidgetToFrame(UIWidget* widget) {
	widget->setActive(false);
	widget->fadeIn();
	gfx->addSceneNode(widget, frames[frames_n]);
}

void UICutscene::endFrame() {
	gfx->addSceneNode(frames[frames_n], this);
	frames_n++;
}

void UICutscene::update(float frameDelta) {
	if(finished) return;
	
	if(timeToNextFrame > 0) timeToNextFrame -= frameDelta;
	else {
		currentFrame++;
		if(currentFrame >= frames_n) {
			finished = true;
			return;
		}
		timeToNextFrame = frame_timings[currentFrame];
		if(currentFrame > 0) {
			for(list<SceneNode*>::iterator x = frames[currentFrame-1]->getChildren()->begin(); x != frames[currentFrame-1]->getChildren()->end(); x++) {
				UIWidget* tmp = (UIWidget*)(*x);
				tmp->fadeOut();
			}
		}

		frames[currentFrame]->setActive(true);
		for(list<SceneNode*>::iterator x = frames[currentFrame]->getChildren()->begin(); x != frames[currentFrame]->getChildren()->end(); x++) {
			UIWidget* tmp = (UIWidget*)(*x);
			tmp->fadeIn();
		}
	}
	
	SceneNode::update(frameDelta);
}	

bool UICutscene::pollStatus() {
	return finished;	
}

// UIMgr class methods
UIMgr::UIMgr(GfxMgr* gfx_n) {
	gfx = gfx_n;
	screenDark = 0.f;
	fadeIn = false;
	fadeOut = false;
	defaultFadeInMax = 0.7f;
	fadeInMax = defaultFadeInMax;
}

void UIMgr::init() {
	
}

void UIMgr::update(float frameDelta) {
	vector2 wparams = gfx->getRenderer()->getWindowParams();
	
	if(fadeIn) {
		if(screenDark < fadeInMax) screenDark += frameDelta * FADEIN_MULTIPLIER;
		else fadeIn = false;	
	} else if(fadeOut) {
		screenDark -= frameDelta * FADEOUT_MULTIPLIER;
		if(screenDark <= 0.f) {
			screenDark = 0.f;
			fadeOut = false;	
		}	
	}
	
	if(screenDark > 0.f)
		gfx->getRenderer()->drawFilledCircle(wparams*0.5f, wparams[0], color(0.f, 0.f, 0.f, screenDark));
	
	SceneNode::update(frameDelta);
	
	gfx->getRenderer()->drawTextArray();
}

void UIMgr::addWidget(SceneNode* widget) {
	if(widget->type == SN_TYPE_WIDGET) {
		gfx->addSceneNode(widget, this);
	}	
}

void UIMgr::darkenScreen(float coeff) {
	screenDark = coeff;	
}

void UIMgr::darkenScreenFadeIn(float max_fadein) {
	if(max_fadein == 0) fadeInMax = defaultFadeInMax;
	else fadeInMax = max_fadein;
	fadeIn = true;
}

void UIMgr::darkenScreenFadeOut() {
	fadeOut = true;
}
