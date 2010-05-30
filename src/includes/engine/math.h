/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __MATH_H
#define __MATH_H

class vector2 {
	// 2d vector class
	float xy[2];

	// cache
	bool lengthCached;
	float C_length;
	float C_length2;
	
	public:
	// constructors
	vector2();
	vector2(float x, float y); // construct 2d vector from two
	void set(float x, float y); // sets vector to xy
	
	// operators
	float& operator[](const int); 
	const float& operator[](const int) const;
	void negate(); // negates all components
	vector2& operator=(const vector2&);
	bool operator==(const vector2&);
	bool operator!=(const vector2&);
	vector2& operator+=(const vector2&);
	vector2& operator-=(const vector2&);
	vector2& operator*=(const float&);
	vector2 operator+(const vector2& v);
	vector2 operator-(const vector2& v);
	vector2 operator*(const float&);
	vector2 cross_product(const float&);
	float cross_product(const vector2& v);
	float dot_product(const vector2& v);
	// misc functions
	float length();
	float length2(); // vector length to the power of two
	void normalize();
};
/*
class vector3 {
	float xyz[3];
	
	public:

	// constructors
	vector3();
	vector3(float, float, float); // construct 3d vector from three floats
	vector3(vector2&); // construct 3d vector from 2d (z component is 0)
	
	// operators
	float& operator[](const int); 
	const float& operator[](const int) const;
	void negate(); // negates all components
	vector3& operator+=(const vector3&);
	vector3& operator-=(const vector3&);
	vector3& operator*=(const float&);
	vector3 operator+(const vector3& v);
	vector3 operator-(const vector3& v);
	vector3 operator*(const float&);
	float dot_product(const vector3& v);
	vector3 cross_product(const vector3& v);

	// misc functions
	float length();
	float length2(); // vector length to the power of two
	void normalize();
};*/

class color {
	// RGB color class
	float rgba[4];
	
	public:

	// constructors
	color();
	color(float r, float g, float b, float alpha=255.f); // construct color from four floats
	void set(float r, float g, float b, float alpha=255.f);
	
	// operators
	float& operator[](const int); 
	const float& operator[](const int index) const;
	void negate(); // negates all components
	color& operator+=(const color&);
	color& operator-=(const color&);
	color& operator*=(const float&);
	color operator+(const color&);
	color operator-(const color&);
	color operator*(const float&);
};

class ray {
	public:
	vector2 start_pos;
	vector2 end_pos; 	
};

#endif
