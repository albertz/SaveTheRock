/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

using namespace std;

bool get_bit(int index, int x) {
	return !((x & 1<<index) == 0);	
}

renderer::renderer(int width, int height, bool fs, bool vsync, int fsaa) {
	w_width = width; w_height = height;
	fullscreen = fs;
	RGBA_tmp = NULL;
	heightmap = NULL;
	vboAvailable = false;
	texRectAvailable = false;
	txt_vbo_ptr = 0;
	txt_vertex_array = NULL;
	txt_texcoords_array = NULL;
	lvl_vbo_ptr = 0;
	lvl_vertex_array = NULL;
	bg_vertex_array = NULL;
	bg_texcoords_array = NULL;

	
	// sets up the opengl window 
	if(!glfwInit()) {
		cout << "Error! Could not init OpenGL!" << endl;
		exit(-1);	
	}
	
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	if(fsaa > 4) fsaa = 4;
	
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, fsaa);
	
	int mode = GLFW_WINDOW;
	if(fullscreen)
		mode = GLFW_FULLSCREEN;
	assert(glfwOpenWindow(w_width, w_height, 0, 0, 0, 0, 0, 0, mode) == GL_TRUE);	
	glfwGetWindowSize(&w_width, &w_height);
	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
  	gluOrtho2D(-0.5f, w_width-0.5f, -0.5f, w_height-0.5f);
  	glMatrixMode(GL_MODELVIEW);
  	glEnable(GL_BLEND);
    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    	glEnable(GL_LINE_SMOOTH);
    	glDisable(GL_DEPTH_TEST);
    	glHint(GL_LINE_SMOOTH_HINT, 2);
    	glfwDisable(GLFW_MOUSE_CURSOR);

    	if(glfwExtensionSupported("GL_EXT_texture_rectangle") || glfwExtensionSupported("GL_ARB_texture_rectangle")) {
		texRectAvailable = true;
	} else texRectAvailable = false;
	
    	if(glfwExtensionSupported("GL_ARB_vertex_buffer_object")) {
		vboAvailable = true;
		C_glGenBuffers = (P_glGenBuffers)glfwGetProcAddress("glGenBuffers");
		C_glBindBuffer = (P_glBindBuffer)glfwGetProcAddress("glBindBuffer");
		C_glBufferData = (P_glBufferData)glfwGetProcAddress("glBufferData");
		C_glDeleteBuffers = (P_glDeleteBuffers)glfwGetProcAddress("glDeleteBuffers");
		if(!(C_glGenBuffers && C_glBindBuffer && C_glBufferData && C_glDeleteBuffers)) {
			vboAvailable = false;
		}
	} else vboAvailable = false;

	if(vsync)
    		glfwSwapInterval(1);
    	
    	if(vboAvailable) {
		generateVBO(&bg_v_vbo);
		generateVBO(&bg_t_vbo);
   	 	generateVBO(&level_tri_vbo);
		generateVBO(&txt_t_vbo);
		generateVBO(&txt_v_vbo);
		generateVBO(&txt_c_vbo);
	}
   	
    	loadFontTexture();
}

void renderer::setWindowTitle(char* title) {
	glfwSetWindowTitle(title);	
}

void renderer::frameStart(vector2 bg_displacement, aabb2 view_bb) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(BG_COLOR);
	glLoadIdentity();
//	glTranslatef (-0.375f, -0.375f, 0.f); // a quick hack for cleaner pixel drawing
	glPushMatrix();
	drawBackground(bg_displacement, view_bb);
	glPopMatrix();
	txt_vbo_ptr = 0;
	txt_clr_ptr = 0;
	txt_vertex_array = new float[VERTEX_ARRAY_SIZE];
	txt_texcoords_array = new float[VERTEX_ARRAY_SIZE];
	txt_colors_array = new float[VERTEX_ARRAY_SIZE];
}

void renderer::frameEnd() {	
	glfwSwapBuffers();	
}

void renderer::generateVBO(unsigned int* id) {
	if(vboAvailable)
		C_glGenBuffers(1, id);
	else assert("Should not be here - generateVBO()" == 0);
}

void renderer::destroyVBO(unsigned int* id) {
	if(vboAvailable)
		C_glDeleteBuffers(1, id);
	else assert("Should not be here - destroyVBO()" == 0);
}
/*
vector3 renderer::vector2to3(vector2 x) {
	return vector3(x[0], x[1], 1.f);	
}*/

float* renderer::generateBoxKernel() {
	int size = 4;
	float* result = new float[size];
	for(int x=0; x<size; x++) {
		result[x] = 0.25f;	
	}
	return result;
}

float* renderer::generateTentKernel(float scale) {
	int size = 8;
	float* result = new float[size];
	float step = 1.f/size*2.f;
	float t = -1.f;
	for(int x=0; x<size; x++) {
		if(abs(t) < abs(scale)) result[x] = 1 - abs(t/scale);
		else result[x] = 0;
		t += step;
	}
	return result;
}

int* renderer::convolute(int* f, int f_size, float* g, int g_size) {
	// convolutes two discrete sampled functions
	int* result = new int[f_size];
	for(int x=0; x<f_size; x++) {
		result[x] = 0;
		for(int y=0; y<g_size; y++) {
			if(y > x) result[x] = f[x];
			else result[x] += int(floor(f[x-y]*g[y]));	
		}
	}
	delete [] f;
	delete [] g;
	return result;
}

int* renderer::generateBackgroundHeightmap(int tiles_x, int tiles_y) {
	if(heightmap) delete heightmap;
	heightmap = new int[tiles_x];
	
	heightmap[0] = rand() % (tiles_y/16);
	//int sign = 0;
	for(int x=1; x<tiles_x; x++) {
	//	heightmap[x] = rand() % tiles_y/2;
		if(rand() % 2) {
			if(heightmap[x-1] != 0)
				heightmap[x] = heightmap[x-1];
			else {
				heightmap[x] = rand() % (tiles_y/16) + 1;	
			}
		} else {
			heightmap[x] = rand() % (tiles_y/16);
		}
		if(heightmap[x] < 0) heightmap[x] = 0;
		if(heightmap[x] > tiles_y) heightmap[x] = tiles_y;
	}	
	
	
	// apply tent filter
	float* tent = generateTentKernel(1.f);
	heightmap = convolute(heightmap, tiles_x, tent, 8);
	// apply lowpass to limit the maximum height
	float* box = generateBoxKernel();
	heightmap = convolute(heightmap, tiles_x, box, 4);

	for(int x=0; x<tiles_x; x++) {
		if(heightmap[x] < 0) heightmap[x] = 0;
		if(heightmap[x] > tiles_y) heightmap[x] = tiles_y;
	}
	
	return heightmap;
}

void renderer::generateBackground(unsigned int texture, float tex_size_x, float tex_size_y) {
	bg_tex = texture;
	int size_x = LEVEL_SIZE_X*PPT_X;
	int size_y = LEVEL_SIZE_Y*PPT_Y;
	int ptr = 0;
	ts_x = 24.f; // tile size x
	ts_y = 24.f;
	int tiles_x = int((size_x+10)/ts_x), tiles_y = int((size_y+10)/ts_y);
	heightmap = generateBackgroundHeightmap(tiles_x, tiles_y);
	if(bg_vertex_array) delete [] bg_vertex_array;
	bg_vertex_array = new float[tiles_x*tiles_y*8];
	if(bg_texcoords_array) delete [] bg_texcoords_array;
	bg_texcoords_array = new float[tiles_x*tiles_y*8];
	
	if(!texRectAvailable) {
		tex_size_x = 1.f;
		tex_size_y = 1.f;	
	}
	
	for(int x=0, pp = -10; x<tiles_x; x++, pp++) {
		for(int y=0; y<heightmap[x]; y++) {
			bg_vertex_array[ptr] = ts_x*pp;
			bg_vertex_array[ptr+1] = ts_y*y;
			bg_vertex_array[ptr+2] = ts_x*pp+ts_x;
			bg_vertex_array[ptr+3] = ts_y*y;	
			bg_vertex_array[ptr+4] = ts_x*pp+ts_x;
			bg_vertex_array[ptr+5] = ts_y*y+ts_y;
			bg_vertex_array[ptr+6] = ts_x*pp;
			bg_vertex_array[ptr+7] = ts_y*y+ts_y;
			ptr += 8;
		}
	}
	bg_vbo_ptr = ptr;
	ptr = 0;
	
	for(int x=0; x<tiles_x; x++) {
		for(int y=0; y<heightmap[x]; y++) {
			bg_texcoords_array[ptr] = 0.f;
			bg_texcoords_array[ptr+1] = 0.f;
			bg_texcoords_array[ptr+2] = tex_size_x;
			bg_texcoords_array[ptr+3] = 0.f;
			bg_texcoords_array[ptr+4] = tex_size_x;
			bg_texcoords_array[ptr+5] = tex_size_y;
			bg_texcoords_array[ptr+6] = 0.f;
			bg_texcoords_array[ptr+7] = tex_size_y;
			ptr += 8;
		}
	}
	
	if(vboAvailable) {
		fillBackgroundVBO();
	}
}

void renderer::drawBackground(vector2 displacement, aabb2 view_bb) {
	glColor4f(0.50f, 0.50f, 0.50f, 1.f);
	glPushMatrix();
	glTranslatef(displacement[0], displacement[1], 0.f);
	drawBackground(view_bb.bottom_left[0], view_bb.top_right[0]);
	glPopMatrix();
}

void renderer::drawText(char* string, int len, bool font, float size, color clr, vector2 position_orig) {
	assert(string != NULL && size > 0.f);	
	
	vector2 position = position_orig;
	
	int currentChar;
	float xpos, ypos;
	int charx, chary;
	int ptr = txt_vbo_ptr;
	int clrptr = txt_clr_ptr;
	
	for(int x=0; x<len; x++) {
		currentChar = int(string[x]);
		if(currentChar == 10) {
			position = vector2(position_orig[0], position[1]-size);
			continue;
		}
		currentChar -= 32;
		
		charx = currentChar%16;
		chary = currentChar/16;
		
		if(font) {
			charx = 15 - charx;
			chary = 15 - chary;
		} else {
			charx = 15 - charx;
			chary = 7 - chary;	
		}
		
		xpos = FONT_SIZE*charx;
		ypos = FONT_SIZE*chary;
		
		for(int i=0; i<4; i++)
		for(int x=0; x<4; x++) {
			txt_colors_array[clrptr] = clr[x];
			clrptr++;
		}
	
		if(texRectAvailable) {	
			txt_vertex_array[ptr] = position[0];
			txt_texcoords_array[ptr] = xpos+FONT_SIZE;
			txt_vertex_array[ptr+1] = position[1];
			txt_texcoords_array[ptr+1] = ypos;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0]+size;
			txt_texcoords_array[ptr] = xpos;
			txt_vertex_array[ptr+1] = position[1];
			txt_texcoords_array[ptr+1] = ypos;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0]+size;
			txt_texcoords_array[ptr] = xpos;
			txt_vertex_array[ptr+1] = position[1]+size;
			txt_texcoords_array[ptr+1] = ypos+FONT_SIZE;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0];
			txt_texcoords_array[ptr] = xpos+FONT_SIZE;
			txt_vertex_array[ptr+1] = position[1]+size;
			txt_texcoords_array[ptr+1] = ypos+FONT_SIZE;
			ptr += 2;	
			position[0] += size-size/2;
		} else {
			txt_vertex_array[ptr] = position[0];
			txt_texcoords_array[ptr] = (xpos+FONT_SIZE)/FONT_TEXTURE_SIZE;
			txt_vertex_array[ptr+1] = position[1];
			txt_texcoords_array[ptr+1] = ypos/FONT_TEXTURE_SIZE;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0]+size;
			txt_texcoords_array[ptr] = xpos/FONT_TEXTURE_SIZE;
			txt_vertex_array[ptr+1] = position[1];
			txt_texcoords_array[ptr+1] = ypos/FONT_TEXTURE_SIZE;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0]+size;
			txt_texcoords_array[ptr] = xpos/FONT_TEXTURE_SIZE;
			txt_vertex_array[ptr+1] = position[1]+size;
			txt_texcoords_array[ptr+1] = (ypos+FONT_SIZE)/FONT_TEXTURE_SIZE;
			ptr += 2;
			
			txt_vertex_array[ptr] = position[0];
			txt_texcoords_array[ptr] = (xpos+FONT_SIZE)/FONT_TEXTURE_SIZE;
			txt_vertex_array[ptr+1] = position[1]+size;
			txt_texcoords_array[ptr+1] = (ypos+FONT_SIZE)/FONT_TEXTURE_SIZE;
			ptr += 2;	
			position[0] += size-size/2;
		}
	}
	
	txt_vbo_ptr = ptr;
	txt_clr_ptr = clrptr;
}

void renderer::drawLine(vector2 a, vector2 b, color clr_a, color clr_b, bool glowing) {
	int iterations;
	float alphaStep;
	if(glowing) {
		iterations = GLOW_ITERATIONS;
		alphaStep = 1.f/iterations;
	} else {
		iterations = 1;
		alphaStep = 0.f;	
	}
	float alpha = 1.f;
	for(int x=0; x<iterations; x++) {
		alpha -= alphaStep;
		glBegin(GL_LINES);
		glColor4f(clr_a[0], clr_a[1], clr_a[2], alpha);
		glVertex2f(a[0], a[1]+(float)x*0.5f);
		glColor4f(clr_b[0], clr_b[1], clr_b[2], alpha);
		glVertex2f(b[0], b[1]+(float)x*0.5f);
		glEnd();
	}
}

void renderer::drawFilledCircle(vector2 center, float radius, color clr) {
 	glColor4f(clr[0], clr[1], clr[2], clr[3]);
 	glPushMatrix();
 	glLoadIdentity();
 	glTranslatef(center[0], center[1], 0.f);
	glBegin(GL_TRIANGLE_FAN);
	for(int x=0; x<=360; x+=5)
 		glVertex2f(cos(deg2rad(x))*radius,sin(deg2rad(x))*radius);
 	glEnd();
	glPopMatrix();
}

void renderer::drawCircle(vector2 center, float radius, int segments) {
 	glColor4f(0.7f, 0.7f, 0.7f, 0.3f);
 	glPushMatrix();
 	glLoadIdentity();
 	
 	float theta = 2 * N_PI / float(segments);
 	float tan_factor = tanf(theta);
 	float rad_factor = cosf(theta);
 	float x = radius;
 	float y = 0 ;
 	glBegin(GL_LINE_LOOP);	
 	for(int i=0; i<segments; i++) {
		glVertex2f(x + center[0], y + center[1]);
		float tx = -y;
		float ty = x;
		x += tx * tan_factor;
		y += ty * tan_factor;
		x *= rad_factor;
		y *= rad_factor;	
	}
	glEnd();
	glPopMatrix();
}

void renderer::drawBox(vector2 bottom_left, vector2 top_right) {
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_LINE_LOOP);
	glColor4f(1.f, 0.f, 0.f, 1.f);
	glVertex2f(bottom_left[0], bottom_left[1]);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex2f(top_right[0], bottom_left[1]);
	glColor4f(0.f, 0.f, 1.f, 1.f);
	glVertex2f(top_right[0], top_right[1]);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex2f(bottom_left[0], top_right[1]);
	glEnd();
	glPopMatrix();
}

void renderer::drawTriangle(DrawPrimitive* x) {
	if(x->type != PRIM_TYPE_TRIANGLE) return;
	glBegin(GL_TRIANGLES);
	for(int i=0;i<3;i++) {
		glColor4f(x->vertices_colors[i][0], x->vertices_colors[i][1], x->vertices_colors[i][2], x->vertices_colors[i][3]);
		glVertex2f(x->vertices[i][0], x->vertices[i][1]);
	}
	glEnd();
}

void renderer::drawQuad(DrawPrimitive* x) {
	if(x->type != PRIM_TYPE_QUAD) return;
	glBegin(GL_QUADS);
	for(int i=0;i<4;i++) {
		glColor4f(x->vertices_colors[i][0], x->vertices_colors[i][1], x->vertices_colors[i][2], x->vertices_colors[i][3]);
		glVertex2f(x->vertices[i][0], x->vertices[i][1]);
	}
	glEnd();
}

void renderer::drawTexturedQuad(DrawPrimitive* x, unsigned int texture) {
	glColor4f(1.0f, 1.0f, 1.0f, x->vertices_colors[0][3]);
	if(texRectAvailable) {
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);			
		glVertex2f(x->vertices[0][0], x->vertices[0][1]);
		glTexCoord2f(x->size[0], 0.f);	
		glVertex2f(x->vertices[1][0], x->vertices[1][1]);
		glTexCoord2f(x->size[0], x->size[1]);	
	 	glVertex2f(x->vertices[2][0], x->vertices[2][1]);
		glTexCoord2f(0, x->size[1]);		
		glVertex2f(x->vertices[3][0], x->vertices[3][1]);
		glEnd();
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
	} else {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
	
		float sprite_size_x = x->vertices[1][0] - x->vertices[0][0];
		float sprite_size_y = x->vertices[2][1] - x->vertices[1][1];
		float texcoord_x = x->size[0] / x->tex_size[0];
		float texcoord_y = x->size[1] / x->tex_size[1];
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);			
		glVertex2f(x->vertices[0][0], x->vertices[0][1]);
		glTexCoord2f(texcoord_x, 0.f);	
		glVertex2f(x->vertices[1][0], x->vertices[1][1]);
		glTexCoord2f(texcoord_x, texcoord_y);	
	 	glVertex2f(x->vertices[2][0], x->vertices[2][1]);
		glTexCoord2f(0.f, texcoord_y);		
		glVertex2f(x->vertices[3][0], x->vertices[3][1]);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}

unsigned char* renderer::RGBtoRGBAdata(char* data, vector2 size) {
	// converts RGB raw data to RGBA raw data and sets transparency according to the specified colorkey
	int RGBsize = (int)(size[0]*size[1]*3);
	int RGBAsize = (int)(size[0]*size[1]*4);
	
	RGBA_tmp = new unsigned char[RGBAsize];
	for(int x=0,y=0; x<RGBsize; x+=3,y+=4) {
		if((unsigned char)data[x] == COLOR_KEY_R &&
		   (unsigned char)data[x+1] == COLOR_KEY_G &&
		   (unsigned char)data[x+2] == COLOR_KEY_B) {
			RGBA_tmp[y+3] = 0; 
			data[x] = 0;
			data[x+1] = 0; 
			data[x+2] = 0; 
		} else {
			RGBA_tmp[y+3] = 255;	
		}
		RGBA_tmp[y+2] = data[x];
		RGBA_tmp[y+1] = data[x+1];
		RGBA_tmp[y] = data[x+2];
	}
	return RGBA_tmp;
}

void renderer::freeRGBAdata() {
	if(RGBA_tmp) delete RGBA_tmp;	
}

void renderer::findPrimitiveBoundingCircle(Primitive* x) {
	if((*x).type == PRIM_TYPE_TRIANGLE) {
		// we use only right triangles, so the center of the circumcircle is the midpoint of the hypotenuse
		float abLength2 = ((*x).vertices[0] - (*x).vertices[1]).length2();
		float bcLength2 = ((*x).vertices[1] - (*x).vertices[2]).length2();
		float acLength2 = ((*x).vertices[0] - (*x).vertices[2]).length2();
		if(abLength2 > bcLength2 && abLength2 > acLength2)
			(*x).center = ((*x).vertices[0] + (*x).vertices[1]) * 0.5f;
		else if(bcLength2 > abLength2 && bcLength2 > acLength2)
			(*x).center = ((*x).vertices[1] + (*x).vertices[2]) * 0.5f;
		else (*x).center = ((*x).vertices[0] + (*x).vertices[2]) * 0.5f;
		(*x).radius = ((*x).center - (*x).vertices[0]).length();
		return;	
	}
	if((*x).type == PRIM_TYPE_QUAD) {
		(*x).center = ((*x).vertices[0] + (*x).vertices[2]) * 0.5f;
		vector2 quad_radius((*x).vertices[0]);
		quad_radius -= (*x).center;
		(*x).radius = quad_radius.length();
		return;
	}
}

void renderer::findPrimitiveBoundingCircle(DrawPrimitive* x) {
	if((*x).type == PRIM_TYPE_TRIANGLE) {
		// we use only right triangles, so the center of the circumcircle is the midpoint of the hypotenuse
		float abLength2 = ((*x).vertices[0] - (*x).vertices[1]).length2();
		float bcLength2 = ((*x).vertices[1] - (*x).vertices[2]).length2();
		float acLength2 = ((*x).vertices[0] - (*x).vertices[2]).length2();
		if(abLength2 > bcLength2 && abLength2 > acLength2)
			(*x).center = ((*x).vertices[0] + (*x).vertices[1]) * 0.5f;
		else if(bcLength2 > abLength2 && bcLength2 > acLength2)
			(*x).center = ((*x).vertices[1] + (*x).vertices[2]) * 0.5f;
		else (*x).center = ((*x).vertices[0] + (*x).vertices[2]) * 0.5f;
		(*x).radius = ((*x).center - (*x).vertices[0]).length();
		return;	
	}
	if((*x).type == PRIM_TYPE_QUAD) {
		(*x).center = ((*x).vertices[0] + (*x).vertices[2]) * 0.5f;
		vector2 quad_radius((*x).vertices[0]);
		quad_radius -= (*x).center;
		(*x).radius = quad_radius.length();
		return;
	}
	if((*x).type == PRIM_TYPE_CIRCLE) {
		(*x).center = (*x).vertices[0];	
	}
}

void renderer::loadFontTexture() {
	GLFWimage fontImage;
	if(!glfwReadImage(FONT_PATH, &fontImage, GLFW_ORIGIN_UL_BIT)) {
		cout << "Error: could not open font" << endl;
		exit(-1);
	}

	font_texture = loadTextureRGB((char*)fontImage.Data, vector2(fontImage.Width, fontImage.Height));
	assert(fontImage.BytesPerPixel == 3);
	glfwFreeImage(&fontImage);
}


void renderer::fillBackgroundVBO() {
	glEnable(GL_ARRAY_BUFFER);
	C_glBindBuffer(GL_ARRAY_BUFFER, bg_v_vbo);
	C_glBufferData(GL_ARRAY_BUFFER, bg_vbo_ptr*sizeof(float), bg_vertex_array, GL_STATIC_DRAW);
	C_glBindBuffer(GL_ARRAY_BUFFER, 0);
	C_glBindBuffer(GL_ARRAY_BUFFER, bg_t_vbo);
	C_glBufferData(GL_ARRAY_BUFFER, bg_vbo_ptr*sizeof(float), bg_texcoords_array, GL_STATIC_DRAW);
	C_glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_ARRAY_BUFFER);
	delete [] bg_vertex_array;
	delete [] bg_texcoords_array;
	bg_vertex_array = NULL;
	bg_texcoords_array = NULL;
}

void renderer::drawTextArray() {
	if(texRectAvailable) {
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, font_texture);
	} else {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, font_texture);
	}
	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	if(vboAvailable) {
		glEnable(GL_ARRAY_BUFFER);
		C_glBindBuffer(GL_ARRAY_BUFFER, txt_v_vbo);
		C_glBufferData(GL_ARRAY_BUFFER, txt_vbo_ptr*sizeof(float), txt_vertex_array, GL_STREAM_DRAW);
		glVertexPointer(2, GL_FLOAT, 0, NULL);
		C_glBindBuffer(GL_ARRAY_BUFFER, txt_t_vbo);
		C_glBufferData(GL_ARRAY_BUFFER, txt_vbo_ptr*sizeof(float), txt_texcoords_array, GL_STREAM_DRAW);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		C_glBindBuffer(GL_ARRAY_BUFFER, txt_c_vbo);
		C_glBufferData(GL_ARRAY_BUFFER, txt_clr_ptr*sizeof(float), txt_colors_array, GL_STREAM_DRAW);
		glColorPointer(4, GL_FLOAT, 0, NULL);
		glDrawArrays(GL_QUADS, 0, txt_vbo_ptr/2);
		C_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_ARRAY_BUFFER);
	} else {
		glVertexPointer(2, GL_FLOAT, 0, txt_vertex_array);
		glTexCoordPointer(2, GL_FLOAT, 0, txt_texcoords_array);
		glColorPointer(4, GL_FLOAT, 0, txt_colors_array);
		glDrawArrays(GL_QUADS, 0, txt_vbo_ptr/2);
	}
	
	glDisable(GL_ARRAY_BUFFER);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	if(texRectAvailable) {
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);	
	}
	
	delete [] txt_vertex_array;
	delete [] txt_texcoords_array;
	delete [] txt_colors_array;
}

void renderer::drawBackground(float left_bound_x, float right_bound_x) {
	if(texRectAvailable) {
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, bg_tex);
	} else {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, bg_tex);
	}
	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	
	int startSprite = 0;
	int numColumnsStart = int(left_bound_x/ts_x);

	for(int x=0; x<numColumnsStart; x++) {
		startSprite += heightmap[x];	
	}

	int offset = startSprite * 32;

	int numSprites = 0;
	int numColumnsEnd = int(right_bound_x/ts_x);
	for(int x=numColumnsStart; x<=numColumnsEnd; x++) {
		numSprites += heightmap[x];
	}
	
	if(vboAvailable) {
		glEnable(GL_ARRAY_BUFFER);
		C_glBindBuffer(GL_ARRAY_BUFFER, bg_v_vbo);
		glVertexPointer(2, GL_FLOAT, 0, GL_OFFSET(offset));
		C_glBindBuffer(GL_ARRAY_BUFFER, bg_t_vbo);
		glTexCoordPointer(2, GL_FLOAT, 0, GL_OFFSET(offset));
		glDrawArrays(GL_QUADS, 0, numSprites*8);
		C_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_ARRAY_BUFFER);
	} else {
		glVertexPointer(2, GL_FLOAT, 0, bg_vertex_array + offset);
		glTexCoordPointer(2, GL_FLOAT, 0, bg_texcoords_array + offset);
		glDrawArrays(GL_QUADS, 0, numSprites*8);
	}
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(texRectAvailable) {
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);	
	}
}	

void renderer::drawLevel(float* vertex_list, int vl_ptr){ 
//	if(lvl_vertex_array) delete [] lvl_vertex_array;
	if(vboAvailable) 
		drawLevelVBO(vertex_list, vl_ptr);	
	else {
		drawLevelArray(vertex_list, vl_ptr);
	}
}

void renderer::drawLevelArray(float* vertex_list, int vl_ptr){
	glColor4f(TILE_COLOR);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertex_list);
	glDrawArrays(GL_TRIANGLES, 0, vl_ptr/2);
	glDisableClientState(GL_VERTEX_ARRAY);
	lvl_vbo_ptr = vl_ptr;
	lvl_vertex_array = vertex_list;
}

void renderer::drawLevelVBO(float* vertex_list, int vl_ptr) {
	glColor4f(TILE_COLOR);
	if(vl_ptr > 0) {
		glEnable(GL_ARRAY_BUFFER);
		C_glBindBuffer(GL_ARRAY_BUFFER, level_tri_vbo);
		C_glBufferData(GL_ARRAY_BUFFER, vl_ptr*sizeof(float), vertex_list, GL_STREAM_DRAW);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, vl_ptr/2);
		glDisableClientState(GL_VERTEX_ARRAY);
		C_glBindBuffer(GL_ARRAY_BUFFER, 0);
		lvl_vbo_ptr = vl_ptr;
		glDisable(GL_ARRAY_BUFFER);
	}
}

void renderer::redrawLevel() {
	glColor4f(TILE_COLOR);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	if(vboAvailable) {
		C_glBindBuffer(GL_ARRAY_BUFFER, level_tri_vbo);
		glVertexPointer(2, GL_FLOAT, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, lvl_vbo_ptr/2);
		C_glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		glVertexPointer(2, GL_FLOAT, 0, lvl_vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, lvl_vbo_ptr/2);
	}
	
	glDisableClientState(GL_VERTEX_ARRAY);
}


unsigned int* renderer::loadTexturesFromFile(char* filename, int* frames_ret, int* size_x_ret, int* size_y_ret, int* real_size_x_ret, int* real_size_y_ret) {
	ifstream animFile(filename, ios::binary);
	assert(animFile.is_open());
	RGB_RAW_ANIMATION_FILE fileAttr;
	animFile.read((char*)(&fileAttr), sizeof(fileAttr));
	vector2 size(fileAttr.size_x, fileAttr.size_y);
	*size_x_ret = (int)size[0]; *size_y_ret = (int)size[1];
	*real_size_x_ret = fileAttr.real_size_x; *real_size_y_ret = fileAttr.real_size_y; 
	int frames_n = fileAttr.frames_n;
	*frames_ret = frames_n;
	tmp_textures = new unsigned int[frames_n];
	char* buffer = new char[int(size[0]*size[1]*3)];
	for(int x=0; x<frames_n; x++) {
		animFile.read(buffer, int(size[0]*size[1]*3));
		tmp_textures[x] = loadTextureRGB(buffer, size);	
	}
	delete buffer;
	return tmp_textures;
}

unsigned int renderer::loadTextureRGB(char* data, vector2 size) {
	unsigned int id;
	if(texRectAvailable) {
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, (int)size[0], (int)size[1]
					, 0, GL_RGBA, GL_UNSIGNED_BYTE, RGBtoRGBAdata(data, size));
		freeRGBAdata();
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
	} else {
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)size[0], (int)size[1]
					, 0, GL_RGBA, GL_UNSIGNED_BYTE, RGBtoRGBAdata(data, size));
		freeRGBAdata();
		glDisable(GL_TEXTURE_2D);	
	}
	return id;
}

void renderer::drawSprite(DrawPrimitive* quad, unsigned int texture) {
	drawTexturedQuad(quad, texture);	
}

vector2 renderer::getWindowParams() {
	return vector2(w_width, w_height);	
}

