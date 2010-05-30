/* AnimationPacker
Takes numbered 24-bit bitmap frames (e.g 01.bmp, 02.bmp etc) from a directory
and packs them into an .anim file for the game engine
*/
#include "engine/headers.h"

using namespace std;

int main(int argc, char* argv[]) {	

	if(argc < 6) {
		cout << "usage:\nanimationpacker.exe <input_dir> <WIDTH> <HEIGHT> <frames_n> <output_file>\n\
input_dir should be a directory filled with 24-bit bmp's named 00.bmp, 01.bmp, 02.bmp, .. 25.bmp etc.\n\
All BMP files should be of equal dimensions, which, in turn, should be a power of two.\n\
frames_n is the number of such bitmaps in the directory\n";
		return 0;
	}
	
	// engine start
	cout << "*----------------------*\n* AnimationPacker v1.0 *\n*----------------------*\n\nStarting..\n\n";
//	GfxMgr* gfx = new GfxMgr();
//	gfx->init(false, false, false, false); // don't initialize the engine, we only need a few simple functions
	// parse arguments
	ofstream output_file(argv[5], ios::binary);
	if(!output_file) { cout << "error: could not open output file" << endl; return 1;}
	int frames_n = atoi(argv[4]);
	int real_width = atoi(argv[2]);
	int real_height = atoi(argv[3]);
	char* dir = argv[1];
	//
	
	char path[32];
	for(int q=0; q<32; q++) path[q] = 0;
	// we peek at the first BMP to determine memory usage
	cout << "Parsing sprite information... ";
	strcat(path, dir);
	strcat(path, "\\00.bmp");
	BMP_Loader* bmp = new BMP_Loader();
//	bmp->init();
	if(!bmp->openFile(path)) {cout << "error: could not open " << path << endl; return 1;}
	int size_x, size_y;
	unsigned int datalen;
	bool error;
	char* data = new char[4096];
	bmp->readFileToBuffer(&size_x, &size_y, &datalen, &error);
	if(error)  {cout << "error: could not read " << path << " \nProbably not a 24-bit bmp file." << endl; return 1;}
	bmp->closeFile();
	//
	cout << "Done!\nsize_x: " << size_x << "\nsize_y: " << size_y << "\ndatalen: " << datalen << endl << endl;
	
	// allocate memory for frames
	char frameData[frames_n][datalen];

	for(int x=0; x<frames_n; x++) {
		// construct path
		for(int q=0; q<32; q++) path[q] = 0;
		strcat(path, dir);
		strcat(path, "\\");
		if(x < 10)
			strcat(path, "0");
		char tmp[3];
		itoa(x, tmp, 10);
		strcat(path, tmp);
		strcat(path, ".bmp");
		if(!bmp->openFile(path)) {cout << "error: could not open " << path << endl; return 1;}
		else cout << "Trying to read " << path << "... "; 
		bmp->readFileToBuffer(&size_x, &size_y, &datalen, &error);
		char* buffer = bmp->getBuffer();
	
		for(int u=0; u<datalen; u++) frameData[x][u] = buffer[u];
		if(error)  {cout << "error: could not read " << path << " \nProbably not a 24-bit bmp file." << endl; return 1;}
		else { cout << "Success!" << endl; };
		bmp->closeFile();
	}
	RGB_RAW_ANIMATION_FILE fileAttr;
	fileAttr.size_x = size_x;
	fileAttr.size_y = size_y;
	fileAttr.real_size_x = real_width;
	fileAttr.real_size_y = real_height;
	fileAttr.frames_n = frames_n;
	cout << endl << "Writing data to " << argv[5] << " ... ";
	output_file.write((char*)(&fileAttr), sizeof(fileAttr));
	for(int x=0; x<frames_n; x++) {
		output_file.write(frameData[x], size_x*size_y*3);
	}
	output_file.close();
	cout << "Done!" << endl;
}
