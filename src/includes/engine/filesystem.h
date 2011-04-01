/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H

#include <string>

class Filesystem {
	std::string root;
	std::string levels_dir;
	std::string sounds_dir;
	std::string music_dir;
	
public:
	Filesystem();
	std::string getLevelFilename(const std::string& name, bool special=false);
	std::list<std::string> getCustomLevels();
	std::list<std::string> getMusicFiles();
	
	std::string getSoundFilename(const std::string& name);
};

#endif
