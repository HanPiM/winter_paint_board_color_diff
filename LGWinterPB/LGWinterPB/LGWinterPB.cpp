// WinterB.cpp: 定义应用程序的入口点。
//
#include "WinterPB.h"
#include <thread>

using namespace std;
using namespace cimg_library;

int main()
{
	std::string name;
	std::cin >> name;
	auto a = CImg<unsigned char>(name.c_str());
	std::thread th([&a]() 
		{ a.display("original image", 0); }
	);

	size_t pos = name.find_last_of('.');
	name = name.substr(0, pos);

	std::thread th_e00([&a,&name]() 
		{ 
			clock_t start, end;
			start = clock();
			auto e00 = board_img(a).get_img();
			end = clock();
			printf("E00 time : %.2lf s\n", double(end - start) / CLOCKS_PER_SEC);
			e00.display("generated image(use CIE2000)", 0);
			e00.save((name + "_e.bmp").c_str());
		}
	);
	std::thread th_e76([&a,&name]()
		{
			clock_t start, end;
			start = clock();
			auto lab = board_img(a, board_img::delta_Lab).get_img();
			end = clock();
			printf("E76 time : %.2lf s\n", double(end - start) / CLOCKS_PER_SEC);
			lab.display("generated image(use CIE1976)", 0);
			lab.save((name + "_l.bmp").c_str());
		}
	);

	th_e00.join();
	th_e76.join();
	th.join();

	return 0;
}
