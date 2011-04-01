/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */

#ifndef __UI_H
#define __UI_H

class GfxMgr;
class ContainerSceneNode;

enum UIWIDGET_TYPE {
	UIWIDGET_TYPE_LABEL,	
	UIWIDGET_TYPE_MENU,
	UIWIDGET_TYPE_INPUTBOX,
	UIWIDGET_TYPE_INFOBOX,
	UIWIDGET_TYPE_IMAGE,
	UIWIDGET_TYPE_CUTSCENE
};

class UIWidget : public SceneNode {
	public:
	bool doFadeIn;
	bool doFadeOut;
	color defaultColor;
	bool defaultColorSet;
	
	color clr;
	UIWIDGET_TYPE uiwidget_type;
	UIWidget();
	void update(float frameDelta);
	void setColor(color clr_n);
	void setColor();
	void fadeIn();
	void fadeOut();
};

class UILabel : public UIWidget {
	std::string text;
	bool text_font;
	float text_size;
	vector2 position;
	bool level_label;
	
	aabb2 bounding_box;
	
	public:
	UILabel(GfxMgr* gfx_n);
	void update(float frameDelta);
	void setPosition(vector2 position_n);
	void setText(const std::string& text_n, bool font_n, float size_n, bool level_label_n=false);
	aabb2 getBoundingBox();
};

class UIImage : public UIWidget {
	vector2 position;
	vector2 size;
	
	float transparency;
	
	AnimatedSpriteSceneNode* sprite;
	public:
	UIImage(GfxMgr* gfx_n);
	void update(float frameDelta);
	void setImage(const std::string& texture_name);
	void setPosition(vector2 position_n);
	vector2 getSize();
};

class UIInfoBox : public UIWidget {
	int labels_n;
	std::list<UILabel*> labels;
	bool doAllowClose;
	
	DrawPrimitive* quad;
	
	vector2 position;
	vector2 size;
	float margin;
	float text_size;
	bool text_font;
	float timeOut;

	void setupPrimitive();
	
	public:
		UIInfoBox(GfxMgr* gfx_n);
		~UIInfoBox();
		void setActive(bool state);
		void update(float frameDelta);
		void setPosition(vector2 position_n);
		void setText(float margin_n, float text_size_n, bool font_n);
		void setTimeout(float seconds);
		void addText(const std::string& text, bool isGray=false);
		void addBreak();
		void receiveKeyEvent(std::list<KeyEvent> events);
		void fadeIn();
		void fadeOut();
		void allowClose(bool doAllow);
		vector2 getSize();
		vector2 getPosition();
};

class UIInputBox : public UIWidget {
	UILabel* label;
	UILabel* text;
	std::string input;
	
	bool alphanumeric;
	color clr2;
	
	std::string returnString;
	int returnState;
	
	vector2 position;
	vector2 size;
	float text_size;
	bool text_font;
	float margin;
	
	DrawPrimitive* quad;
	DrawPrimitive* quad2;
	bool dotCount;
	
	void setupPrimitives();
	
	public:
	UIInputBox(GfxMgr* gfx_n);
	~UIInputBox();
	void setActive(bool state);
	void update(float frameDelta);
	void setPosition(vector2 position_n);
	void setInputBox(const std::string& label_text, bool alphanumeric_n=true);
	void setText(const std::string& text_n);
	int pollStatus();
	std::string getReturnString();
	void resetStatus();
	void receiveKeyEvent(std::list<KeyEvent> events);
	void fadeIn();
	void fadeOut();
};

class UIMenu : public UIWidget {
	int labels_n;
	std::list<UILabel*> labels;
	
	DrawPrimitive* quad;
	bool doAllowClose;
	bool mouseControl;
	
	vector2 position;
	vector2 size;
	float margin;
	float text_size;
	bool text_font;
	
	int currentSelection;
	int returnItem;
	
	void setupPrimitive();
	void updateMousePos();
	void updateKeyboardSelection();
	
	public:
	UIMenu(GfxMgr* gfx_n);
	~UIMenu();
	void setActive(bool state);
	void update(float frameDelta);
	void setPosition(vector2 position_n);
	void setMenu(float margin_n, float text_size_n, bool font_n, bool mouseControl_n=true);
	void addMenuOption(const std::string& text, bool isSelected=false);
	void addMenuBreak();
	int pollStatus();
	void resetStatus();
	void receiveKeyEvent(std::list<KeyEvent> events);
	void fadeIn();
	void fadeOut();
	void allowClose(bool doAllow);
	vector2 getSize();
	vector2 getPosition();
};

class UICutscene : public UIWidget {
	ContainerSceneNode** frames;
	int frames_n;
	float* frame_timings;
	
	int currentFrame;
	float timeToNextFrame;
	
	bool finished;
	
	public:
	UICutscene(GfxMgr* gfx_n);
	void startFrame(float frameTime);
	void addWidgetToFrame(UIWidget* widget);
	void endFrame();
	void update(float frameDelta);
	bool pollStatus();
};

class UIMgr : public SceneNode {
	std::list<UIWidget> widgets;
	
	float screenDark;
	bool fadeIn;
	bool fadeOut;
	
	float fadeInMax;
	float defaultFadeInMax;
		
	public:
	UIMgr(GfxMgr* gfx_n);
	void init();
	void update(float frameDelta);
	void addWidget(SceneNode* widget);
	void darkenScreen(float coeff);
	void darkenScreenFadeIn(float max_fadein=0.f);
	void darkenScreenFadeOut();
};

#endif
