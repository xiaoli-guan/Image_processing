#ifndef BMP_H
#define BMP_H
#include<string>
#include<vector>
#include<cmath>
#include<iostream>
// 针对该结构体的字节对齐问题调整对齐策略
#pragma pack(push,1)

//bmp文件头，14字节
struct BMPFILEHEADER
{
    unsigned short bfType;              // 位图文件的类型，必须为BMP (2个字节)
    unsigned int   bfSize;              // 位图文件的大小，以字节为单位 (4个字节)
    unsigned short bfReserved1;         // 位图文件保留字，必须为0 (2个字节)
    unsigned short bfReserved2;         // 位图文件保留字，必须为0 (2个字节)
    unsigned int   bfOffBits;           // 位图数据的起始位置，以相对于位图 (4个字节)
};
#pragma pack(pop)

//bmp信息头，40字节
struct BITMAPINFOHEADER
{
    unsigned long    biSize;            //本结构所占用字节数 40字节
    long             biWidth;           //位图的宽度，以像素为单位
    long             biHeight;          //位图的高度，以像素为单位
    unsigned short   biPlanes;          //目标设备的级别，必须为1
    unsigned short   biBitCount;        //每个像素所需的位数，必须是1（双色）、
                                        //4（16色）、8（256色）、16（65536色）、24（真彩色）或32（增强型真彩色）之一
    unsigned long    biCompression;     //位图压缩类型，必须是 0（BI_RGB不压缩）、
                                        //1（BI_RLE8压缩类型）
                                        //2（BI_RLE压缩类型）之一
    unsigned long    biSizeImage;       //位图的大小，以字节为单位
    long             biXPelsPerMeter;   //位图水平分辨率，每米像素数
    long             biYPelsPerMeter;   //位图垂直分辨率，每米像素数
    unsigned long    biClrUsed;         //位图实际使用的颜色表中的颜色数
    unsigned long    biClrImportant;    //位图显示过程中重要的颜色数
};

//bmp调色盘，2、4、8位深的bmp需要,大小为（2^biBitCount）*4字节
struct RGBQUAD {
    unsigned char rgbBlue;              //蓝
    unsigned char rgbGreen;             //绿
    unsigned char rgbRed;               //红
    unsigned char rgbReserved;          //保留字
};

// 像素数据结构
struct PIXELS {
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char A; //alpha通道，32bit时用到;当biBitCount = 1、4、8时的颜色索引也存在这里

    // 重载运算符
    PIXELS operator = (float fn){
        return {B = G = R = (unsigned char)fn};
    }
};

// 重载运算符
PIXELS operator - (unsigned char num , PIXELS& p);


enum Interpolation {INTER_NEAREST = 1,INTER_LINEAR};
class BMP{
public:
    BMP();

    BMP(int height,int width,int BitCount);

    ~BMP();

    // 读取BMP图像
    virtual bool ReadImage(std::string ImageName);

    // 写入BMP图像
    virtual bool WriteImage(std::string ImageName);

    // 返回图像Size
    std::pair<int,int> GetSize(){return {height,width};}

    // 图像翻转，FlipCode：0 --- 垂直方向翻转；  1----- 水平方向翻转；  -1 ------- 水平、垂直方向同时翻转
    bool Flip(int FlipCode);

    // 调整图像大小。Dst:调整后的图像，Size(height,width)：调整后的尺寸，Fy(Fx):沿垂直(水平)轴的缩放因子，默认为0,表示不进行水平缩放。Size和Fx(Fy)不能同时为0
    // Interpolation：插值方法，默认用双线性插值。INTER_NEAREST----- 最近邻插值，速度最快，但质量最差，INTER_LINEAR------双线性插值，速度较快，质量较好
    bool Resize(BMP& Dst, std::pair<int,int> Size,double Fy = 0,double Fx = 0,Interpolation Interpolation = INTER_LINEAR);

    // 图像灰度化.dst:灰度化后的图像。weights: bgr的权重，默认各为0.333
    void WeightGray(BMP& dst,std::vector<float> weights = {0.333,0.333,0.333});

    // 图像取反
    void Inverse(BMP& dst);

    // 图像矩形裁剪，dst:裁剪后的图像。ul<x,y>是左上点的坐标，lr<x,y>是右下点的坐标,以图像左上角为原点,水平向右为x正轴，垂直向下为y正轴
    void RectCut(BMP& dst,std::pair<int,int> ul,std::pair<int,int> lr);
private:

    // 设置图像的参数
    void SetSize(int height,int width,int bitCount);

    //读取bmp图像时用于初始化pixels
    bool PixelsDeserialization(std::vector<std::vector<unsigned char> > data);

    //写入bmp图像时用于序列化像素信息
    bool PixelsSerialization(std::vector<std::vector<unsigned char> >& data);

    // 最近邻插值
    void InterNearest(BMP& dst,std::pair<int,int> Size);

    // 双线性插值
    void InterLinear(BMP& dst, std::pair<int,int> size);
private:
    BMPFILEHEADER bmpfileheader;
    BITMAPINFOHEADER bitmapinfoheader;
    std::vector<RGBQUAD> rgbquad;
    std::vector<std::vector<PIXELS> > pixels;
    int height;
    int width;
    int biBitCount;
};

#endif
