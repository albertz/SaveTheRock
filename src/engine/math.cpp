/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#include "engine/headers.h"

// vector2 member functions

vector2::vector2() {
	xy[0] = 0.f;
	xy[1] = 0.f;
	lengthCached = false;
}

vector2::vector2(float x, float y) {
	xy[0] = x;
	xy[1] = y;
	lengthCached = false;	
}

void vector2::set(float x, float y) {
	xy[0] = x;
	xy[1] = y;
	lengthCached = false;
}

float& vector2::operator[](const int index) {
	lengthCached = false;
	return xy[index];
}

const float& vector2::operator[](const int index) const {
	return xy[index];	
}

void vector2::negate() {
	lengthCached = false;
	xy[0] *= -1.f;
	xy[1] *= -1.f;
}

vector2& vector2::operator=(const vector2& v) {
	if(this == &v) return *this;
	lengthCached = false;
	xy[0] = v[0];
	xy[1] = v[1];
	return *this;
}
bool vector2::operator==(const vector2& v) {
	if(xy[0] == v[0] && xy[1] == v[1]) return true;
	return false;	
}

bool vector2::operator!=(const vector2& v) {
	return !(*this == v);
}

vector2& vector2::operator+=(const vector2& v) {
	lengthCached = false;
	xy[0] += v[0];
	xy[1] += v[1];
	return *this;
}

vector2& vector2::operator-=(const vector2& v) {
	lengthCached = false;
	vector2 tmp(v);
	tmp.negate();
	*this += tmp;
	return *this;
}

vector2& vector2::operator*=(const float& x) {
	lengthCached = false;
	xy[0] *= x;
	xy[1] *= x;
	return *this;
}

vector2 vector2::operator+(const vector2& v) {
	return vector2(*this) += v;
}

vector2 vector2::operator-(const vector2& v) {
	return vector2(*this) -= v;
}

vector2 vector2::operator*(const float& x) {
	return vector2(*this) *= x;
}

vector2 vector2::cross_product(const float& x) {
	return vector2(x*xy[1], -x*xy[0]);	
}

float vector2::cross_product(const vector2& v) {
	return (xy[0] * v[1] - xy[1] * v[0]);	
}

float vector2::dot_product(const vector2& v) {
	return (xy[0] * v[0] + xy[1] * v[1]);
}

float vector2::length() {
	if(!lengthCached) {
		C_length2 = xy[0] * xy[0] + xy[1] * xy[1];
		C_length = std::sqrt(C_length2);
		lengthCached = true;
	}
	return C_length;
}

float vector2::length2() {
	if(!lengthCached) {
		C_length2 = xy[0] * xy[0] + xy[1] * xy[1];
	}
	return C_length2;
}

void vector2::normalize() {
	float len = this->length();
	xy[0] /= len;
	xy[1] /= len;
	lengthCached = true;
	C_length2 = C_length = 1.f;
}
/*
// vector3 member functions

vector3::vector3() {
	for(int a=0;a<3;a++)
		xyz[a] = 0.f;
}

vector3::vector3(float a, float b, float c) {
	xyz[0] = a;
	xyz[1] = b;
	xyz[2] = c;	
}

vector3::vector3(vector2& v) {
	xyz[0] = v[0];
	xyz[1] = v[1];
	xyz[2] = 0.f;	
}

float& vector3::operator[](const int index) {
	return xyz[index];
}

const float& vector3::operator[](const int index) const {
	return xyz[index];	
}

void vector3::negate() {
	for(int a=0;a<3;a++)
		xyz[a] = xyz[a] * -1.f;
}

vector3& vector3::operator+=(const vector3& v) {
	for(int a=0;a<3;a++)
		xyz[a] += v[a];
	return *this;
}

vector3& vector3::operator-=(const vector3& v) {
	vector3 tmp(v);
	tmp.negate();
	*this += tmp;
	return *this;
}

vector3& vector3::operator*=(const float& x) {
	for(int a=0;a<3;a++)
		xyz[a] *= x;
	return *this;
}

vector3 vector3::operator+(const vector3& v) {
	return vector3(*this) += v;
}

vector3 vector3::operator-(const vector3& v) {
	return vector3(*this) -= v;
}

vector3 vector3::operator*(const float& x) {
	return vector3(*this) *= x;
}

float vector3::dot_product(const vector3& v) {
	return xyz[0] * v[0] + xyz[1] * v[1] + xyz[2] * v[2]; 
}

vector3 vector3::cross_product(const vector3& v) {
	return vector3(xyz[1]*v[2] - xyz[2]*v[1], xyz[2]*v[0] - xyz[0]*v[2], xyz[0]*v[1] - xyz[1]*v[0]);
}

float vector3::length() {
	return std::sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
}

float vector3::length2() {
	return (xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
}

void vector3::normalize() {
	float len = this->length();
	for(int x=0;x<3;x++)
		xyz[x] = xyz[x] / len;
}*/

// color member functions

color::color() {
	for(int a=0;a<4;a++)
		rgba[a] = 0.f;
}

color::color(float r, float g, float b, float alpha) {
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = alpha;	
}

void color::set(float r, float g, float b, float alpha) {
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = alpha;	
}

float& color::operator[](const int index) {
	return rgba[index];
}

const float& color::operator[](const int index) const {
	return rgba[index];	
}

void color::negate() { 
	for(int a=0;a<4;a++) // negates all components (makes sense only when subtracting colors)
		rgba[a] = -1 * rgba[a];
}

color& color::operator+=(const color& v) {
	for(int a=0;a<3;a++)
		rgba[a] += v[a];
	return *this;
}

color& color::operator-=(const color& v) {
	color tmp(v);
	tmp.negate();
	*this += tmp;
	return *this;
}

color& color::operator*=(const float& x) {
	for(int a=0;a<4;a++)
		rgba[a] *= x;
	return *this;
}

color color::operator+(const color& v) {
	return color(*this) += v;
}

color color::operator-(const color& v) {
	return color(*this) -= v;
}

color color::operator*(const float& x) {
	return color(*this) *= x;
}
