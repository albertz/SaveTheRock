/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

// NamedPosStringParser


NamedPosStringParser::NamedPosStringParser() {
}

void NamedPosStringParser::__parseFields() {
	char* buffer = new char[string.size() + 1];
	strcpy(buffer, string.c_str());
	char* field = strtok(buffer, "_");
	while(field != NULL) {
		fields.push_back(field);
		field = strtok(NULL, "_");
	}
	delete[] buffer;
}

void NamedPosStringParser::__parseVarValuePairs() {
	for(list<std::string>::iterator x = fields.begin(); x != fields.end(); x++) {
		char* buffer = new char[x->size() + 1];
		strcpy(buffer, x->c_str());
		char* var = strtok(buffer, "=");
		char* value = strtok(NULL, "=");
		if(var && value) {
			vars.push_back(var);
			values.push_back(value);
		}
		else
			cout << "NamedPosStringParser::__parseVarValuePairs: bad field '" << *x << "' in string '" << string << "'" << endl;
		delete[] buffer;
	}	
}

void NamedPosStringParser::parseString(const std::string& string_n) {
	fields.clear();
	vars.clear();
	values.clear();
	
	string = string_n;
	
	__parseFields();
	__parseVarValuePairs();
}

list<std::string> NamedPosStringParser::getVars() {
	return vars;	
}

list<std::string> NamedPosStringParser::getValues() {
	return values;	
}


// bitmap2 class methods

bitmap2::bitmap2(int size_x, int size_y) {
	columns = size_x/BITS_IN_INT;
	rows = size_y/BITS_IN_INT;
	array = new unsigned int*[columns];
	for(int x=0; x<columns; x++) {
		array[x] = new unsigned int[rows];
		for(int y=0; y<rows; y++)
			array[x][y] = 0;	
	}
}

bitmap2::~bitmap2() {
	delete [] array;
}

bool bitmap2::get(int x, int y) {
	return !((array[x/BITS_IN_INT][y/BITS_IN_INT] & 1<<(y%BITS_IN_INT)) == 0);	
}

void bitmap2::set(int x, int y) {
	array[x/BITS_IN_INT][y/BITS_IN_INT] = array[x/BITS_IN_INT][y/BITS_IN_INT] | 1<<(y%BITS_IN_INT);
}

void bitmap2::unset(int x, int y) {
	array[x/BITS_IN_INT][y/BITS_IN_INT] = array[x/BITS_IN_INT][y/BITS_IN_INT] & ~(1<<y%BITS_IN_INT);
}

unsigned int** bitmap2::getArray() {
	return array;		
}

void bitmap2::setLevelArray(unsigned int newArray[LEVEL_SIZE_X/BITS_IN_INT][LEVEL_SIZE_Y/BITS_IN_INT]) {
	for(int x=0; x<LEVEL_SIZE_X/BITS_IN_INT; x++)
	for(int y=0; y<LEVEL_SIZE_Y/BITS_IN_INT; y++)
		array[x][y] = newArray[x][y];
}

// BMP_Loader class methods

BMP_Loader::BMP_Loader() {
	isOpened = false;
}

bool BMP_Loader::openFile(const std::string& filename) {
	if(isOpened) closeFile();
	currentFile.open(filename.c_str(), ios::binary);
	if(currentFile.is_open()) {
		isOpened = true;
		return true;
	}
	isOpened = false;
	return false;
}

void BMP_Loader::readFileToBuffer(int* size_x, int* size_y, unsigned int* datalen, bool* error) {
	*error = false;
	if(isOpened) {
		currentFile.read((char*)(&magic_bits), sizeof(magic_bits));
		if(magic_bits.magicbits[0] != 'B' || magic_bits.magicbits[1] != 'M') {
			*error = true;
			return;	
		}
		currentFile.read((char*)(&header), sizeof(header));
		currentFile.read((char*)(&info), sizeof(info));
		*size_x = info.width; *size_y = info.height; *datalen = info.datasize;
		currentBuffer = std::string(info.datasize, '\0');
		currentFile.seekg(header.bmp_offset);
		currentFile.read(&currentBuffer[0], info.datasize);
	} else *error = true;	
}

void BMP_Loader::writeBufferToFile(const std::string& filename) {
	ofstream output_file(filename.c_str(), ios::binary);
	output_file.write((char*)(&magic_bits), sizeof(magic_bits));
	output_file.write((char*)(&header), sizeof(header));
	output_file.write((char*)(&info), sizeof(info));
	output_file.write(&currentBuffer[0], info.datasize);
}

std::string BMP_Loader::getBuffer() {
	return currentBuffer;	
}

void BMP_Loader::closeFile() {
	if(isOpened) {
		currentBuffer = "";
		currentFile.close();
		isOpened = false;
	}
}
