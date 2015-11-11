// Zimg.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h" 
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


int WINAPI CreateGuid(LPWSTR outGuid) 
{	 
	 int isOk = 0;
	 CoInitializeEx(NULL,COINIT_MULTITHREADED);
     //WCHAR* buf = NULL;
	 GUID guid;	
	 //CoInitialize(NULL);_T("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"),
	 if (S_OK == ::CoCreateGuid(&guid))
	 {	 
		//buf = new WCHAR[34];
	    memset(outGuid,0,34);
	 	swprintf_s(outGuid, 34,\
	 		_T("%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"),\
	 		guid.Data1,\
	 		guid.Data2,\
	 		guid.Data3,\
	 		guid.Data4[0],\
	 		guid.Data4[1],\
	 		guid.Data4[2],\
	 		guid.Data4[3],\
	 		guid.Data4[4],\
	 		guid.Data4[5],\
	 		guid.Data4[6], \
	 		guid.Data4[7]);
		isOk = 1;
	 }		  
	 ::CoUninitialize();
	 return isOk;
}//CreateGuid



 GdiplusStartupInput m_gdiplusStartupInput = NULL;
 ULONG_PTR m_pGdiToken = NULL;
/* format 取值为(只能取其一)： 
   image/bmp 
   image/jpeg 
   image/gif 
   image/tiff 
   image/png 
  */
 //读出系统中的图片编码信息 Clsid
 int WINAPI GetEncoderClsid(const WCHAR* format, CLSID* pClsid )
 {	
	  UINT  num = 0;          // number of image encoders
	  UINT  size = 0;         // size of the image encoder array in bytes  
	  ImageCodecInfo* pImageCodecInfo = NULL; 	  
	  GetImageEncodersSize(&num, &size);	 
	  if(size == 0)
	  {
		  return -1;  // Failure
	  } 	 
	  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	  if(pImageCodecInfo == NULL)
	  {
		  return -1;  // Failure
	  }
	  GetImageEncoders(num, size, pImageCodecInfo);	   
	  for(UINT j = 0; j < num; ++j)
	  {
		  if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		  {

			  *pClsid = pImageCodecInfo[j].Clsid;
			  if(pImageCodecInfo != NULL)
			  {
				 free(pImageCodecInfo);
			  }
			  return j;  // Success
		  }   
	  }	  
	  if(pImageCodecInfo != NULL)
	  {
	     free(pImageCodecInfo);
	  }
	  return -1;  // Failure  

  }


 //初始化Gdi+
 void WINAPI initGdis()
 {
	  if(m_pGdiToken == NULL)
	  {
       	  Gdiplus::GdiplusStartup(&m_pGdiToken,&m_gdiplusStartupInput,NULL); 
	  }
 }


 //释放Gdi+ 
 void WINAPI freeGdis()
 {
	  if(m_pGdiToken != NULL)
	  {
	     Gdiplus::GdiplusShutdown(m_pGdiToken);	
		 m_gdiplusStartupInput = NULL;
		 m_pGdiToken = NULL;
		 //MessageBox(NULL,L"已经释放",L"呵呵",0);
	  }
 }


 //释放Guid
 void WINAPI freeGuid(LPWSTR outpath)
 {
	  
	  if(outpath != NULL)
	  {
		 MessageBox(NULL,outpath,outpath,0);
	     delete outpath;
	  }
	  outpath = NULL;
 }

 //压缩委托-是否提前取消
 //BOOL  CALLBACK  imageAbort(VOID *)
 //{
   // return TRUE;
 //}
 //#define DLLEXPORT extern "C" __declspec(dllexport)
 //压缩图片
 int WINAPI zImg(LPWSTR  srcPath,LPWSTR outDir,LPWSTR fileGuid,int imgFormat ,int w,int h,int q)
 {		 
	int isOk = 0;   
	if(srcPath != NULL)
	{
		initGdis();	
		//Bitmap  image1(srcPath,0);
		Image srcImage(srcPath,0);
		int mHeight = srcImage.GetHeight();  
		int mWidth = srcImage.GetWidth();  
		int mThumbWidth = w;  
		int mThumbHeight = h;  
		if(mHeight > 0 && mWidth > 0)
		{
			if(mWidth > mHeight)
			{
				if(w > mWidth)
				{
					mThumbWidth = mWidth;
				}
				mThumbHeight = (int)(mThumbWidth * ((float)mHeight / mWidth));		
				if(mThumbHeight > h)
				{
				   h = mThumbHeight;
				}
			}else
			{
				if(h > mHeight)
				{
					mThumbHeight = mHeight;
				}
				mThumbWidth = (int)(mThumbHeight * ((float)mWidth / mHeight));
				if(mThumbWidth > w)
				{
				   w = mThumbWidth;
				}
			}
			Bitmap  bitmap(mThumbWidth, mThumbHeight,PixelFormat32bppARGB);  
			//Image* image_smll = srcImage.GetThumbnailImage(mThumbWidth,mThumbHeight,imageAbort);			
			Graphics graph(&bitmap);  			
			//graph.Save();			 
			CLSID bmpClsid;
		 	WCHAR*  imgType = _T("image/jpeg");
			WCHAR* img_File_Suffix = _T(".jpg");
			WCHAR* imgSuffix = NULL;
			if(imgFormat == 0) //自动
			{			   
			   imgSuffix = wcschr(srcPath,L'.');
			   if(imgSuffix != NULL)
			   {				  
				   _wcslwr_s(imgSuffix,wcslen(imgSuffix)+1);	
			   }				 
			}
			if(imgFormat == 1 || (imgSuffix != NULL && wcscmp(imgSuffix,_T(".bmp")) == 0)) //bmp
			{
				imgType = _T("image/bmp");
				img_File_Suffix = _T(".bmp");
				graph.Clear(Color(255,255,255,255));

			}else if(imgFormat == 2 || (imgSuffix != NULL && (wcscmp(imgSuffix,_T(".jpg")) == 0 
				                                          || wcscmp(imgSuffix,_T(".jpeg")) == 0))) //jpeg 
			{
				imgType = _T("image/jpeg");
				img_File_Suffix = _T(".jpg");
				graph.Clear(Color(255,255,255,255));

			}else if(imgFormat == 3 || (imgSuffix != NULL && wcscmp(imgSuffix,_T(".gif")) == 0)) //gif
			{
				imgType = _T("image/gif");
				img_File_Suffix = _T(".gif");
				graph.Clear(Color(255,255,255,255));

			}else if(imgFormat == 4 || (imgSuffix != NULL && (wcscmp(imgSuffix,_T(".tiff")) == 0 
				                                          || wcscmp(imgSuffix,_T(".tif")) == 0))) //tiff 
			{  
				imgType = _T("image/tiff");
				img_File_Suffix = _T(".tif");

			}else if(imgFormat == 5 || (imgSuffix != NULL && wcscmp(imgSuffix,_T(".png")) == 0)) //png 
			{
				imgType = _T("image/png");
				img_File_Suffix = _T(".png");
			}
			if(imgSuffix != NULL)
			{
				 free(imgSuffix);
				 imgSuffix = NULL;
			}
			if(GetEncoderClsid(imgType, &bmpClsid) != -1)
			{
				WCHAR* out_guid  = new WCHAR[34];
				memset(out_guid,0,34);	
				if(!CreateGuid(out_guid))
				{
					delete out_guid;
					out_guid = NULL;
				}
				if(out_guid != NULL)
				{
					/************拼接GUID文件******开始********/
					int outdir_lg = wcslen(outDir);
					int outguid_lg = wcslen(out_guid);

					int out_lg = outdir_lg+outguid_lg+1+5;
					WCHAR* outPath = new WCHAR[out_lg];
					memset(outPath,0,out_lg);	
				
					wcscpy_s(outPath,outdir_lg+1,outDir);				
				
					outPath += outdir_lg;
				
					wcscpy_s(outPath,outguid_lg+1,out_guid);				

					outPath += outguid_lg;
				
					wcscpy_s(outPath,outguid_lg,img_File_Suffix);
				
					outPath -= outdir_lg+outguid_lg;
					

					/*************复制Guid到外部**************开始***********/

					memset(fileGuid,0,outguid_lg+1+5);
					wcscpy_s(fileGuid,outguid_lg+1,out_guid);	

					fileGuid += outguid_lg;
					wcscpy_s(fileGuid,5,img_File_Suffix);
					fileGuid-=outguid_lg;
					/*************复制Guid到外部***************结束**********/
					delete out_guid;
					out_guid = NULL;

					/************拼接GUID文件**********结束***********/

					graph.DrawImage(&srcImage, 0,0,mThumbWidth,mThumbHeight);			

					if(q > 0 && q < 100)
					{
						//数组中只包含一个EncoderParameter对象 
						EncoderParameters  eps; 
						eps.Count =  1;
						eps.Parameter[0].Guid = EncoderQuality; 
						//参数类型为LONG
						eps.Parameter[0].Type = EncoderParameterValueTypeLong;
						//只设置一个参数
						eps.Parameter[0].NumberOfValues = 1; 
						//设置JPEG图片质量
						ULONG   quality = q; 
						eps.Parameter[0].Value = &quality;
    					if(bitmap.Save(outPath, &bmpClsid, &eps) == Ok)
						{
							isOk = 1;
						}
						  
					}else
					{
						if(bitmap.Save(outPath, &bmpClsid, NULL) == Ok)
						{
							isOk = 1;
						}
					}
					delete outPath;
					outPath = NULL;
				}
			}
			//graph.Restore(NULL);			
			//delete image_smll;
			//image_smll = NULL;
		}
	}
	//freeGdis(); 最好不在这里调用，我发现在 pb 下会出现错误，C#下没有问题,所以freeGdis()留给调用者调用释放.
	//MessageBox(NULL,srcPath,outPath,0);
	//MessageBox(NULL,L"123456",outPath,0);
	return isOk;
 } 

typedef   void  (WINAPI *fn)(LPCWSTR);
 //DLLEXPORT
 
//http://www.gissky.net/Article/400.htm
/*void WINAPI callFunc(fn fun)
{
	 //fun(L"你好你好你好哦");
	 //PostMessage(

}*/


