/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __SOUND_H
#define __SOUND_H

class GfxMgr;

class SndMgr {
	GfxMgr* gfx;
	
	audiere::AudioDevicePtr device;
	audiere::SoundEffectPtr* sounds;
	
	float nextTrackTimer;
	
	bool __muteAll;
	bool __muteMusic;
	
	char** sound_names;
	int sounds_n;
	int music_tracks_n;
	
	void __addSound(const char* name, const char* filepath);
	void __addMusic(const char* name, const char* filepath);
	
	public:
	SndMgr(GfxMgr* gfx_n);
	void init();
	void update(float frameDelta);
	void playSound(const char* name, vector2 transmitter_pos);
	void playRandMusic();
	void playMusic(const char* name);
	void muteAll(bool state);
	void muteMusic(bool state);
};

#endif
