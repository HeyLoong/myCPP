
//调用方法
// int nOriX=288; //重采样后tif的像素长col
// int nOriY=266; //重采样后tif的像素宽row
// resizeGByteTiff(nOriX, nOriY, "原始数据.tif", "重采样后数据.tif")


#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
#include<opencv2/highgui.hpp>
using namespace cv;
using namespace std;
void resizeGByteTiff(nOriImgX, nOriImgY, std::string initialTiff, std::string newTiff)
{
	GDALAllRegister();
	//设置支持中文路径
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	const char * pszFile = initialTiff.c_str();
	GDALDataset *poDataset = (GDALDataset*)GDALOpen(pszFile, GA_ReadOnly);//使用只读方式打开图像 
	if (!poDataset)
	{
		printf("File: %s不能打开！\n", pszFile);
	}


	//仿射参数
	double padfTransform0[6];
	if (poDataset->GetGeoTransform(padfTransform0) == CE_Failure)
	{
		printf("获取仿射变换参数失败");
	}
	int iImgSizeX0 = poDataset->GetRasterXSize();
	int iImgSizeY0 = poDataset->GetRasterYSize();
	int nCount = poDataset->GetRasterCount(); 	//影像的波段数
	GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();//获取栅格类型
	double dnodata = poDataset->GetRasterBand(1)->GetNoDataValue();//获取空值对应大小
	unsigned char *_pNewValue = new unsigned char[iImgSizeX0*iImgSizeY0];
	poDataset->RasterIO(GF_Read, 0, 0, iImgSizeX0, iImgSizeY0, _pNewValue, iImgSizeX0, iImgSizeY0, gdal_data_type, 1, 0, 0, 0, 0);

	//转化为Mat
	cv::Mat _Mymat(iImgSizeY0, iImgSizeX0, CV_8UC1);
	for (int i = 0; i < iImgSizeX0; i++)
	{
		for (int j = 0; j < iImgSizeY0; j++)
		{
			_Mymat.at<uchar>(j, i) = (uchar)_pNewValue[j * iImgSizeX0 + i];	
		}
	}
    //基于Opencv重采样
	Size dsize = Size(nOriImgX, nOriImgY);
	Mat newMat = Mat(dsize, CV_32S);
	resize(_Mymat, newMat, dsize);//可修改该函数参数，实现基于不同算法的插值处理
 
    //将重采样后的Mat数据读出
	int a = 0;
	unsigned char* _pimageData = new unsigned char[nOriImgX*nOriImgY];
	for (int i = 0; i < newMat.rows; i++)
	{
		for (int j = 0; j < newMat.cols; j++)
		{
			_pimageData[a] = newMat.at<uchar>(i, j);
			a++;
		}
	}
	
	//缩放：保证大小一致，分辨率增加
	double scale = (double)nOriImgX / iImgSizeX0;
	padfTransform0[1] = padfTransform0[1] / scale;
	scale = (double)nOriImgY / iImgSizeY0;;
	padfTransform0[5] = padfTransform0[5] / scale;

	//写入新的tif	 
	int pBandMap[3] = { 1,2,3 };//定义波段排序顺序
	const char* pszDstFilename = newTiff.c_str();
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* poDstDS = poDriver->Create(pszDstFilename, nOriImgX, nOriImgY, nCount, GDT_Byte, NULL);
	poDstDS->SetProjection(poDataset->GetProjectionRef());//给它设置投影
	poDstDS->SetGeoTransform(padfTransform0);//给设置空间转换的六参数
	poDstDS->GetRasterBand(1)->SetNoDataValue(dnodata);//将空值设置为“无数据值”
	//保存影像
	poDstDS->RasterIO(GF_Write, 0, 0, nOriImgX, nOriImgY, _pimageData, nOriImgX, nOriImgY, GDT_Byte, nCount, pBandMap, 0, 0, 0);
	//保存单波段
	poDstDS->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, nOriImgX, nOriImgY, _pimageData, nOriImgX, nOriImgY, GDT_Byte, 0, 0);

	//释放内存
	GDALClose(poDstDS);
	GDALClose(poDataset);
	delete[]_pNewValue;
	delete[]_pimageData;
}
