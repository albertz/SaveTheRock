/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H

enum FILESYSTEM_MEMORY {
	MEMORY_TYPE_LEVEL_FILENAME = 0,
	MEMORY_TYPE_CUSTOM_LEVELS = 1,
	MEMORY_TYPE_SOUND_FILENAME = 2,
};

class Filesystem {
	char* root;
	char* levels_dir;
	char* sounds_dir;
	char* music_dir;
	
	char* level_filename;
	std::list<char*>* custom_levels;
	std::list<char*>* music_names;
	char* sound_filename;
	
	public:
	Filesystem();
	char* getLevelFilename(char* name, bool special=false);
	std::list<char*>* getCustomLevels();
	std::list<char*>* getMusicFiles();
	
	char* getSoundFilename(char* name);
	
	void freeMemory(FILESYSTEM_MEMORY type);
};

#endif
