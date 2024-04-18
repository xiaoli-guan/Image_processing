
#include "BMP.h"
#include <iostream>
using namespace std;
int main() {
    BMP test;
    string ImageName = "Lena512x512.bmp";
    test.ReadImage(ImageName);
    BMP test2;
    BMP test3;
    BMP test4;
    test.WeightGray(test2);
    test2.GaussianBlur(test3,{3,3},1.2);
    test3.WriteImage("GrayDst.bmp");
    test3.Sobel(test4,1,1);
    test4.WriteImage("GraySobelDst.bmp");
    test3.Sobel(test4,1,0);
    test4.WriteImage("GraySobelDst1.bmp");
    test3.Sobel(test4,0,1);
    test4.WriteImage("GraySobelDst2.bmp");
    // test.MeanBlur(test2,{3,3});
    // test.MadianBlur(test3,{3,3});
    // test2.WriteImage("meandst.bmp");
    // test3.WriteImage("mediandst.bmp");
    // test.Resize(test2,{0,0},5,5);
    // test.WeightGray(test2,{0.8,0.1,0.1});
    // test.Inverse(test2);
    // test2.WriteImage("testLINEA.bmp");
    // test.WeightGray(test2);
    // test2.WriteImage("testLINEA.bmp");
    // test.Inverse(test2);
    // test.RectCut(test2,{0,0},{512,512});
    // test.Resize(test2,{0,0},5,5,INTER_NEAREST);
    // test2.WriteImage("testNEAR.bmp");
    return 0;
}