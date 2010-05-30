/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

// NamedPosStringParser


NamedPosStringParser::NamedPosStringParser() {
	fields = NULL;
	vars = NULL;
	values = NULL;
}

void NamedPosStringParser::__parseFields() {
	char* field = strtok(string, "_");
	while(field != NULL) {
		char* tmp = new char[NAMED_POSITION_LEN];
		for(int x=0; x<strlen(field); x++) tmp[x] = field[x];
		tmp[strlen(field)] = 0;
		fields->push_back(tmp);
		field = strtok(NULL, "_");
	}	
}

void NamedPosStringParser::__parseVarValuePairs() {
	for(list<char*>::iterator x = fields->begin(); x != fields->end(); x++){
		char* var = new char[NAMED_POSITION_LEN];
		char* value = new char[NAMED_POSITION_LEN];
		
		char* tmp = strtok(*x, "=");
		for(int p=0; p<strlen(tmp); p++) var[p] = tmp[p];
		var[strlen(tmp)] = 0;
		tmp = strtok(NULL, "=");
		for(int p=0; p<strlen(tmp); p++) value[p] = tmp[p];
		value[strlen(tmp)] = 0;
		vars->push_back(var);
		values->push_back(value);
	}	
}

void NamedPosStringParser::parseString(char* string_n) {
	fields = new list<char*>;
	vars = new list<char*>;
	values = new list<char*>;
	
	len = strlen(string_n);
	string = new char[len+1];
	for(int x=0; x<len; x++) string[x] = string_n[x];
	string[len] = 0;
	
	__parseFields();
	__parseVarValuePairs();
}

list<char*>* NamedPosStringParser::getVars() {
	return vars;	
}

list<char*>* NamedPosStringParser::getValues() {
	return values;	
}

void NamedPosStringParser::freeMemory() {
	fields->clear();
	vars->clear();
	values->clear();
	delete fields, vars, values;
	fields = NULL;
	vars = NULL;
	values = NULL;
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
	currentBuffer = NULL;		
}

bool BMP_Loader::openFile(char* filename) {
	if(isOpened) closeFile();
	currentFile.open(filename, ios::binary);
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
		currentBuffer = new char[info.datasize];
		currentFile.seekg(header.bmp_offset);
		currentFile.read(currentBuffer, info.datasize);
	} else *error = true;	
}

void BMP_Loader::writeBufferToFile(char* filename) {
	ofstream output_file(filename, ios::binary);
	output_file.write((char*)(&magic_bits), sizeof(magic_bits));
	output_file.write((char*)(&header), sizeof(header));
	output_file.write((char*)(&info), sizeof(info));
	output_file.write(currentBuffer, info.datasize);
}

char* BMP_Loader::getBuffer() {
	return currentBuffer;	
}

void BMP_Loader::closeFile() {
	if(isOpened) {
		delete [] currentBuffer;
		currentFile.close();
		isOpened = false;
	}
}
