#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <cstring>

typedef unsigned char BYTE;
typedef unsigned int uint;
// 3D的数据的类
class VolumeData
{
public:
	float maxValue;
	float minValue;
	uint size[3];
	float *data;

	VolumeData() {
		size[0] = 0;
		size[1] = 0;
		size[2] = 0;
	}

	~VolumeData() {
		if (size[0] != 0) {
			free(data);
		}
	}

	void allocate(uint sizex, uint sizey, uint sizez);

	void newData(uint sizex, uint sizey, uint sizez, float *data);

	void clear();

	float get(uint x, uint y, uint z);
	void set(uint x, uint y, uint z, float data);

	void printSize();

	void fillDummyData();
};

class CCP4Reader
{
public:
	// 解析ccp4数据的大小
	// 大小的每3个成分为uint array
	void ccp4size(std::vector<BYTE> &ccp4Data, uint *size);

	VolumeData* read(std::string filename);
};
