// H264DLL.cpp : Defines the exported functions for the DLL application.
//


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    typedef struct
    {
        int width;
        int height;
        int fps;
    }paramInput;

    //初始化DLL参数
    //==0 success ，<0 false 
    int InitDLL(paramInput* paramUser);
    int EncodeBuf(uint8_t* inBuf, int inBufsize, int picType, uint8_t* outBuf);
    void ClearDLL(void);

#ifdef __cplusplus
}
#endif
