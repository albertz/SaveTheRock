/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"
#include <dirent.h>

Filesystem::Filesystem() {
	root = FILESYSTEM_ROOT;
	levels_dir = root + FILESYSTEM_LEVELS_DIR; 
	sounds_dir = root + FILESYSTEM_SOUNDS_DIR;
	music_dir = root + FILESYSTEM_MUSIC_DIR;
}

std::string Filesystem::getLevelFilename(const std::string& name, bool special) {
	std::string level_filename = levels_dir;
	if(special) level_filename += "special/";
	level_filename += name + ".lvl";
	return level_filename;
}

std::list<std::string> Filesystem::getCustomLevels() {
	std::list<std::string> custom_levels;
#ifdef _WIN32
	char path[MAX_PATH];
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
					custom_levels.push_back(levelname);
					delete[] levelname;
				}
			}
		}
		FindClose(hFind);
	}
#else
	
#endif
	return custom_levels;
}

std::list<std::string> Filesystem::getMusicFiles() {
	std::list<std::string> music_names;
#ifdef _WIN32
	char path[MAX_PATH];
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
				music_names.push_back(musicname);
				delete musicname;
			}
		}
		FindClose(hFind);
	}
#else
	
#endif
	return music_names;
}

std::string Filesystem::getSoundFilename(const std::string& name) {
	return sounds_dir + name + ".wav";
}


