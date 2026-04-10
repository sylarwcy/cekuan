//共享内存类头函数                                                 
//何纯玉 开发
//最终更新：2016.1.25

#ifndef DEF_SHARE_MEM
#define DEF_SHARE_MEM

#include <windows.h>
////////////////////////////////////////////////////////////////////////////
//函数返回值定义
#define FUNC_OK  0
#define FUNC_ERROR 1
////////////////////////////////////////////////////////////////////////////

//共享内存类定义
class ShareMem
{
public:
	HANDLE hRecvMap;
	void *lpData; 
	char share_name[32];
	long mem_sizes;
	bool m_isMemoryMap;

public:
	ShareMem();
	ShareMem(char *pName,long sizes);
	~ShareMem();
	
	int SetShareMem(char *pName,long sizes);
	void * GetPointer(void);

	void SetShareTagData(char *pData, int start, int sizes);

    void ReadFromShareMem(void *pData);
    void WriteToShareMem(void *pData);
};

#endif
