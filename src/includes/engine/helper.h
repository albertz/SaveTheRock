/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __HELPER_H
#define __HELPER_H

class NamedPosStringParser {
	char* string;
	int len;
	
	std::list<char*>* fields;
	std::list<char*>* vars;
	std::list<char*>* values;
	
	void __parseFields();
	void __parseVarValuePairs();
	
	public:
	NamedPosStringParser();
	void parseString(char* string_n);
	std::list<char*>* getVars();
	std::list<char*>* getValues();
	void freeMemory();
};

class bitmap2 {
	public:
	unsigned int** array;
	int arrayLen;
	int rows, columns;
	bitmap2(int size_x, int size_y);
	~bitmap2();
	bool get(int index_x, int index_y);
	void set(int index_x, int index_y);
	void set(int index_x, int index_y, bool value);
	void unset(int index_x, int index_y);
	unsigned int** getArray();
	void setLevelArray(unsigned int newArray[LEVEL_SIZE_X/BITS_IN_INT][LEVEL_SIZE_Y/BITS_IN_INT]);
};

struct BMP_MAGIC_BITS {
	unsigned char magicbits[2];
};

struct BMP_HEADER {
	uint32_t filesize;
	uint16_t reserved_a;
	uint16_t reserved_b;
	uint32_t bmp_offset;
};

struct BMP_INFO {
	uint32_t headersize;
	uint32_t width;
	uint32_t height;
	uint16_t colplanes;
	uint16_t bpp;
	uint32_t compress;
	uint32_t datasize;
	uint32_t hres;
	uint32_t vres;
	uint32_t colorsnum;
	uint32_t impcolorsnum;	
};

class BMP_Loader {
	public:
		BMP_MAGIC_BITS magic_bits;
		BMP_HEADER header;
		BMP_INFO info;
		
		std::ifstream currentFile;
		bool isOpened;
		char* currentBuffer; // BMP data buffer
		char* tmpData;
		BMP_Loader();
		// constructor
		bool openFile(char* filename);
		// opens filename for binary reading
		void readFileToBuffer(int* size_x, int* size_y, unsigned int* datalen, bool* error);
		// reads file data and returns a pointer to it, fills in size_x and size_y with the bitmap's width and height
		// if error is 1 after a call, then the file was not read (most probably wasn't a BMP)
		// and the buffer might be null or point to incomplete data
		// strongly advise checking error value after reading a file
		void writeBufferToFile(char* filename);
		// writes buffer data to a file
		char* getBuffer();
		// returns a pointer to the current buffer
		void closeFile();
		// closes the opened file and frees allocated memory
};

#endif
