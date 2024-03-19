#ifndef BMP_H
#define BMP_H
#include<string>
#include<vector>
#include<cmath>
#include<iostream>
// ��Ըýṹ����ֽڶ�����������������
#pragma pack(push,1)

//bmp�ļ�ͷ��14�ֽ�
struct BMPFILEHEADER
{
    unsigned short bfType;              // λͼ�ļ������ͣ�����ΪBMP (2���ֽ�)
    unsigned int   bfSize;              // λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ (4���ֽ�)
    unsigned short bfReserved1;         // λͼ�ļ������֣�����Ϊ0 (2���ֽ�)
    unsigned short bfReserved2;         // λͼ�ļ������֣�����Ϊ0 (2���ֽ�)
    unsigned int   bfOffBits;           // λͼ���ݵ���ʼλ�ã��������λͼ (4���ֽ�)
};
#pragma pack(pop)

//bmp��Ϣͷ��40�ֽ�
struct BITMAPINFOHEADER
{
    unsigned long    biSize;            //���ṹ��ռ���ֽ��� 40�ֽ�
    long             biWidth;           //λͼ�Ŀ�ȣ�������Ϊ��λ
    long             biHeight;          //λͼ�ĸ߶ȣ�������Ϊ��λ
    unsigned short   biPlanes;          //Ŀ���豸�ļ��𣬱���Ϊ1
    unsigned short   biBitCount;        //ÿ�����������λ����������1��˫ɫ����
                                        //4��16ɫ����8��256ɫ����16��65536ɫ����24�����ɫ����32����ǿ�����ɫ��֮һ
    unsigned long    biCompression;     //λͼѹ�����ͣ������� 0��BI_RGB��ѹ������
                                        //1��BI_RLE8ѹ�����ͣ�
                                        //2��BI_RLEѹ�����ͣ�֮һ
    unsigned long    biSizeImage;       //λͼ�Ĵ�С�����ֽ�Ϊ��λ
    long             biXPelsPerMeter;   //λͼˮƽ�ֱ��ʣ�ÿ��������
    long             biYPelsPerMeter;   //λͼ��ֱ�ֱ��ʣ�ÿ��������
    unsigned long    biClrUsed;         //λͼʵ��ʹ�õ���ɫ���е���ɫ��
    unsigned long    biClrImportant;    //λͼ��ʾ��������Ҫ����ɫ��
};

//bmp��ɫ�̣�2��4��8λ���bmp��Ҫ,��СΪ��2^biBitCount��*4�ֽ�
struct RGBQUAD {
    unsigned char rgbBlue;              //��
    unsigned char rgbGreen;             //��
    unsigned char rgbRed;               //��
    unsigned char rgbReserved;          //������
};

// �������ݽṹ
struct PIXELS {
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char A; //alphaͨ����32bitʱ�õ�;��biBitCount = 1��4��8ʱ����ɫ����Ҳ��������

    // ���������
    PIXELS operator = (float fn){
        return {B = G = R = (unsigned char)fn};
    }
};

// ���������
PIXELS operator - (unsigned char num , PIXELS& p);


enum Interpolation {INTER_NEAREST = 1,INTER_LINEAR};
class BMP{
public:
    BMP();

    BMP(int height,int width,int BitCount);

    ~BMP();

    // ��ȡBMPͼ��
    virtual bool ReadImage(std::string ImageName);

    // д��BMPͼ��
    virtual bool WriteImage(std::string ImageName);

    // ����ͼ��Size
    std::pair<int,int> GetSize(){return {height,width};}

    // ͼ��ת��FlipCode��0 --- ��ֱ����ת��  1----- ˮƽ����ת��  -1 ------- ˮƽ����ֱ����ͬʱ��ת
    bool Flip(int FlipCode);

    // ����ͼ���С��Dst:�������ͼ��Size(height,width)��������ĳߴ磬Fy(Fx):�ش�ֱ(ˮƽ)����������ӣ�Ĭ��Ϊ0,��ʾ������ˮƽ���š�Size��Fx(Fy)����ͬʱΪ0
    // Interpolation����ֵ������Ĭ����˫���Բ�ֵ��INTER_NEAREST----- ����ڲ�ֵ���ٶ���죬��������INTER_LINEAR------˫���Բ�ֵ���ٶȽϿ죬�����Ϻ�
    bool Resize(BMP& Dst, std::pair<int,int> Size,double Fy = 0,double Fx = 0,Interpolation Interpolation = INTER_LINEAR);

    // ͼ��ҶȻ�.dst:�ҶȻ����ͼ��weights: bgr��Ȩ�أ�Ĭ�ϸ�Ϊ0.333
    void WeightGray(BMP& dst,std::vector<float> weights = {0.333,0.333,0.333});

    // ͼ��ȡ��
    void Inverse(BMP& dst);

    // ͼ����βü���dst:�ü����ͼ��ul<x,y>�����ϵ�����꣬lr<x,y>�����µ������,��ͼ�����Ͻ�Ϊԭ��,ˮƽ����Ϊx���ᣬ��ֱ����Ϊy����
    void RectCut(BMP& dst,std::pair<int,int> ul,std::pair<int,int> lr);
private:

    // ����ͼ��Ĳ���
    void SetSize(int height,int width,int bitCount);

    //��ȡbmpͼ��ʱ���ڳ�ʼ��pixels
    bool PixelsDeserialization(std::vector<std::vector<unsigned char> > data);

    //д��bmpͼ��ʱ�������л�������Ϣ
    bool PixelsSerialization(std::vector<std::vector<unsigned char> >& data);

    // ����ڲ�ֵ
    void InterNearest(BMP& dst,std::pair<int,int> Size);

    // ˫���Բ�ֵ
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
