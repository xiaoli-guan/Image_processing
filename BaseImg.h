#ifndef BASEIMG_H
#define BASEIMG_H
#include<string>
#include<vector>
#include<iostream>


class BaseImg{
public:
    BaseImg();

    ~BaseImg();

    int GetHeight()const { return height; }

    int GetWidth()const { return width; }

    int GetBitDeep()const { return bitDeep; }

    void SetHeight(int h) { height = h; }

    void SetWidth(int w) { width = w; }

    virtual bool ReadImage(std::string ImageName) = 0;

    virtual bool WriteImage(std::string ImageName) = 0;
    /*
    *���ܣ�ͼ��ת
    *@param direction 1:ˮƽ��ת 2����ֱ��ת
    */
    void Flip(int direction);
protected:
    std::vector<std::vector<unsigned char> > pixels;
    int height; 
    int width;
    int bitDeep;
};


#endif