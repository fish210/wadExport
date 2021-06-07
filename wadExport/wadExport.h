#pragma once
#include <Windows.h>
#include <fstream>
#include <vector>
#include <map>
#include <imagehlp.h>  
#pragma comment(lib,"imagehlp.lib") 
#include <atlstr.h>//MFC CString File Header 
#include <iostream>
using namespace std;

//zstd函数声明-动态加载
typedef size_t(*ZSTD_getDecompressedSize)(const void* src, size_t srcSize);
typedef size_t(*ZSTD_decompress)(void* dst, size_t dstCapacity, const void* src, size_t compressedSize);
ZSTD_getDecompressedSize ZSTD_getDecSiz;
ZSTD_decompress ZSTD_dec;

//加载哈希表
bool load_hashes(const char* in_filePath);

//打开WAD文件
int openfile(const char* in_filePath);

//导出全部文件
int export_all(const char* out_directory);

//WAD数据结构
struct Wad_Struct
{
	CString FileName;//条目文件名
	CString xxHash_str;//条目文本哈希
	unsigned int DataOffset;//数据偏移
	unsigned int CompressedSize;//压缩后大小
	unsigned int UncompressedSize;//压缩前大小
	unsigned char Type;//条目类型   
};
