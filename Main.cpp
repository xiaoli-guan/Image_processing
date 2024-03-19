
#include "BMP.h"
#include <iostream>
using namespace std;
int main() {
    BMP test;
    string ImageName = "Baboon.bmp";
    test.ReadImage(ImageName);
    BMP test2;
    // test.Resize(test2,{0,0},5,5);
    // test.WeightGray(test2,{0.8,0.1,0.1});
    // test.Inverse(test2);
    // test2.WriteImage("testLINEA.bmp");
    // test.WeightGray(test2);
    // test2.WriteImage("testLINEA.bmp");
    // test.Inverse(test2);
    test.RectCut(test2,{0,0},{512,512});
    // test.Resize(test2,{0,0},5,5,INTER_NEAREST);
    test2.WriteImage("testNEAR.bmp");
    return 0;
}