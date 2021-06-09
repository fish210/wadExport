#include "wadExport.h"

vector<Wad_Struct> vec;
map <CString, CString> mp;
CString wad_filePath;//WAD文件路径
UINT32 wad_count;//WAD文件数量

int main()
{
	//*********************************************************** Init
	HMODULE hdll;
	hdll = LoadLibraryA("libzstd.dll");
	if (hdll == 0)
	{
		cout << "load error, not libzstd.dll ; 模块加载失败 libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_getDecSiz = (ZSTD_getDecompressedSize)GetProcAddress(hdll, "ZSTD_getDecompressedSize");
	if (ZSTD_getDecSiz == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; 找不到函数 ZSTD_getDecompressedSize\n" << endl;
		return 0;
	}
	ZSTD_dec = (ZSTD_decompress)GetProcAddress(hdll, "ZSTD_decompress");//ZSTD_decompress
	if (ZSTD_dec == 0)
	{		
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; 找不到函数 ZSTD_decompress\n" << endl;
		return 0;
	}
	//*********************************************************** Start
	cout << "Are you ready\n" << endl;
	bool hash = load_hashes("hashes.game.txt");
	if (hash == true)
	{
		int err = openfile("Map22LEVELS.wad.client");//打开文件
		if (err == 0)
		{
			CreateDirectory("new", NULL);//创建目录
			int err2 = export_all("new");//导出文件
		}
		if (err == 0)
		{
			cout << "success ; 导出成功\n" << endl;
		}
		else
		{
			cout << "failure ; 导出失败\n" << endl;
		}
	}
	else
	{
		cout << "load hash err : hash file last line is Enter(0x0A) ? ; 加载哈希失败，哈希文件异常，检查最后一行是否换行符\n" << endl;
	}
	return 0;
}
//打开WAD文件
int openfile(const char* in_filePath)
{
	ifstream in(in_filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "open file error ; 打开wad失败，不存在或被占用" << endl;
		return 1;
	}
	int DataOffset = 0;//数据偏移	
	UINT32 ver_header = 0;//版本头
	UINT32 ver30 = 0x00035752;
	UINT32 ver31 = 0x01035752;

	in.read((char*)&ver_header, 4);
	if (ver_header == ver31)//判断文件版本
	{
		DataOffset = 268;
	}
	else
	{
		in.close();
		cout << "file no is version 3.1 ; 读取失败，不是3.1版本" << endl;
		return 2;
	}
	wad_filePath = in_filePath;
	in.seekg(0, ios::beg);
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();//取文件大小
	//cout << "fileSize:" << fileSize << endl;
	in.seekg(DataOffset, ios::beg);
	in.read((char*)&wad_count, 4);//取文件数量
	//cout << "wad_count:" << wad_count << endl;

	int tem_DataOffset = DataOffset + 4;
	for (UINT32 i = 0; i < wad_count; i++)
	{
		if (i == 0)
		{
			in.seekg(tem_DataOffset, ios::beg);
		}
		else
		{
			UINT32 Offset = tem_DataOffset + 32 * i;//设置偏移值
			in.seekg(Offset, ios::beg);
		}
		UINT64 xxhash = 0;
		in.read((char*)&xxhash, 8);//条目名	

		char xxhash_tem[64];
		sprintf_s(xxhash_tem, 64, "%016llx", xxhash);
		CString xxhash_str = xxhash_tem;

		UINT32 DataOffset = 0;
		in.read((char*)&DataOffset, 4);//数据偏移

		UINT32 Compressed = 0;
		in.read((char*)&Compressed, 4);//压缩大小

		UINT32 Uncompressed = 0;
		in.read((char*)&Uncompressed, 4);//未压缩大小

		UINT8 Ctype = 0;
		in.read((char*)&Ctype, 1);//压缩类型
		Wad_Struct* p = new Wad_Struct();
		p->xxHash_str = xxhash_str;//16进制文件名

		CString mp_name;
		if (mp.count(xxhash_str) != 0)
		{
			mp_name = mp[xxhash_str];//如果map容器键存在就使用哈希表的文件名
		}
		else
		{
			mp_name = xxhash_str;//如果不存在就是要文本哈希名
		}

		if (mp_name.GetLength() <= MAX_PATH / 2)//如果文件名超过130个字节，文件名取16位文本哈希
		{
			p->FileName = mp_name;
		}
		else
		{
			p->FileName = xxhash_str;
		}
		p->DataOffset = DataOffset;
		p->CompressedSize = Compressed;
		p->UncompressedSize = Uncompressed;
		p->Type = Ctype;
		vec.push_back(*p);
		delete p;
	}
	in.close();
	return 0;
}
//导出全部文件
int export_all(const char* out_directory)
{
	for (vector <Wad_Struct> ::iterator it = vec.begin(); it != vec.end(); it++)//历遍容器
	{
		CString tem = out_directory;
		CString FileName = tem + "\\" + it->FileName;//目录加上条目名
		UINT8 Type = it->Type;
		UINT32 DataOffset = it->DataOffset;
		UINT32 c = it->CompressedSize;
		UINT32 u = it->UncompressedSize;

		UINT32 bufSize;
		char* c_buf = 0;
		if (Type == 3)
		{
			c_buf = new char[c]();
			bufSize = c;
		}
		else if (Type == 1)
		{
			c_buf = new char[c]();
			bufSize = c;
		}
		else if (Type == 0)
		{
			c_buf = new char[u]();
			bufSize = u;
		}
		else
		{
			continue;
		}
		ifstream in(wad_filePath, ios::binary);
		in.seekg(DataOffset, ios::beg);//读取数据
		in.read(c_buf, bufSize);
		in.close();

		CString FileName_tem = FileName;
		FileName_tem.Replace("/", "\\");//转换斜杠
		char* Path = FileName_tem.GetBuffer(0);//转为char*
		FileName_tem.ReleaseBuffer();
		CString p1 = Path;
		
		char* Name = PathFindFileNameA(Path);//取出文件名
		PathRemoveFileSpecA(Path);//删除文件名

		CString xg = "\\";
		CString Path2 = Path + xg;//加上右斜杠
		MakeSureDirectoryPathExists(Path2);//创建多级目录

		size_t retSize = ZSTD_getDecSiz(c_buf, bufSize);
		char* dec_buf = new char[retSize]();
		size_t decSize = ZSTD_dec(dec_buf, retSize, c_buf, bufSize);//解压数据

		ofstream out(p1, ios::binary);
		out.write(dec_buf, decSize);//写出文件
		out.close();

		delete[] dec_buf;
		dec_buf = 0;
		delete[] c_buf;
		c_buf = 0;
	}
	return 0;
}
//加载哈希表
bool load_hashes(const char* filePath)
{
	ifstream in(filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "load hashes error 读取哈希文件错误" << endl;
		return false;
	}
	in.seekg(0, ios::end);//取文件大小
	long long fileSize = in.tellg();
	in.seekg(0, ios::beg);
	char c_lineBuf[4096];//用一个变量保存一行的内容
	CString s_line, s_Left, s_Right;
	in.seekg(0, ios::beg);
	while (!in.eof())
	{
		in.getline(c_lineBuf, sizeof(c_lineBuf));
		s_line = c_lineBuf;
		s_Left = s_line.Left(16);//哈希表的左边部分
		s_Left.MakeLower();//转小写
		//cout << s_Left << endl;
		s_Right = s_line.Right(s_line.GetLength() - 17);//哈希表的右边部分		
		s_Right.Replace("\r\n", "");//去掉 Windos linux 各种换行符
		s_Right.Replace("\r", "");
		//cout << s_Right << endl;
		mp.insert(pair<CString, CString>(s_Left, s_Right));//把哈希表左右两段丢进map容器里面
	}
	in.close();//关闭文件
	return true;
}
