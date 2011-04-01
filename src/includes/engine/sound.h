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
	
	std::string* sound_names;
	int sounds_n;
	int music_tracks_n;
	
	void __addSound(const std::string& name, const std::string& filepath);
	void __addMusic(const std::string& name, const std::string& filepath);
	
	public:
	SndMgr(GfxMgr* gfx_n);
	void init();
	void update(float frameDelta);
	void playSound(const std::string& name, vector2 transmitter_pos);
	void playRandMusic();
	void playMusic(const std::string& name);
	void muteAll(bool state);
	void muteMusic(bool state);
};

#endif
