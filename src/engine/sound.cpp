/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace audiere;
using namespace std;

// SndMgr class

SndMgr::SndMgr(GfxMgr* gfx_n) {
	gfx = gfx_n;
//	callback = NULL;
	__muteAll = false;
	__muteMusic = false;
	
	sounds = new SoundEffectPtr[MAX_SOUNDS_N];
	sound_names = new char*[MAX_SOUNDS_N];
	sounds_n = 0;
	music_tracks_n = 0;
	
	nextTrackTimer = MUSIC_TRACK_SWITCH_TIME;
}

void SndMgr::__addSound(const char* name, const char* filepath) {
	sounds[sounds_n] = OpenSoundEffect(device, filepath, SINGLE);
	if(sounds[sounds_n].get() == NULL)
		std::cout << "failed to load sound: " << filepath << std::endl;
	sound_names[sounds_n] = new char[MAX_SOUND_NAME];
	int namelen = strlen(name);
	for(int x=0; x<namelen; x++) sound_names[sounds_n][x] = name[x];
	sound_names[sounds_n][namelen] = 0;
	sounds_n++;
}

void SndMgr::__addMusic(const char* name, const char* filepath) {
	sounds[sounds_n] = OpenSoundEffect(device, filepath, SINGLE);
	if(sounds[sounds_n].get() == NULL)
		std::cout << "failed to load music: " << filepath << std::endl;
	sound_names[sounds_n] = new char[MAX_SOUND_NAME];
	int namelen = strlen(name);
	for(int x=0; x<namelen; x++) sound_names[sounds_n][x] = name[x];
	sound_names[sounds_n][namelen] = 0;
	sounds_n++;
}

void SndMgr::init() {
	device = OpenDevice();
	if(device.get() == NULL)
		std::cout << "failed to open sound device" << std::endl;
	
	__addSound("LASER", "../media/sounds/laser.wav");
	__addSound("EXPLOSION", "../media/sounds/explosion.wav");
	__addSound("PLASMA", "../media/sounds/plasma.wav");
	__addSound("GRAVLIFT", "../media/sounds/gravlift.wav");
	
	list<char*>* music = gfx->getFilesystem()->getMusicFiles();
	int music_n = 0;
	for(list<char*>::iterator x = music->begin(); x != music->end(); x++) {
		char* tmp = new char[MAX_PATH];
		tmp[0] = 0;
		sprintf(tmp, "MUSIC_%i", music_n);
		char* path = new char[MAX_PATH];
		sprintf(path, "../media/music/%s", *x);
		__addMusic(tmp, path);	
		delete [] tmp;
		delete [] path;
		music_n++;
	}
	music_tracks_n = music_n;
}

void SndMgr::update(float frameDelta) {
	nextTrackTimer -= frameDelta;
	if(nextTrackTimer <= 0.f) {
		playRandMusic();
		nextTrackTimer = MUSIC_TRACK_SWITCH_TIME;	
	}
}

void SndMgr::playSound(const char* name, vector2 transmitter_pos) {
	if(__muteAll) return;
	
	vector2 camerapos = gfx->getCamera()->getAbsoluteTranslation();
	vector2 wparams = gfx->getRenderer()->getWindowParams();
	vector2 center = camerapos + wparams*0.5f;
	aabb2 screen;
	screen.bottom_left = camerapos;
	screen.top_right = camerapos + wparams;
	if(!screen.checkPointInside(transmitter_pos)) return;
	
	int sign = 0;
	if(transmitter_pos[0] < center[0]) sign = -1;
	else if(transmitter_pos[0] > center[0]) sign = 1;
	
	float pan = fabs(center[0] - transmitter_pos[0])/(wparams[0]*0.5f) * sign;
	
	for(int x=0; x<sounds_n; x++) {
		if(!strcmp(sound_names[x], name)) {
			if(sounds[x]) {
				sounds[x]->setPan(pan);
				sounds[x]->setVolume(0.5f);
				sounds[x]->play();
			}
		}	
	}

}

void SndMgr::playRandMusic() {
	for(int x=0; x<sounds_n; x++) {
		if(sounds[x])
			sounds[x]->stop();	
	}
	
	if(music_tracks_n == 0) return;
	int x = rand() % music_tracks_n;
	char* name = new char[MAX_PATH];
	sprintf(name, "MUSIC_%i", x);
	playMusic(name);
	delete [] name;	
}

void SndMgr::playMusic(const char* name) {
	if(__muteAll) return;
	if(__muteMusic) return;

	for(int x=0; x<sounds_n; x++) {
		if(!strcmp(sound_names[x], name)) {
			if(sounds[x]) {
				sounds[x]->setVolume(0.2f);
				sounds[x]->play();
			}	
		}	
	}
}

void SndMgr::muteAll(bool state) {
	__muteAll = state;
	for(int x=0; x<sounds_n; x++) {
		if(sounds[x])
			sounds[x]->stop();	
	}	
}

void SndMgr::muteMusic(bool state) {
	__muteMusic = state;
	for(int x=0; x<sounds_n; x++) {
		if(sounds[x])
			sounds[x]->stop();	
	}
}

