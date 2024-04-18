#include "BMP.h"

const bool FAIL = false;
const bool SUCCESS = true;

BMP::BMP()
{
    height = 0;
    width = 0;
    biBitCount = 0;
}

BMP::BMP(int H,int W,int BitCount)
{
    height = H;
    width = W;
    biBitCount = BitCount;
    std::vector<std::vector<PIXELS> >(height,std::vector<PIXELS>(width)).swap(pixels);
}

BMP::~BMP()
{
   
}

bool BMP::ReadImage(std::string ImageName)
{
    FILE* fp;

    if (fopen_s(&fp, ImageName.c_str(), "rb"))
    {
        std::cout << ImageName << " open fail" << std::endl;
        return FAIL;
    }else{
        std::cout << ImageName << " open success" <<std::endl;
    }

    fread(&bmpfileheader, sizeof(bmpfileheader), 1, fp);
    fread(&bitmapinfoheader, sizeof(bitmapinfoheader), 1, fp);
    //读取调色盘
    if (bitmapinfoheader.biBitCount <= 8)
    {
        int n = 1 << bitmapinfoheader.biBitCount;
        rgbquad.resize(n);
        fread(rgbquad.data(), n * 4, 1, fp);
    }

    height = bitmapinfoheader.biHeight;
    width = bitmapinfoheader.biWidth;
    biBitCount = bitmapinfoheader.biBitCount;

    //每行像素占用的内存大小（字节）
    int pixMemorySize = ((width * biBitCount + 7) / 8);
    // 计算像素填充的字节数
    int p = (pixMemorySize % 4 == 0) ? 0 : (4 - (pixMemorySize % 4));

    std::vector<std::vector<unsigned char>> data(height,std::vector<unsigned char>(pixMemorySize));
    for (int i = 0; i < height; i++)
    {
        //读入一行像素，pixMemorySize个字节
        fread(data[i].data(), pixMemorySize, 1, fp);

        //舍弃每行像素后面填充的p个字节
        unsigned char tmp[4];
        fread(tmp, p, 1, fp);
    }
    fclose(fp);
    if(PixelsDeserialization(data) == FAIL){
        std::cout<<"PixelsDeserialization fail";
    }
    return SUCCESS;
}

void BMP::MeanBlur(BMP &dst, std::pair<int, int> size)
{
    dst.SetSize(height,width,biBitCount);

    // 求均值
    auto mean = [this,&size](int ii,int jj)->PIXELS{
        int sum_R = 0;
        int sum_G = 0;
        int sum_B = 0;
        int sum_A = 0;
        int temp_i = size.first / 2;
        int temp_j = size.second / 2;
        int count = 0;
        for(int i = std::max(ii-temp_i,0); i < std::min(ii+temp_i+1,this->height); i++){
            for(int j = std::max(jj-temp_j,0);j< std::min(jj+temp_j+1,this->width);j++){
                sum_R += this->pixels[i][j].R;
                sum_G += this->pixels[i][j].G;
                sum_B += this->pixels[i][j].B;
                sum_A += this->pixels[i][j].A;
                count++;
            }
        }
        return {(unsigned char)(sum_B/count),(unsigned char)(sum_G/count),(unsigned char)(sum_R/count),(unsigned char)(sum_B/count)};

    };

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = mean(i,j);
        }
    }
}

void BMP::GaussianBlur(BMP &dst, std::pair<int, int> size, float sigma)
{
    dst.SetSize(height,width,biBitCount);

    // 计算高斯滤波的卷积核
    auto GetGaussianKernel = [&size,sigma]()->std::vector<std::vector<float>>{
        std::vector<std::vector<float>> kernel(size.first,std::vector<float>(size.second));
        int sum = 0;
        float y = -size.first/2;
        float x = -size.second/2;
        
        float temp = 1/(std::exp(-(x*x+y*y)/(2 * sigma * sigma)));
        for(int i = 0; i<size.first;i++){
            for(int j = 0;j<size.second;j++){
                y = i - size.first/2;
                x = j - size.second/2;
                kernel[i][j] = (int)(std::exp(-(x*x+y*y)/(2 * sigma * sigma))*temp);
                sum += kernel[i][j];
            }
        }

        // 归一化
        for(int i = 0;i<size.first;i++){
            for(int j = 0;j<size.second;j++){
                kernel[i][j] /= sum;
            }
        }
        return kernel;
    };

    auto kernel = GetGaussianKernel();        

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = Convolution(kernel,i,j);
        }
    }
}

void BMP::SetSize(int height, int width, int bitCount)
{
    this->height = height;
    this->width = width;
    this->biBitCount = bitCount;
    std::vector<std::vector<PIXELS> >(height,std::vector<PIXELS>(width)).swap(pixels);
}

bool BMP::PixelsDeserialization(std::vector<std::vector<unsigned char>> data)
{
    //申请height * width大小的空间并释放原来空间
    std::vector<std::vector<PIXELS> >(height,std::vector<PIXELS>(width)).swap(pixels);

    if(biBitCount == 1){
        //biBitCount为1，一个字节里有八个像素的索引信息，用掩码提取出来
        unsigned char mask = 1;
        int temp = 7;
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                mask = 1<<temp;//掩码为1,2,4,8,16...128
                pixels[i][j].A = (data[i][j/8]&mask)>>temp;
                temp = (temp + 7)%8;//7,6,5...0,7,6,5...
            }
        }
    }else if(biBitCount == 4){
        //biBitCount为4，一个字节里有两个像素的索引信息，用掩码提取出来
        unsigned char mask = 0b11110000;
        int temp = 4;
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                pixels[i][j].A = (data[i][j/2]&mask)>>temp;
                mask = mask^0xff;//mask为0b11110000或者0b00001111
                temp = (temp - 4) * (-1);//temp为4或者0；
            }
        }
    }else if(biBitCount == 8){
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                //将调色盘的索引存在PIXELS.A里
                pixels[i][j].A = data[i][j];
            }
        }
    }else if(biBitCount == 16){
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                unsigned short temp = (data[i][j*2]<<8) + data[i][j*2 + 1];
                //通过掩码将像素信息为16位的提取成BGRA(四个部分,占比5551)
                pixels[i][j] = {(unsigned char)((temp&0xF800)>>11),(unsigned char)((temp&0x07C0)>>6),(unsigned char)((temp&0x003E)>>1),(unsigned char)((temp&0x0001))};
            }
        }
    }else if(biBitCount == 24){
        for(int i = 0;i<height;i++){
            for(int j = 0;j < width; j++){
                pixels[i][j] = {data[i][j * 3], data[i][j * 3 + 1],data[i][j * 3 + 2], 0};
            }
        }
    }else if(biBitCount == 32){
        for(int i = 0;i<height;i++){
            for(int j = 0;j < width; j++){
                pixels[i][j] = {data[i][j * 3], data[i][j * 3 + 1],data[i][j * 3 + 2], data[i][j * 3 + 3]};
            }
        }
    }else{
        return FAIL;
    }
    return SUCCESS;
}

bool BMP::WriteImage(std::string ImageName)
{
    FILE* fp = nullptr;
    fopen_s(&fp, ImageName.c_str(), "wb");

    if (fp == nullptr)
    {
        std::cout << "文件打开错误" << std::endl;
        return FAIL;
    }
    //每行像素占用的内存大小（字节）
    int pixMemorySize = ((width * biBitCount + 7) / 8);
    // 计算像素填充的字节数
    int p = (pixMemorySize % 4 == 0) ? 0 : (4 - (pixMemorySize % 4));

    //文件头
    bmpfileheader.bfType = 0x4d42;
    bmpfileheader.bfSize = 14 + 40 + (pixMemorySize + p) * height;
    bmpfileheader.bfReserved1 = 0;
    bmpfileheader.bfReserved2 = 0;
    bmpfileheader.bfOffBits = 54;
    fwrite(&bmpfileheader, sizeof(bmpfileheader), 1, fp);


    //信息头
    bitmapinfoheader.biSize = 40;
    bitmapinfoheader.biWidth = width;
    bitmapinfoheader.biHeight = height;
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = biBitCount;
    bitmapinfoheader.biCompression = 0;
    bitmapinfoheader.biSizeImage = (pixMemorySize + p) * height;
    bitmapinfoheader.biXPelsPerMeter = 0;
    bitmapinfoheader.biYPelsPerMeter = 0;
    bitmapinfoheader.biClrUsed = 0;
    bitmapinfoheader.biClrImportant = 0;
    fwrite(&bitmapinfoheader, sizeof(bitmapinfoheader), 1, fp);

    //调色板
    if (rgbquad.size() > 0)
    {
        fwrite(rgbquad.data(), rgbquad.size() * 4, 1, fp);
    }

    char ch = 0;

    std::vector<std::vector<unsigned char>> data(height,std::vector<unsigned char>(pixMemorySize));
    if(PixelsSerialization(data) == FAIL){
        std::cout<<"PixelsSerialization fail"<<std::endl;
    }
    for (int i = 0; i < height; i++)
    {
        //写入像素数据
        fwrite(data[i].data(), pixMemorySize, 1, fp);
        //填充字节
        fwrite(&ch, 1, p, fp);
    }

    fclose(fp);
    return SUCCESS;
}

bool BMP::Flip(int FlipCode)
{
    // 垂直翻转
    if(FlipCode == 0){
        for(int i = 0;i<height/2; i++){
            pixels[i].swap(pixels[height - i - 1]);
        }
        return SUCCESS;
    }else if(FlipCode > 0){
        // 水平翻转
        PIXELS temp;
        for(int i = 0;i<height;i++){
            for(int j = 0;j< width/2; j++){
                temp = pixels[i][j];
                pixels[i][j] = pixels[i][width - j - 1];
                pixels[i][width - j - 1] = temp;
            }
        }
        return SUCCESS;
    }else if(FlipCode < 0){
        // 同时垂直和水平翻转
        for(int i = 0;i<height/2; i++){
            pixels[i].swap(pixels[height - i - 1]);
        }
        PIXELS temp;
        for(int i = 0;i<height;i++){
            for(int j = 0;j< width/2; j++){
                temp = pixels[i][j];
                pixels[i][j] = pixels[i][width - j - 1];
                pixels[i][width - j - 1] = temp;
            }
        }
        return SUCCESS;
    }else{
        return FAIL;
    }
}

bool BMP::Resize(BMP& Dst,std::pair<int, int> Size, double Fy, double Fx, Interpolation Interpolation)
{
    if(Size.first == 0){
        if(Fy == 0){
            std::cout<<"Size和Fx（Fy）不能同时为0"<<std::endl;
            return FAIL;
        }
        Size.first = height * Fy;
    }
    if(Size.second == 0){
        if(Fx == 0){
            std::cout<<"Size和Fx（Fy）不能同时为0"<<std::endl;
            return FAIL;
        }
        Size.second = width * Fx;
    }
    
    // 初始化Dst的大小
    Dst.height = Size.first;
    Dst.width = Size.second;
    Dst.biBitCount = biBitCount;
    std::vector<std::vector<PIXELS> >(Dst.height,std::vector<PIXELS>(Dst.width)).swap(Dst.pixels);

    switch (Interpolation)
    {
    case INTER_NEAREST:
        InterNearest(Dst,Size);
        break;
    case INTER_LINEAR:
        InterLinear(Dst,Size);
    default:
        break;
    }
    return SUCCESS;
}

void BMP::WeightGray(BMP &dst, std::vector<float> weights)
{
    dst.SetSize(height,width,biBitCount);

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = weights[0] * pixels[i][j].B + weights[1] * pixels[i][j].G + weights[2] * pixels[i][j].R;
        }
    }
}

void BMP::Inverse(BMP &dst)
{
    dst.SetSize(height,width,biBitCount);
    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = 255 - pixels[i][j];
        }
    }
}

void BMP::RectCut(BMP& dst,std::pair<int,int> ul,std::pair<int,int> lr)
{
    dst.SetSize(lr.second - ul.second,lr.first - ul.first,biBitCount);

    // pixels中的行是上下颠倒的
    ul.second = height - ul.second;
    lr.second = height - lr.second;

    int temp = ul.second;
    ul.second = lr.second;
    lr.second = temp;

    for(int i = 0;i<dst.height;i++){
        for(int j = 0;j<dst.width;j++){
            dst.pixels[i][j] = pixels[i+ul.second][j+ul.first];
        }
    }

}

void BMP::MadianBlur(BMP &dst, std::pair<int, int> size)
{
    dst.SetSize(height,width,biBitCount);

    // 求中值
    auto madian = [this,size](int ii,int jj)->PIXELS{
        std::vector<std::vector<unsigned char> > arr(4);
        int temp_i = size.first / 2;
        int temp_j = size.second / 2;
        for(int i = std::max(ii-temp_i,0); i < std::min(ii+temp_i+1,this->height); i++){
            for(int j = std::max(jj-temp_j,0);j< std::min(jj+temp_j+1,this->width);j++){
                arr[0].push_back(this->pixels[i][j].B);
                arr[1].push_back(this->pixels[i][j].G);
                arr[2].push_back(this->pixels[i][j].R);
                arr[3].push_back(this->pixels[i][j].A);
            }
        }
        std::sort(arr[0].begin(),arr[0].end());
        std::sort(arr[1].begin(),arr[1].end());
        std::sort(arr[2].begin(),arr[2].end());
        std::sort(arr[3].begin(),arr[3].end());
        int leng = arr[0].size();
        return {arr[0][leng/2],arr[1][leng/2],arr[2][leng/2],arr[3][leng/2]};
    };

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = madian(i,j);
        }
    }
}

bool BMP::PixelsSerialization(std::vector<std::vector<unsigned char> >& data){
    if(biBitCount == 1){
        //biBitCount为1，将八个像素的索引信息放到一个字节里
        for(int i = 0;i<height;i++){
            int j;
            for(j = 7;j<width;j+=8){
                data[i][j/8] = pixels[i][j-7].A<<7
                    + pixels[i][j - 6].A<<6
                    + pixels[i][j - 5].A<<5
                    + pixels[i][j - 4].A<<4
                    + pixels[i][j - 3].A<<3
                    + pixels[i][j - 2].A<<2
                    + pixels[i][j - 1].A<<1
                    + pixels[i][j].A;
            }
            int temp = 7;
            data[i][width / 8] = 0;
            for(j;j<width;j++){//剩余不足8个像素的部分
                data[i][width / 8] += pixels[i][j].A<<temp;
                temp--;
            }
        }
    }else if(biBitCount == 4){
        //biBitCount为4，将两个像素的索引信息放在一个字节里
        for(int i = 0;i<height;i++){
            for(int j = 1;j<width;j+=2){
                data[i][j/2] = pixels[i][j-1].A<<4 + pixels[i][j].A;
            }
            if((width&1)==1){//width是奇数
                data[i][width/2] = pixels[i][width - 1].A<<4;
            }
        }
    }else if(biBitCount == 8){
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                //调色盘的索引存在PIXELS.A里
                data[i][j] = pixels[i][j].A;
            }
        }
    }else if(biBitCount == 16){
        for(int i = 0;i<height;i++){
            for(int j = 0;j<width;j++){
                unsigned short temp = (unsigned short)pixels[i][j].B<<11 
                    + (unsigned short)pixels[i][j].G<<6
                    + (unsigned short)pixels[i][j].R<<1
                    + (unsigned short)pixels[i][j].A;   

                data[i][j*2] = (temp&0xff00)>>8;
                data[i][j*2+1] = (temp&0x00ff);         
            }
        }
    }else if(biBitCount == 24){
        for(int i = 0;i<height;i++){
            for(int j = 0;j < width; j++){
                data[i][j*3] = pixels[i][j].B;
                data[i][j*3 + 1] = pixels[i][j].G;
                data[i][j*3 + 2] = pixels[i][j].R;
            }
        }
    }else if(biBitCount == 32){
        for(int i = 0;i<height;i++){
            for(int j = 0;j < width; j++){
                data[i][j*4] = pixels[i][j].B;
                data[i][j*4 + 1] = pixels[i][j].G;
                data[i][j*4 + 2] = pixels[i][j].R;
                data[i][j*4 + 3] = pixels[i][j].A;
            }
        }
    }else{
        return FAIL;
    }
    return SUCCESS;
}

void BMP::InterNearest(BMP &dst, std::pair<int, int> Size)
{
    // 垂直缩放因子
    double fy = (double)Size.first/height;
    // 水平缩放因子
    double fx = (double)Size.second/width;

    for(int i = 0;i<Size.first;i++){
        // 对应的原图的像素的垂直位置
        int sy = ((double)i + 0.5)/fy;
        sy = std::min(sy,Size.first - 1);

        for(int j = 0;j<Size.second;j++){

            // 对应的原图的像素的水平位置
            int sx = ((double)j + 0.5)/fx;
            sx = std::min(sx,Size.second - 1);
            dst.pixels[i][j] = pixels[sy][sx];
        }
    }
}

void BMP::InterLinear(BMP &dst, std::pair<int, int> Size)
{
    // 垂直缩放因子
    double fy = (double)Size.first/height;
    // 水平缩放因子
    double fx = (double)Size.second/width;

    for (int i = 0; i < Size.first; i++)
    {
        //几何中心对齐
        double index_i = (i + 0.5) / fy - 0.5;
        //相邻2*2像素的行（坐标）
        int i1 = floor(index_i);
        int i2 = ceil(index_i);
        //u为得到浮点型坐标行的小数部分
        double u = index_i - i1;
        //防止越界
        if (i1 < 0) i1 = 0;
        if (i2 >= height) i2 = height - 1;
        
        for (int j = 0; j < Size.second; j++)
        {
            //几何中心对齐
            double index_j = (j + 0.5) / fx - 0.5;
            
            //相邻2*2像素的列（坐标）
            int j1 = floor(index_j);
            int j2 = ceil(index_j);
            //v为得到浮点型坐标列的小数部分
            double v = index_j - j1;
            //防止越界
            if (j1 < 0) j1 = 0;
            if (j2 >= width) j2 = width - 1;
            

            dst.pixels[i][j].B = (1 - u) * (1 - v) * pixels[i1][j1].B + (1 - u) * v * pixels[i1][j2].B + u * (1 - v) * pixels[i2][j1].B + u * v * pixels[i2][j2].B;
            dst.pixels[i][j].G = (1 - u) * (1 - v) * pixels[i1][j1].G + (1 - u) * v * pixels[i1][j2].G + u * (1 - v) * pixels[i2][j1].G + u * v * pixels[i2][j2].G;
            dst.pixels[i][j].R = (1 - u) * (1 - v) * pixels[i1][j1].R + (1 - u) * v * pixels[i1][j2].R + u * (1 - v) * pixels[i2][j1].R + u * v * pixels[i2][j2].R;
            dst.pixels[i][j].A = (1 - u) * (1 - v) * pixels[i1][j1].A + (1 - u) * v * pixels[i1][j2].A + u * (1 - v) * pixels[i2][j1].A + u * v * pixels[i2][j2].A;
          
        }
    }
}

template <typename T>
PIXELS BMP::Convolution(const std::vector<std::vector<T>>& kernel, int ii, int jj)
{
    int sum_R = 0;
    int sum_G = 0;
    int sum_B = 0;
    int sum_A = 0;
    int temp_i = kernel.size() / 2;
    int temp_j = kernel[0].size() / 2;
    int x = -1,y = -1;
    for(int i = std::max(ii-temp_i,0); i < std::min(ii+temp_i+1,this->height); i++){
        y = i-ii+temp_i;
        for(int j = std::max(jj-temp_j,0);j< std::min(jj+temp_j+1,this->width);j++){
            x = j-jj+temp_j;
            sum_R += this->pixels[i][j].R * kernel[y][x];
            sum_G += this->pixels[i][j].G * kernel[y][x];
            sum_B += this->pixels[i][j].B * kernel[y][x];
            sum_A += this->pixels[i][j].A * kernel[y][x];
        }
    }

    return {(unsigned char)abs(sum_B),(unsigned char)abs(sum_G),(unsigned char)abs(sum_R),(unsigned char)abs(sum_A)};
}

void BMP::Sobel(BMP &dst, int dy, int dx)
{
    dst.SetSize(height,width,biBitCount);

    std::vector<std::vector<int>> kernelX = {{-1,0,1},{-2,0,2},{-1,0,1}};
    std::vector<std::vector<int>> kernelY = {{-1,-2,-1},{0,0,0},{1,2,1}};
    

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            PIXELS x = Convolution(kernelX,i,j);
            PIXELS y = Convolution(kernelY,i,j);
            dst.pixels[i][j] = x*dx+y*dy;
        }
    }
}

void BMP::Sharpen(BMP &dst,std::vector<std::vector<int>> kernel={{0,-1,0},{-1,5,-1},{0,-1,0}})
{
    dst.SetSize(height,width,biBitCount);

    for(int i = 0;i<height;i++){
        for(int j = 0;j<width;j++){
            dst.pixels[i][j] = Convolution(kernel,i,j);
        }
    }
    
}

PIXELS operator-(unsigned char num, PIXELS &p)
{
    return {(unsigned char)(num - p.B),(unsigned char)(num - p.G),(unsigned char)(num - p.R),(unsigned char)(num - p.A)};
}
