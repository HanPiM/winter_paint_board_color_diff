#include "WinterPB.h"

using namespace cimg_library;

constexpr float param_13 = 1.0f / 3.0f;
constexpr float param_16116 = 16.0f / 116.0f;
constexpr float Xn = 0.950456f;
constexpr float Yn = 1.0f;
constexpr float Zn = 1.088754f;

constexpr float gamma(float x)
{
	return (x > 0.04045) ? powf((x + 0.055f) / 1.055f, 2.4f) : (x / 12.92f);
}

_xyz::_xyz(_rgb val)
{
	float rr = gamma(val.r / 255.0),
		gg = gamma(val.g / 255.0),
		bb = gamma(val.b / 255.0);
	x = 0.4124564f * rr + 0.3575761f * gg + 0.1804375f * bb;
	y = 0.2126729f * rr + 0.7151522f * gg + 0.0721750f * bb;
	z = 0.0193339f * rr + 0.1191920f * gg + 0.9503041f * bb;
}
_lab::_lab(_xyz x)
{
	constexpr float v1 = 0.008856f, v2 = 7.787f;
	x.x /= Xn, x.y /= Yn, x.z /= Zn;
	float xx = x.x > v1 ? powf(x.x, param_13) : v2 * x.x + param_16116;
	float yy = x.y > v1 ? powf(x.y, param_13) : v2 * x.y + param_16116;
	float zz = x.z > v1 ? powf(x.z, param_13) : v2 * x.z + param_16116;
	l = 116.0f * yy - 16.0f;
	if (l < 0)l = 0;
	a = 500.0f * (xx - yy);
	b = 200.0f * (yy - zz);
}

static _rgb col_table[32] = {
	_rgb(0, 0, 0),
	_rgb(255, 255, 255),
	_rgb(170, 170, 170),
	_rgb(85, 85, 85),
	_rgb(254, 211, 199),
	_rgb(255, 196, 206),
	_rgb(250, 172, 142),
	_rgb(255, 139, 131),
	_rgb(244, 67, 54),
	_rgb(233, 30, 99),
	_rgb(226, 102, 158),
	_rgb(156, 39, 176),
	_rgb(103, 58, 183),
	_rgb(63, 81, 181),
	_rgb(0, 70, 112),
	_rgb(5, 113, 151),
	_rgb(33, 150, 243),
	_rgb(0, 188, 212),
	_rgb(59, 229, 219),
	_rgb(151, 253, 220),
	_rgb(22, 115, 0),
	_rgb(55, 169, 60),
	_rgb(137, 230, 66),
	_rgb(215, 255, 7),
	_rgb(255, 246, 209),
	_rgb(248, 203, 140),
	_rgb(255, 235, 59),
	_rgb(255, 193, 7),
	_rgb(255, 152, 0),
	_rgb(255, 87, 34),
	_rgb(184, 63, 39),
	_rgb(121, 85, 72)
};
_lab lcol_table[32];

constexpr int col_table_cnt = 32;
// 彩度计算
static double chroma(double a, double b)
{
	return pow(a * a + b * b, 0.5);
}
// 色调角计算
static double hue_angle(double a, double b)
{
	double h = 0;
	double hab = 0;

	h = (180 / M_PI) * atan(a == 0 ? 0 : b / a); // 有正有负

	if (a > 0 && b > 0)
		hab = h;
	else if (a < 0 && b>0)
		hab = 180 + h;
	else if (a < 0 && b < 0)
		hab = 180 + h;
	else // a > 0 && b < 0
		hab = 360 + h;
	return hab;
}
double board_img::delta_E00(const _lab& col1, const _lab& col2)
{
	double L1 = col1.l, a1 = col1.a, b1 = col1.b,
		L2 = col2.l, a2 = col2.a, b2 = col2.b;

	double E00 = 0;         // CIEDE2000色差E00
	double LL1 = 0, LL2 = 0, aa1 = 0, aa2 = 0, bb1 = 0, bb2 = 0;
	double delta_LL = 0, delta_CC = 0, delta_hh = 0, delta_HH = 0; // 第二步的四个量
	double kL = 0, kC = 0, kH = 0;
	double RT = 0;          // 旋转函数RT
	double G = 0;           // CIELab 颜色空间 a 轴的调整因子, 是彩度的函数
	double mean_Cab = 0;    //两个样品彩度的算术平均值
	//------------------------------------------
	kL = 1;
	kC = 1;
	kH = 1;
	//------------------------------------------
	mean_Cab = (chroma(a1, b1) + chroma(a2, b2)) / 2;
	double mean_Cab_pow7 = pow(mean_Cab, 7);
	G = 0.5 * (1 - pow(mean_Cab_pow7 / (mean_Cab_pow7 + pow(25, 7)), 0.5));

	LL1 = L1;
	aa1 = a1 * (1 + G);
	bb1 = b1;

	LL2 = L2;
	aa2 = a2 * (1 + G);
	bb2 = b2;

	double CC1 = chroma(aa1, bb1);
	double CC2 = chroma(aa2, bb2);
	double hh1 = hue_angle(aa1, bb1);
	double hh2 = hue_angle(aa2, bb2);

	delta_LL = LL1 - LL2;
	delta_CC = CC1 - CC2;
	delta_hh = hue_angle(aa1, bb1) - hue_angle(aa2, bb2);
	delta_HH = 2 * sin(M_PI * delta_hh / 360) * pow(CC1 * CC2, 0.5);

	// 三 计算公式中的加权函数 SL,SC,SH,T
	double mean_LL = (LL1 + LL2) / 2;
	double mean_CC = (CC1 + CC2) / 2;
	double mean_hh = (hh1 + hh2) / 2;

	double SL = 1 + 0.015 * pow(mean_LL - 50, 2) / pow(20 + pow(mean_LL - 50, 2), 0.5);
	double SC = 1 + 0.045 * mean_CC;
	double T = 1 - 0.17 * cos((mean_hh - 30) * M_PI / 180) + 0.24 * cos((2 * mean_hh) * M_PI / 180)
		+ 0.32 * cos((3 * mean_hh + 6) * M_PI / 180) - 0.2 * cos((4 * mean_hh - 63) * M_PI / 180);
	double SH = 1 + 0.015 * mean_CC * T;

	// 四 计算公式中的RT
	double mean_CC_pow7 = pow(mean_CC, 7);
	double RC = 2 * pow(mean_CC_pow7 / (mean_CC_pow7 + pow(25, 7)), 0.5);
	double delta_xita = 30 * exp(-pow((mean_hh - 275) / 25, 2)); // △θ 以°为单位
	RT = -sin((2 * delta_xita) * M_PI / 180) * RC;

	double L_item = delta_LL / (kL * SL);
	double C_item = delta_CC / (kC * SC);
	double H_item = delta_HH / (kH * SH);

	E00 = pow(L_item * L_item + C_item * C_item + H_item * H_item + RT * C_item * H_item, 0.5);

	return E00;
}
double board_img::delta_Lab(const _lab& e1, const _lab& e2)
{
	double dl = e1.l - e2.l, da = e1.a - e2.a, db = e1.b - e2.b;
	return pow(dl * dl + da * da + db * db, 0.5);
}

int board_img::get_color_idx(const _lab& x)const
{
	double minn = 1e9;
	int minnidx = 1, offest;
	for (int i = 0; i < col_table_cnt; ++i)
	{
		offest = get_e(lcol_table[i], x);
		if (offest < minn)
		{
			minn = offest, minnidx = i;
		}
	}
	return minnidx;
}

board_img::board_img(const cimg_library::CImg<unsigned char>& x, efunc e)
{
	w = x.width(), h = x.height();
	data.resize(w * h + 1);
	get_e = e;

	for (int i = 0; i < col_table_cnt; ++i)lcol_table[i] = _lab(col_table[i]);
	cimg_forX(x, i)
	{
		cimg_forY(x, j)
		{
			data[j * w + i] = get_color_idx(_lab(_rgb(
				x.atXY(i, j, 0), x.atXY(i, j, 1), x.atXY(i, j, 2)
			)));
		}
	}
}
board_img::board_img(const std::string& map)
{
	std::string x;
	std::istringstream iss(map);
	std::vector<int>tmp;
	w = h = 0;
	while (iss >> x)
	{
		for (auto ch : x)
			tmp.push_back(isdigit(ch) ? ch - '0' : ch - 'a' + 10);
		h++;
	}
	w = x.size();
	int maxx = std::max(w, h);
	int minn = std::min(w, h);
	int oldw = w, oldh = h;
	std::swap(w, h);
	tmp.resize(maxx * maxx + 1);
	data.resize(tmp.size());
	
	for (int i = 0; i < maxx; ++i)
	{
		for (int j = 0; j < maxx; ++j)
		{
			if (i >= minn && j >= minn)continue;
			data[j * w + i] = tmp[i * oldw + j];
		}
	}
}
cimg_library::CImg<unsigned char> board_img::get_img() const
{
	CImg<unsigned char> res(w, h, 1, 3);
	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			res.atXY(i, j, 0) = col_table[data[j * w + i]].r;
			res.atXY(i, j, 1) = col_table[data[j * w + i]].g;
			res.atXY(i, j, 2) = col_table[data[j * w + i]].b;
		}
	}
	return res;
}
