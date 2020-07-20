#include <ccp4reader.h>
#include <EasyBMP/EasyBMP.h>

int main(int argc, char *argv[])
{
    // 读文件
    CCP4Reader reader;
    VolumeData *densityData;
    if(argc > 1) {
        densityData = reader.read(argv[1]);
    } else {
        densityData = reader.read("data/emd_10410_96.map");
    }

    // BMP image可以做一个图片
    int xPixelNum = densityData->size[0];
    int yPixelNum = densityData->size[1];
    BMP bpmImage;
    bpmImage.SetSize(xPixelNum, yPixelNum);
    bpmImage.SetBitDepth(24);

    // 每个像素：计算结果颜色
    for(int j = 0; j < densityData->size[1]; j++) {
        for(int i = 0; i < densityData->size[1]; i++) {
            bpmImage(i, j)->Red = 255;
            bpmImage(i, j)->Green = 0;
            bpmImage(i, j)->Blue = 0;
        }
    }

    // 保存到文件
    bpmImage.WriteToFile("result.bmp");

    return 0;
}