/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

#ifdef _WIN32

Filesystem::Filesystem() {
	root = new char[strlen(FILESYSTEM_ROOT)+1];
	root = FILESYSTEM_ROOT;
	levels_dir = new char[strlen(FILESYSTEM_ROOT)+strlen(FILESYSTEM_LEVELS_DIR)+1];
	levels_dir[0] = 0;
	strcat(levels_dir, root);
	strcat(levels_dir, FILESYSTEM_LEVELS_DIR);

	custom_levels = new list<char*>;
	music_names = new list<char*>;

	sounds_dir = new char[strlen(FILESYSTEM_ROOT)+strlen(FILESYSTEM_SOUNDS_DIR)+1];
	sounds_dir[0] = 0;
	strcat(sounds_dir, root);
	strcat(sounds_dir, FILESYSTEM_SOUNDS_DIR);

	music_dir = new char[strlen(FILESYSTEM_ROOT)+strlen(FILESYSTEM_MUSIC_DIR)+1];
	music_dir[0] = 0;
	strcat(music_dir, root);
	strcat(music_dir, FILESYSTEM_MUSIC_DIR);
}

char* Filesystem::getLevelFilename(char* name, bool special) {
	int size;
	if(!special)
		size = strlen(levels_dir) + strlen(name) + strlen(".lvl");
	else
		size = strlen(levels_dir) + strlen("\\special\\") + strlen(name) + strlen(".lvl");
	level_filename = new char[size];
	for(int x=0; x<size; x++) level_filename[x] = 0;
	strcat(level_filename, levels_dir);
	if(special) strcat(level_filename, "\\special\\");
	strcat(level_filename, name);
	strcat(level_filename, ".lvl");
	return level_filename;
}

list<char*>* Filesystem::getCustomLevels() {
	custom_levels->clear();
	char* path = new char[MAX_PATH];
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;
	sprintf(path, "%s\\*", levels_dir);
	HANDLE hFind = FindFirstFile(path, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		while(FindNextFile(hFind, &fd))	{
			if(strcmp(fd.cFileName, "..")) {
				if(strcmp(fd.cFileName, "special") != 0) {
					int len = strlen(fd.cFileName);
					char* levelname = new char[len+1];
					for(int x=0; x<len; x++) levelname[x] = fd.cFileName[x];
					levelname[len-4] = 0;
					custom_levels->push_back(levelname);
				}
			}
		}
		FindClose(hFind);
	}
	return custom_levels;
}

list<char*>* Filesystem::getMusicFiles() {
	music_names->clear();
	char* path = new char[MAX_PATH];
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_DIRECTORY;
	sprintf(path, "%s\\*", music_dir);
	HANDLE hFind = FindFirstFile(path, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		while(FindNextFile(hFind, &fd))	{
			if(strcmp(fd.cFileName, "..")) {
				int len = strlen(fd.cFileName);
				char* musicname = new char[len+1];
				for(int x=0; x<len; x++) musicname[x] = fd.cFileName[x];
				musicname[len] = 0;
				music_names->push_back(musicname);
			}
		}
		FindClose(hFind);
	}
	return music_names;
}

char* Filesystem::getSoundFilename(char* name) {
	char* sound_filename = new char[MAX_PATH];
	sound_filename[0] = 0;
	sprintf(sound_filename, "%s%s.wav", sounds_dir, name);
	return sound_filename;
}

void Filesystem::freeMemory(FILESYSTEM_MEMORY type) {
	if(type == MEMORY_TYPE_LEVEL_FILENAME) {
		delete [] level_filename;
	} else if(type == MEMORY_TYPE_CUSTOM_LEVELS) {
		for(list<char*>::iterator x = custom_levels->begin(); x != custom_levels->end(); x++) {
			delete *x;
		}
		custom_levels->clear();
	} else if(type == MEMORY_TYPE_SOUND_FILENAME) {
		delete sound_filename;
	}
}

#else
static std::list<char*> musicfiles;
static std::list<char*> customlevels;

Filesystem::Filesystem() {}
char* Filesystem::getLevelFilename(char* name, bool special) { return NULL; }
std::list<char*>* Filesystem::getCustomLevels() { return &customlevels; }
std::list<char*>* Filesystem::getMusicFiles() { return &musicfiles; }
char* Filesystem::getSoundFilename(char* name) { return NULL; }
void Filesystem::freeMemory(FILESYSTEM_MEMORY type) {}
#endif
