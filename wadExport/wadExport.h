#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <imagehlp.h>  
#pragma comment(lib,"imagehlp.lib") 

#include <atlstr.h> 
//MFC CString File header 
//Ϊʲôʹ��MFC�����ִ����࣬�ҵ�ǰ���ǰ���MFCΪ�˸��Ӽ��ݣ����ܶ�GCCƽ̨���Ѻã�����VC
// VC++11 test ����δ���Ż� ��ʵ��ʹ���ֿ��ܻ����Ч�ʺ��� �������� ��Ҫ�Լ��Ż�

using namespace std;

//zstd��������-��̬����
typedef size_t (*ZSTD_getDecompressedSize)(const void* src, size_t srcSize);
typedef size_t (*ZSTD_decompress)( void* dst, size_t dstCapacity,const void* src, size_t compressedSize);
ZSTD_getDecompressedSize ZSTD_getDecSiz;
ZSTD_decompress ZSTD_dec;

//���ع�ϣ��
bool load_hashes(const char * in_filePath);

//��WAD�ļ�
int openfile(const char * in_filePath);

//����ȫ���ļ�
int export_all(const char * out_directory);

//WAD���ݽṹ
struct Wad_Struct
{
    CString FileName;//��Ŀ�ļ���
	CString xxHash_str;//��Ŀ�ı���ϣ
	unsigned int DataOffset;//����ƫ��
	unsigned int CompressedSize;//ѹ�����С
	unsigned int UncompressedSize;//ѹ��ǰ��С
	unsigned char Type;//��Ŀ����   
};
