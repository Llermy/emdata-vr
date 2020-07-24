#include "ccp4reader.h"
#include "ccp4reader.h"


void VolumeData::allocate(uint sizex, uint sizey, uint sizez)
{
	this->data = new float[sizex*sizey*sizez];
	size[0] = sizex;
	size[1] = sizey;
	size[2] = sizez;
}

void VolumeData::newData(uint sizex, uint sizey, uint sizez, float *data) {
	this->data = data;
	size[0] = sizex;
	size[1] = sizey;
	size[2] = sizez;
}

void VolumeData::clear() {
	size[0] = 0;
	size[1] = 0;
	size[2] = 0;
	free(this->data);
}

float VolumeData::get(uint x, uint y, uint z) {
	return this->data[x + y * size[0] + z * size[0] * size[1]];
}

void VolumeData::set(uint x, uint y, uint z, float data) {
	this->data[x + y * size[0] + z * size[0] * size[1]] = data;
}

void VolumeData::printSize() {
	std::cout << "���ݴ�СΪ��" << size[0] << "��" << size[1] << "��" << size[2] << "��\n";
}

void VolumeData::fillDummyData() {
	if (size[0] != 0) {
		this->clear();
	}
	size[0] = 2;
	size[1] = 2;
	size[2] = 2;
	uint length = size[0] * size[1] * size[2];
	this->data = new float[length];
	float dt[8] = { 1,  1.5, 0.5, 1, 2, 1, 0, 0.5 };
	memcpy(this->data, dt, length * sizeof(float));

	this->minValue = 0;
	this->maxValue = 2;
}


void CCP4Reader::ccp4size(std::vector<BYTE> &ccp4Data, uint *size)
{
	size[0] = ccp4Data[2] * 65536 + ccp4Data[1] * 256 + ccp4Data[0];
	size[1] = ccp4Data[6] * 65536 + ccp4Data[5] * 256 + ccp4Data[4];
	size[2] = ccp4Data[10] * 65536 + ccp4Data[9] * 256 + ccp4Data[8];
}

VolumeData* CCP4Reader::read(std::string filename)
{
	FILE *pFile;
	long fileSize;
	VolumeData *voldata = new VolumeData();

	// ���ļ�
	pFile = fopen(filename.c_str(), "rb");
	if (pFile == NULL) { fputs("File error", stderr); exit(1); }

	// ȡ�ļ���С
	fseek(pFile, 0, SEEK_END);
	fileSize = ftell(pFile);
	rewind(pFile);
	std::cout << "������С��" << fileSize << "\n";

	// ����ccp4���ݵĴ�С
	uint *size = (uint*)malloc(sizeof(uint) * 3);
	fread((void*)(size), sizeof(size[0]), 3, pFile);

	// �������ݵ�������ֵ
	fseek(pFile, 19 * 4, SEEK_SET);
	fread((void*)(&voldata->minValue), sizeof(voldata->minValue), 1, pFile);
	fread((void*)(&voldata->maxValue), sizeof(voldata->maxValue), 1, pFile);

	// ��ccp4����
	uint numfloats = (fileSize - 1024) / 4; // ͷ��Ϊ1024byte��ÿ����������4byte��TODO���п���ͷ���������м���symmetry records�����ڼ���û�У�
	fseek(pFile, 1024, SEEK_SET);
	float *buffer = (float*)malloc(sizeof(float)*numfloats);
	fread((void*)(buffer), sizeof(buffer[0]), numfloats, pFile); // TODO������ccp4�ļ��Ķ����������ǵ��ֽ����Ǹ��ֽ�������Ĭ�ϴ���Ϊ���ֽ���

	voldata->newData(size[0], size[1], size[2], buffer);

	fclose(pFile);
	free(size);

	return voldata;
}