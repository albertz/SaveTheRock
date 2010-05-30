/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */

#ifndef __SPRITE_H
#define __SPRITE_H


class AnimatedSpriteSceneNode : public SceneNode {
	unsigned int* textures; // opengl texture ids for each frame
	unsigned int currentTexture;
	int frames_n;	
	float animSpeed; // animation speed in frames per second
	bool isAnimRunning;
	bool isAnimLooping;
	bool isAnimForward;
	int startFrame;
	int endFrame;
	int currentFrame;
	int restFrame;
	float timeToNextFrame;
	
	SceneNode* animationCallback;
	
	public:
		DrawPrimitive* quad;
		vector2 size;
		vector2 tex_size;
		
		AnimatedSpriteSceneNode(GfxMgr* gfx_n);
		~AnimatedSpriteSceneNode();
		bool setSprite(int texture_id);
		void setFrame(int frame_n);
		void setAnimationLoop(int start_frame, int end_frame, float animSpeed);
		void setAnimationOnce(int start_frame, int end_frame, int restFrame, float animSpeed);
		void startAnimation();
		void stopAnimation();
		bool isAnimationRunning();
		void animateSelf(float frameDelta);
		void update(float frameDelta);
		void setAnimationEndCallback(SceneNode* node);
		DrawPrimitive* getPrimitive();
	//	void receiveCollisionEvent(CollisionEvent event);
};

class CollisionResponseSceneNode : public SceneNode {
	SceneNode* callbackNode;
	void setQuadEdgesAndConstraints(Primitive* quad);
	vector2 netForce;
	bool doGravity;
	bool doFriction;
	bool isStatic;
	
	public:
		Primitive* shape;
		
		CollisionResponseSceneNode(GfxMgr* gfx_n);
		~CollisionResponseSceneNode();
		void init(Primitive* shape_n, vector2 initPosition, vector2 initVelocity, vector2 initAcceleration, bool friction=false);
		void setPosition(vector2 position_n);
		void update(float frameDelta);
		void setStatic(bool state);
		bool checkStatic();
		bool receiveCollisionEvent(CollisionEvent event);
		void registerCollisionCallback(SceneNode* node);
		SceneNode* getCollisionCallback();
		Primitive* getCollisionPrimitive();
		int findVertex(vector2* vertex);
		void setGravity(bool state);
		void addForce(vector2 accel);
		void setForce(vector2 force);
		void stopMotion();
		vector2 getNetForce();	
		bool getFriction();
		bool inView();	
};

#endif
