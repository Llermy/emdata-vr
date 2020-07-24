
#include "stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include "glut.h"
#include "ccp4reader.h"

#define EPSILON 0.000001
#define WIDTH 800
#define HEIGTH 600
float Image[WIDTH*HEIGTH * 4];

void GenerateVolume(int *Data, int* Dim);//����������
void GenerateVolume(VolumeData *Data, int* Dim);//����������
void GenCube(int x, int y, int z, int side, int density, int *Data, int *Dim);//��������������
void GenSphere(int x, int y, int z, int radius, int density, int *Data, int *Dim);//������������
void Classify(float* CData, float *Data, int *Dim);//���ݷ���
void RotationMatrix(float *R, float *eye, float *center, float *up);//��ȡͼ��ռ䵽����ռ�任����ת����
void Composite(float *rgba, int x0, int y0, float *CData, int *Dim, float *R, float *T);//�ϳ�������ɫֵ
bool Intersection(float *startpos, float *pos, float *dir, int *Dim);//��������Χ�н�������
void TrInterpolation(float *rgba, float *pos, float *CData, int *Dim);//�����Բ�ֵ
bool CheckinBox(float *point, int *Dim);//�жϵ��Ƿ��ڰ�Χ����
void MatrixmulVec(float *c, float *a, float *b);//���������˻�
void CrossProd(float *c, float *a, float *b);//�������
void Normalize(float *norm, float *a);//������һ��
void Mydisplay();//��ʾͼ��
void MyIdle();
void MouseHandler(int button, int state, int x, int y);
void keyBoardHandler(unsigned char c, int x, int y);
void InitGL();

float minvalue, maxvalue;
static float threshold = 6.0f;
std::string datapath = "data/emd_21965_128.map";

float R[9];//��ת����
float T[3] = { 0,0,450 };//ƽ�ƾ���
int Dim[3];
float *CDataptr, *densitydata;

int main()
{
	CCP4Reader reader;
	VolumeData *densityData;
	densityData = reader.read(datapath);
	densitydata = densityData->data;
	int xPixelNum = densityData->size[0];
	int yPixelNum = densityData->size[1];
	int zPixelNum = densityData->size[2];

	Dim[0] = xPixelNum;
	Dim[1] = yPixelNum;
	Dim[2] = zPixelNum;

	float *CData = (float*)malloc(sizeof(float)*Dim[0] * Dim[1] * Dim[2] * 4);
	CDataptr = CData;
	minvalue = densityData->minValue;
	maxvalue = densityData->maxValue;
	printf("%d %d %d %f %f \n", xPixelNum, yPixelNum, zPixelNum, maxvalue, minvalue);

	float eye[3] = { 1,1,2 };//�ӵ�λ��
	float center[3] = { 0,0,0 };//����ο���λ��
	float up[3] = { 0,1,0 };//������ϵķ���

	RotationMatrix(R, eye, center, up);//�����ת����

	Classify(CData, densityData->data, Dim);//�������ݷ���

	float *LinePS = Image;
	for (int j = 0; j < HEIGTH; j++)//����ϳ�����ֵ
	{
		for (int i = 0; i < WIDTH; i++)
		{
			Composite(LinePS, i, j, CData, Dim, R, T);
			LinePS += 4;
		}
	}
	InitGL();
}

void InitGL() {
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Ray-Casting");
	glClearColor(1, 1, 1, 1);//������Ϊ��ɫ
	glutDisplayFunc(Mydisplay);//��ʾͼ��
	glutIdleFunc(MyIdle);
	glutMouseFunc(MouseHandler);
	glutKeyboardFunc(keyBoardHandler);
	glutMainLoop();
}

void ClassifyAndComposite() {

	Classify(CDataptr, densitydata, Dim);//�������ݷ���

	float *LinePS = Image;
	for (int j = 0; j < HEIGTH; j++)//����ϳ�����ֵ
	{
		for (int i = 0; i < WIDTH; i++)
		{
			Composite(LinePS, i, j, CDataptr, Dim, R, T);
			LinePS += 4;
		}
	}
}

//���ݷ���
//��ԭʼ�����ݵı���ֵӳ��Ϊ��ɫ�Ͳ�͸���� 
//CData:�����������
//Data:ԭʼ������
//Dim:�����ݴ�С
void Classify(float* CData, float *Data, int *Dim)
{
	float *LinePS = Data;
	float *LinePD = CData;
	for (int k = 0; k < Dim[2]; k++)
	{
		for (int j = 0; j < Dim[1]; j++)
		{
			for (int i = 0; i < Dim[0]; i++)
			{
				if (LinePS[0] <= (minvalue + threshold))
				{
					//�˳�������Ҫ��
					LinePD[0] = 1.0;
					LinePD[1] = 1.0;
					LinePD[2] = 1.0;
					LinePD[3] = 0.005;
				}
				else {
					LinePD[0] = 1.0;
					LinePD[1] = (LinePS[0] - minvalue) / (maxvalue - minvalue);
					LinePD[2] = 0.0;
					LinePD[3] = (LinePS[0] - minvalue) / (maxvalue - minvalue);
				}
				LinePS++;
				LinePD += 4;
			}
		}
	}
}

//��ȡ��ͼ��ռ䵽����ռ�任����ת����
//����������OpenGL�е�gluLookAt����
//�ο���http://blog.csdn.net/popy007/article/details/5120158
//R:��ת����
//eye:�ӵ�λ��
//center:����ο���λ��
//up:������ϵķ���
void RotationMatrix(float *R, float *eye, float *center, float *up)
{
	float XX[3], YY[3], ZZ[3];//ͼ��ռ�Ļ�����
	ZZ[0] = eye[0] - center[0];
	ZZ[1] = eye[1] - center[1];
	ZZ[2] = eye[2] - center[2];
	CrossProd(XX, up, ZZ);
	CrossProd(YY, ZZ, XX);
	Normalize(XX, XX);
	Normalize(YY, YY);
	Normalize(ZZ, ZZ);
	//��ͼ��ռ������������ת����
	R[0] = XX[0]; R[1] = YY[0]; R[2] = ZZ[0];
	R[3] = XX[1]; R[4] = YY[1]; R[5] = ZZ[1];
	R[6] = XX[2]; R[7] = YY[2]; R[8] = ZZ[2];
}

//�ϳ�����ֵ
//rgba:�ϳ���ɫֵ
//x0,y0:��άͼ����������
//CData:�����������
//Dim:�����ݴ�С
//R:��ת���󣨻�����ͼ��ռ䵽����ռ��ת����
//T:ƽ��������ͬ�ϣ�
void Composite(float *rgba, int x0, int y0, float *CData, int *Dim, float *R, float *T)
{
	int stepsize = 1;//��������
	float cumcolor[4];//�ۼ���ɫֵ
	cumcolor[0] = cumcolor[1] = cumcolor[2] = cumcolor[3] = 0.0;
	float pos[3], dir[3];//Ͷ�������㡢����
	float startpos[3];//�������Χ�н��ӵ㴦�Ľ�������
	float samplepos[3];//����������
	float samplecolor[4];//��������ɫ
						 //����ƽ��ͶӰ������ͼ��ռ���Ͷ����ߵķ���(0,0,-1),���(x0,y0,0)
	pos[0] = x0; pos[1] = y0; pos[2] = 0;
	//����������ת��������ռ�

	dir[0] = -R[2]; dir[1] = -R[5]; dir[2] = -R[8];//���߷���������ռ�ı��
	MatrixmulVec(pos, R, pos);//��ת
	pos[0] += T[0];//ƽ��
	pos[1] += T[1];
	pos[2] += T[2];

	if (Intersection(startpos, pos, dir, Dim))//�жϹ������Χ���Ƿ��ཻ
	{
		samplepos[0] = startpos[0];
		samplepos[1] = startpos[1];
		samplepos[2] = startpos[2];
		while (CheckinBox(samplepos, Dim) && cumcolor[3] < 1)//�����������Χ�л��ۼƲ�͸���ȳ���1ʱ��ֹ�ϳ�
		{
			TrInterpolation(samplecolor, samplepos, CData, Dim);//�����Բ�ֵ��ò����㴦����ɫ����͸����
																//�ϳ���ɫ����͸����,���õ��Ǵ�ǰ����ĺϳɹ�ʽ
			cumcolor[0] += samplecolor[0] * samplecolor[3] * (1 - cumcolor[3]);//R
			cumcolor[1] += samplecolor[1] * samplecolor[3] * (1 - cumcolor[3]);//G
			cumcolor[2] += samplecolor[2] * samplecolor[3] * (1 - cumcolor[3]);//B
			cumcolor[3] += samplecolor[3] * (1 - cumcolor[3]);				//A
																			//��һ��������
			samplepos[0] += dir[0] * stepsize;
			samplepos[1] += dir[1] * stepsize;
			samplepos[2] += dir[2] * stepsize;
		}
		rgba[0] = cumcolor[0];
		rgba[1] = cumcolor[1];
		rgba[2] = cumcolor[2];
		rgba[3] = cumcolor[3];
		//printf("%d %d \n", x0, y0);
		return;
	}
	rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1.0;//���������Χ�в��ཻ������ɫ
}

//�ж�Ͷ��������Χ���Ƿ��ཻ�����ཻ���󿿽��ӵ㴦�Ľ������꣩
//˼·������Χ��6����������չ�����ֳ�3�飬��ƽ����XOY,YOZ,ZOXƽ��ĸ���2����
//�������6��ƽ��Ľ��㣬��ÿ���2��������ѡ�������ӵ�Ͻ��ߣ������õ�3����
//ѡ���㣻����3����ѡ������ѡ�������ӵ���Զ���Ǹ�������ж�������Ƿ����ڰ�
//Χ���ڣ����ڣ���Ϊ�������Χ�еĿ����ӵ㴦�Ľ��㡣
//stratpos:�����ӵ㴦�Ľ�������
//pos:�����������
//dir:���߷�������
//Dim:��Χ�����Ͻ����꣨���½�����Ϊ��0,0,0����
bool Intersection(float *startpos, float *pos, float *dir, int *Dim)
{
	float nearscale = -1000000;
	float scale1, scale2;
	//�������Χ��ƽ����YOZ��2��ƽ�潻��
	if ((dir[0] <= -EPSILON) || (dir[0] >= EPSILON))
	{
		scale1 = (0 - pos[0]) / dir[0];
		scale2 = (Dim[0] - 1 - pos[0]) / dir[0];
		//ѡ�������ӵ�Ľ��㣬���뵱ǰ��ѡ��Ƚϣ�������Զ��
		if (scale1 < scale2)
		{
			if (scale1 > nearscale)
				nearscale = scale1;
		}
		else
		{
			if (scale2 > nearscale)
				nearscale = scale2;
		}
	}
	//�������Χ��ƽ����ZOX��2��ƽ�潻��
	if ((dir[1] <= -EPSILON) || (dir[1] >= EPSILON))
	{
		scale1 = (0 - pos[1]) / dir[1];
		scale2 = (Dim[1] - 1 - pos[1]) / dir[1];
		//ѡ�������ӵ�Ľ��㣬���뵱ǰ��ѡ��Ƚϣ�������Զ��
		if (scale1 < scale2)
		{
			if (scale1 > nearscale)
				nearscale = scale1;
		}
		else
		{
			if (scale2 > nearscale)
				nearscale = scale2;
		}
	}
	//�������Χ��ƽ����XOY��2��ƽ�潻��
	if ((dir[2] <= -EPSILON) || (dir[2] >= EPSILON))
	{
		scale1 = (0 - pos[2]) / dir[2];
		scale2 = (Dim[2] - 1 - pos[2]) / dir[2];
		//ѡ�������ӵ�Ľ��㣬���뵱ǰ��ѡ��Ƚϣ�������Զ��
		if (scale1 < scale2)
		{
			if (scale1 > nearscale)
				nearscale = scale1;
		}
		else
		{
			if (scale2 > nearscale)
				nearscale = scale2;
		}
	}
	startpos[0] = pos[0] + nearscale * dir[0];
	startpos[1] = pos[1] + nearscale * dir[1];
	startpos[2] = pos[2] + nearscale * dir[2];
	return CheckinBox(startpos, Dim);  //�жϸõ��Ƿ��ڰ�Χ����
}

//�����Բ�ֵ
//rgba:��ֵ���
//pos:����������
//CData:�����������
//Dim:�����ݴ�С
void TrInterpolation(float *rgba, float *pos, float *CData, int *Dim)
{
	int x0, y0, z0, x1, y1, z1;
	float fx, fy, fz;
	float v0, v1, v2, v3, v4, v5, v6;
	int Slicesize = Dim[0] * Dim[1] * 4;
	int Stepsize = Dim[0] * 4;
	x0 = (int)pos[0];//��������
	y0 = (int)pos[1];
	z0 = (int)pos[2];
	fx = pos[0] - x0;//С������
	fy = pos[1] - y0;
	fz = pos[2] - z0;
	x1 = x0 + 1;
	y1 = y0 + 1;
	z1 = z0 + 1;
	if (x1 >= Dim[0])x1 = Dim[0] - 1;//��ֹԽ��
	if (y1 >= Dim[1])y1 = Dim[1] - 1;
	if (z1 >= Dim[2])z1 = Dim[2] - 1;
	for (int i = 0; i < 4; i++)
	{
		//�����㴦��ֵ���ڽ���8�����ֵ���
		v0 = CData[z0*Slicesize + y0*Stepsize + 4 * x0 + i] * (1 - fx) + CData[z0*Slicesize + y0*Stepsize + 4 * x1 + i] * fx;
		v1 = CData[z0*Slicesize + y1*Stepsize + 4 * x0 + i] * (1 - fx) + CData[z0*Slicesize + y1*Stepsize + 4 * x1 + i] * fx;
		v2 = CData[z1*Slicesize + y0*Stepsize + 4 * x0 + i] * (1 - fx) + CData[z1*Slicesize + y0*Stepsize + 4 * x1 + i] * fx;
		v3 = CData[z1*Slicesize + y1*Stepsize + 4 * x0 + i] * (1 - fx) + CData[z1*Slicesize + y1*Stepsize + 4 * x1 + i] * fx;
		v4 = v0*(1 - fy) + v1*fy;
		v5 = v2*(1 - fy) + v3*fy;
		v6 = v4*(1 - fz) + v5*fz;
		if (v6 > 1)v6 = 1;//��ֹԽ��
		rgba[i] = v6;
	}
}

//�жϵ��Ƿ��ڰ�Χ����
//point:������
//Dim:��Χ�����Ͻ����꣨���½�����Ϊ��0,0,0����
bool CheckinBox(float *point, int *Dim)
{
	if (point[0] < 0 || point[0] >= Dim[0] || point[1] < 0 || point[1] >= Dim[1] || point[2] < 0 || point[2] >= Dim[2])
		return false;
	else
		return true;
}

//�����������˻�
//c=a*b
//c:�������
//a:�������
//b:��������
void MatrixmulVec(float *c, float *a, float *b)
{
	float x, y, z;
	x = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	y = a[3] * b[0] + a[4] * b[1] + a[5] * b[2];
	z = a[6] * b[0] + a[7] * b[1] + a[8] * b[2];
	c[0] = x;
	c[1] = y;
	c[2] = z;
}

//�������
//c=a x b
//c:�������
//a:��������
//b:��������
void CrossProd(float *c, float *a, float *b)
{
	float x, y, z;
	x = a[1] * b[2] - b[1] * a[2];
	y = a[2] * b[0] - b[2] * a[0];
	z = a[0] * b[1] - b[0] * a[1];
	c[0] = x;
	c[1] = y;
	c[2] = z;
}

//������һ��
//norm:��һ�����
//a:��������
void Normalize(float *norm, float *a)
{
	float len = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	norm[0] = a[0] / len;
	norm[1] = a[1] / len;
	norm[2] = a[2] / len;
}

//��ʾ����
void Mydisplay()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(WIDTH, HEIGTH, GL_RGBA, GL_FLOAT, Image);//ʹ��OpenGL�Ļ�ͼ����
	glFlush();
}

void MyIdle() {
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(WIDTH, HEIGTH, GL_RGBA, GL_FLOAT, Image);//ʹ��OpenGL�Ļ�ͼ����
	glFlush();
}

void MouseHandler(int button, int state, int x, int y) {

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

	}
}

void keyBoardHandler(unsigned char c, int x, int y) {
	//float step = (maxvalue - minvalue) / 20;
	float step = 0.1f;
	switch (c) {
	case 'd':
		threshold += step;
		if (threshold >= maxvalue) {
			threshold = (maxvalue)-(maxvalue - minvalue) / 10;
		}
		ClassifyAndComposite();
		break;
	case 'a':
		threshold -= step;
		if (threshold <= minvalue) {
			threshold = (minvalue)+(maxvalue - minvalue) / 10;
		}		
		ClassifyAndComposite();
		break;
	case 'w':

		break;
	case 's':

		break;
	case 27:
		exit(0);
		break;
	}


}
