//共享内存类实现函数                                                 
//何纯玉 开发
//最终更新：2016.1.25

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <QString>

#include "ShareMem.h"

ShareMem::ShareMem()
{
	m_isMemoryMap=false; 
	lpData=NULL;
	hRecvMap=NULL;

}

ShareMem::ShareMem(char *pName,long sizes)
{
	m_isMemoryMap=false; 
	lpData=NULL;
	hRecvMap=NULL;

	SetShareMem(pName,sizes);
}

int ShareMem::SetShareMem(char *pName,long sizes)
{
    strcpy(share_name,pName);
    mem_sizes=sizes;

	//如果已经建立，先释放
	if(m_isMemoryMap)
	{
		if (lpData != NULL)
		{
			UnmapViewOfFile(lpData);
			lpData = NULL;
		}
		if (hRecvMap!=NULL)
		{
			CloseHandle(hRecvMap);
			hRecvMap = NULL;
		}
		m_isMemoryMap=false; 
	}

	//建立共享内存
    //适应字符处理
    QString str;
    str=pName;

    hRecvMap=::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, sizes, (LPCWSTR)str.utf16());
	if (hRecvMap!=NULL)
	{
		lpData=(LPBYTE)MapViewOfFile(hRecvMap,FILE_MAP_WRITE,0,0,0);
		if (lpData==NULL)
		{
			CloseHandle(hRecvMap);
			hRecvMap=NULL;
			return FUNC_ERROR;
		}
		else
		{
			m_isMemoryMap=true;
			return FUNC_OK;
		}
	}
	else
		return FUNC_ERROR;

}

void * ShareMem::GetPointer(void)
{
	return lpData;
}

ShareMem::~ShareMem()
{

	//memory mapping quit process
	if (lpData != NULL)
	{
		UnmapViewOfFile(lpData);
		lpData = NULL;
	}
	if (hRecvMap!=NULL)
	{
		CloseHandle(hRecvMap);
		hRecvMap = NULL;
	}
	m_isMemoryMap=false; 
}

void ShareMem::SetShareTagData(char *pData,int start,int sizes)
{

	//复制到共享内存
	memcpy((char *)lpData+ start, (char *)pData, sizes);
}

void ShareMem::ReadFromShareMem(void *pData)
{
    memcpy((char *)pData, (char *)lpData, mem_sizes);
}

void ShareMem::WriteToShareMem(void *pData)
{
    memcpy((char *)lpData,(char *)pData, mem_sizes);
}

