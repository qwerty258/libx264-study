// H264DLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
extern "C" 
{
#include <stdint.h>
#include <x264.h>
};

typedef struct
{
	int width;
	int height;
	int fps;
}paramInput;

unsigned int g_uiPTSFactor = 0;
unsigned int uiComponent = 0;
int iNal = 0;
x264_nal_t* pNals = NULL;
int iResult = 0,iDataLen=0,filesize=0;
x264_t* pX264Handle = NULL;
x264_picture_t* pPicIn ;
x264_picture_t* pPicOut;

//��ʼ��DLL����
//==0 success ��<0 false 
extern __declspec(dllexport) int InitDLL(paramInput *paramUser)
{
    x264_param_t* pX264Param = new x264_param_t;
	x264_param_default_preset(pX264Param, "ultrafast", "zerolatency");
	pX264Param->i_threads =1;
	pX264Param->i_width = paramUser->width; //* ���.
	pX264Param->i_height =paramUser->height ; //* �߶�
	pX264Param->i_keyint_min=5;
    pX264Param->i_keyint_max=2;
	//pX264Param->i_bframe_pyramid = 0;
	//pX264Param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pX264Param->i_bframe = 16;  //I��P֮���B֡�� 
	pX264Param->rc.i_bitrate = 40960;
	pX264Param->rc.i_rc_method =X264_RC_CRF;
	//pX264Param->b_intra_refresh = 1;
	pX264Param->i_fps_den = 1; //* ֡�ʷ�ĸ
	pX264Param->i_fps_num = paramUser->fps; //* ֡�ʷ���
	x264_param_apply_profile(pX264Param, x264_profile_names[0]);
	//* �򿪱��������,ͨ��x264_encoder_parameters�õ����ø�X264
	//* �Ĳ���.ͨ��x264_encoder_reconfig����X264�Ĳ���
	pX264Handle = x264_encoder_open(pX264Param);
	if (pX264Handle==NULL)
	{
		return -1;
	}
	//* ��ȡ��������PPS��SPS,����Ҫ���Բ�����.
	iResult = x264_encoder_headers(pX264Handle, &pNals, &iNal);
	if (iResult<0)
	{
		return -2;
	}
	//* ��ȡ����������֡��.
	int iMaxFrames = x264_encoder_maximum_delayed_frames(pX264Handle);
	//* ������Ҫ�Ĳ���.
    pPicIn = new x264_picture_t;
    pPicOut = new x264_picture_t;
	x264_picture_init(pPicOut);
	x264_picture_alloc(pPicIn, X264_CSP_I420, pX264Param->i_width,
	pX264Param->i_height);
	pPicIn->img.i_csp = X264_CSP_I420;
	pPicIn->img.i_plane = 3;
	pPicIn->i_type= X264_TYPE_AUTO;
	BYTE *bufptr;
	//* �����ļ�,���ڴ洢��������
	//* ʾ���ñ�������.
	iDataLen = pX264Param->i_width * pX264Param->i_height;
	filesize=pX264Param->i_width * pX264Param->i_height*1.5;
	return 0;
}

extern __declspec(dllexport) int EncodeBuf(BYTE *inBuf,int inBufsize,int picType,BYTE *outBuf)
{
     if(inBufsize<filesize) return -1;
	 if (picType==0)  //yuv12
	 {
	 memcpy(pPicIn->img.plane[0], inBuf,iDataLen);//YUV��Y����
	 memcpy(pPicIn->img.plane[2], inBuf+iDataLen, iDataLen / 4);//YUV��U����
	 memcpy(pPicIn->img.plane[1], inBuf+iDataLen+iDataLen / 4, iDataLen / 4);//YUV��V����  
	 }if (picType==1)  //yuv420
	 {
		 memcpy(pPicIn->img.plane[0], inBuf,iDataLen);//YUV��Y����
		 memcpy(pPicIn->img.plane[1], inBuf+iDataLen, iDataLen / 4);//YUV��U����
		 memcpy(pPicIn->img.plane[2], inBuf+iDataLen+iDataLen / 4, iDataLen / 4);//YUV��V����
	 }
	 //��ʼ���� 	
	if (uiComponent <= 1000) 
	{
		  pPicIn->i_pts = uiComponent + g_uiPTSFactor * 1000;
		  x264_encoder_encode(pX264Handle, &pNals, &iNal, pPicIn, pPicOut);
	} else 
	{
		int iResult = x264_encoder_encode(pX264Handle, &pNals, &iNal, NULL, pPicOut);
		if (0 == iResult) 
		{
		//break; //* ȡ��,����
		uiComponent = 0;
		++g_uiPTSFactor;
		}
		uiComponent++;
    }
	//����������д�뻺��
	int outBufsize=0;
	BYTE *bufptr=(BYTE *)outBuf;
    for (int i=0;i<iNal;++i)
	{
		memcpy(bufptr,pNals[i].p_payload,pNals[i].i_payload);
		bufptr +=pNals[i].i_payload;
		outBufsize+=pNals[i].i_payload;
	}
	return outBufsize;
}

extern __declspec(dllexport) void ClearDLL(void)
{
  x264_picture_clean(pPicIn);
  x264_picture_clean(pPicOut);
}