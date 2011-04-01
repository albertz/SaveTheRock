/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __RENDERER_H
#define __RENDERER_H

#include <string>

// opengl renderer

struct DrawPrimitive;
struct Primitive;
class aabb2;

enum QUADS_EXT {
	QUADS_NONE_EXT = 0,
	QUADS_NV_EXT = 1,
	QUADS_ATI_EXT = 2,
	QUADS_EXT_EXT = 3,
	QUADS_ARB_EXT = 4
};

struct RGB_RAW_ANIMATION_FILE {
	int size_x; // size of texture
	int size_y;
	int real_size_x; // size of sprite
	int real_size_y;
	int frames_n;
};

typedef void (APIENTRY * P_glGenBuffers) (GLsizei n, GLuint* buffers);
typedef void (APIENTRY * P_glBindBuffer) (GLenum target, GLuint buffer);
typedef void (APIENTRY * P_glBufferData) (GLenum target, GLsizei size, const void* data, GLenum usage);
typedef void (APIENTRY * P_glDeleteBuffers) (GLsizei n, const GLuint* ids);

class renderer {
	int w_width, w_height; bool fullscreen;
	
	bool vboAvailable;
	bool texRectAvailable;
	
	unsigned char* RGBA_tmp;
	unsigned char* RGBA_frame_tmp;
	unsigned int* tmp_textures;
	unsigned int font_texture;
	
	unsigned int txt_v_vbo;
	unsigned int txt_t_vbo;
	unsigned int txt_c_vbo;
	int txt_vbo_ptr;
	int txt_clr_ptr;
	float* txt_vertex_array;
	float* txt_texcoords_array;
	float* txt_colors_array;
	
	unsigned int level_tri_vbo; // level VBO (triangles)
	int lvl_vbo_ptr;
	float* lvl_vertex_array;
	
	unsigned int bg_tex;
	unsigned int bg_v_vbo; // background VBO (textured quads)
	unsigned int bg_t_vbo;
	int bg_vbo_ptr;
	int* heightmap;
	float ts_x, ts_y; // background tile size x and y
	float* bg_vertex_array;
	float* bg_texcoords_array;
	
	P_glGenBuffers C_glGenBuffers;
	P_glBindBuffer C_glBindBuffer;
	P_glBufferData C_glBufferData;
	P_glDeleteBuffers C_glDeleteBuffers;
	
	public:
	/* HELPER FUNCTIONS */
	float* generateBoxKernel();
	float* generateTentKernel(float scale);
	int* convolute(int* f, int f_size, float* g, int g_size);
	int* generateBackgroundHeightmap(int tiles_x, int tiles_y);
	void generateBackground(unsigned int texture, float tex_size_x, float tex_size_y);
	void drawBackground(vector2 displacement, aabb2 view_bb);
	void drawText(const std::string& string, int len, bool font, float size, color clr, vector2 position);
	void drawLine(vector2 a, vector2 b, color clr_a, color clr_b, bool glowing=false);
	void drawFilledCircle(vector2 center, float radius, color clr);
	void drawCircle(vector2 center, float radius, int segments=50);
	void drawBox(vector2 bottom_left, vector2 top_right);
	void drawTriangle(DrawPrimitive*);
	void drawQuad(DrawPrimitive*);
	void drawTexturedQuad(DrawPrimitive* x, unsigned int texture);
	unsigned char* RGBtoRGBAdata(char* data, vector2 size);
	void freeRGBAdata();
	void findPrimitiveBoundingCircle(DrawPrimitive* x);
	void findPrimitiveBoundingCircle(Primitive* x);
	
	void loadFontTexture();
	/********************/	
	
	renderer(int width, int height, bool fs, bool vsync, int fsaa);
	void setWindowTitle(const std::string& title);
	void frameStart(vector2 bg_displacement, aabb2 view_bb);
	void frameEnd();
	void generateVBO(unsigned int* id);
	void destroyVBO(unsigned int* id);
	void drawTextArray();
	void fillBackgroundVBO();
	void drawBackground(float left_bound_x, float right_bound_x);
	void drawLevel(float* vertex_list, int vl_ptr);
	void drawLevelArray(float* vertex_list, int vl_ptr);
	void drawLevelVBO(float* vertex_list, int vl_ptr);
	void redrawLevel();
	void drawSprite(DrawPrimitive* x, unsigned int texture);
	unsigned int* loadTexturesFromFile(const std::string& filename, int* frames_ret, int* size_x_ret, int* size_y_ret, int* real_size_x_ret, int* real_size_y_ret);
	unsigned int loadTextureRGB(char* data, vector2 size);
	vector2 getWindowParams();
};

#endif
