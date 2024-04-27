
#include "BMP.h"
#include <iostream>
using namespace std;
int main() {
    BMP test;
    BMP test2;
    BMP test3;
    BMP test4;
    string ImageName = "Lena512x512.bmp";
    test.ReadImage(ImageName);

    test.GaussianBlur(test3,{3,3},1.2);
    test3.WriteImage("gaussDst.bmp");
    test3.Sharpen(test4);
    test4.WriteImage("sharpenDst.bmp");
    return 0;
}