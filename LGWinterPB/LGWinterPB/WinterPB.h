// WinterB.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

#include <stdlib.h>
#include <time.h>

#include "CImg/CImg.h"

struct _rgb
{
	unsigned char r, g, b;
	_rgb() :r(0), g(0), b(0) {}
	_rgb(unsigned char r, unsigned char g, unsigned char b) :
		r(r), g(g), b(b) {}
};
struct _xyz
{
	float x, y, z;
	_xyz(float x, float y, float z) :
		x(x), y(y), z(z) {}
	_xyz(_rgb val);
};
struct _lab
{
	float l, a, b;
	_lab() :l(0), a(0), b(0) {}
	_lab(float l, float a, float b) :
		l(l), a(a), b(b) {}
	_lab(_xyz x);
};

/*
* 32 色像素图
*/
class board_img
{
public:
	using efunc = std::function<double(const _lab&, const _lab&)>;

	board_img(const cimg_library::CImg<unsigned char>& x, efunc e = delta_E00);
	/* 以空白字符间隔，每段字符数应相等 */
	board_img(const std::string& map);
	cimg_library::CImg<unsigned char> get_img()const;
	int* operator [](size_t idx)
	{
		return data.data() + idx * w;
	}

	static double delta_E00(const _lab& col1, const _lab& col2);
	static double delta_Lab(const _lab& col1, const _lab& col2);

private:
	int w, h;
	std::vector<int> data;
	efunc get_e = delta_E00;
	int get_color_idx(const _lab& x)const;
};
