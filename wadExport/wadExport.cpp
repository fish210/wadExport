#include "wadExport.h"

vector<Wad_Struct> vec;
map <CString, CString> mp;
CString wad_filePath;//WAD�ļ�·��
UINT32 wad_count;//WAD�ļ�����

int main()
{
	//*********************************************************** Init
	HMODULE hdll;
	hdll = LoadLibraryA("libzstd.dll");
	if (hdll == 0)
	{
		cout << "load error, not libzstd.dll ; ģ�����ʧ�� libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_getDecSiz = (ZSTD_getDecompressedSize)GetProcAddress(hdll, "ZSTD_getDecompressedSize");
	if (ZSTD_getDecSiz == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; �Ҳ������� ZSTD_getDecompressedSize\n" << endl;
		return 0;
	}
	ZSTD_dec = (ZSTD_decompress)GetProcAddress(hdll, "ZSTD_decompress");//ZSTD_decompress
	if (ZSTD_dec == 0)
	{		
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; �Ҳ������� ZSTD_decompress\n" << endl;
		return 0;
	}
	//*********************************************************** Start
	bool hash = load_hashes("hashes.game.txt");
	if (hash == true)
	{
		int err = openfile("Map22LEVELS.wad.client");//���ļ�
		if (err == 0)
		{
			CreateDirectory("new", NULL);//����Ŀ¼
			int err2 = export_all("new");//�����ļ�
		}
		if (err == 0)
		{
			cout << "success ; �����ɹ�\n" << endl;
		}
		else
		{
			cout << "failure ; ����ʧ��\n" << endl;
		}
	}
	else
	{
		cout << "load hash err : hash file last line is Enter(0x0A) ? ; ���ع�ϣʧ�ܣ���ϣ�ļ��쳣��������һ���Ƿ��з�\n" << endl;
	}
	return 0;
}
//��WAD�ļ�
int openfile(const char* in_filePath)
{
	ifstream in(in_filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "open file error ; ��wadʧ�ܣ������ڻ�ռ��" << endl;
		return 1;
	}
	int DataOffset = 0;//����ƫ��	
	UINT32 ver_header = 0;//�汾ͷ
	UINT32 ver30 = 0x00035752;
	UINT32 ver31 = 0x01035752;

	in.read((char*)&ver_header, 4);
	if (ver_header == ver31)//�ж��ļ��汾
	{
		DataOffset = 268;
	}
	else
	{
		in.close();
		cout << "wadfile not version 3.1 ; ��ȡʧ�ܣ�����3.1�汾" << endl;
		return 2;
	}
	wad_filePath = in_filePath;
	in.seekg(0, ios::beg);
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();//ȡ�ļ���С
	//cout << "fileSize:" << fileSize << endl;
	in.seekg(DataOffset, ios::beg);
	in.read((char*)&wad_count, 4);//ȡ�ļ�����
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
			UINT32 Offset = tem_DataOffset + 32 * i;//����ƫ��ֵ
			in.seekg(Offset, ios::beg);
		}
		UINT64 xxhash = 0;
		in.read((char*)&xxhash, 8);//��Ŀ��	

		char xxhash_tem[64];
		sprintf_s(xxhash_tem, 64, "%016llx", xxhash);
		CString xxhash_str = xxhash_tem;

		UINT32 DataOffset = 0;
		in.read((char*)&DataOffset, 4);//����ƫ��

		UINT32 Compressed = 0;
		in.read((char*)&Compressed, 4);//ѹ����С

		UINT32 Uncompressed = 0;
		in.read((char*)&Uncompressed, 4);//δѹ����С

		UINT8 Ctype = 0;
		in.read((char*)&Ctype, 1);//ѹ������
		Wad_Struct* p = new Wad_Struct();
		p->xxHash_str = xxhash_str;//16�����ļ���

		CString mp_name;
		if (mp.count(xxhash_str) != 0)
		{
			mp_name = mp[xxhash_str];//���map���������ھ�ʹ�ù�ϣ����ļ���
		}
		else
		{
			mp_name = xxhash_str;//��������ھ���Ҫ�ı���ϣ��
		}

		if (mp_name.GetLength() <= MAX_PATH / 2)//����ļ�������130���ֽڣ��ļ���ȡ16λ�ı���ϣ
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
//����ȫ���ļ�
int export_all(const char* out_directory)
{
	for (vector <Wad_Struct> ::iterator it = vec.begin(); it != vec.end(); it++)//��������
	{
		CString tem = out_directory;
		CString FileName = tem + "\\" + it->FileName;//Ŀ¼������Ŀ��
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
		in.seekg(DataOffset, ios::beg);//��ȡ����
		in.read(c_buf, bufSize);
		in.close();

		CString FileName_tem = FileName;
		FileName_tem.Replace("/", "\\");//ת��б��
		char* Path = FileName_tem.GetBuffer(0);//תΪchar*
		FileName_tem.ReleaseBuffer();
		CString p1 = Path;
		
		char* Name = PathFindFileNameA(Path);//ȡ���ļ���
		PathRemoveFileSpecA(Path);//ɾ���ļ���

		CString xg = "\\";
		CString Path2 = Path + xg;//������б��
		MakeSureDirectoryPathExists(Path2);//�����༶Ŀ¼

		size_t retSize = ZSTD_getDecSiz(c_buf, bufSize);
		char* dec_buf = new char[retSize]();
		size_t decSize = ZSTD_dec(dec_buf, retSize, c_buf, bufSize);//��ѹ����

		ofstream out(p1, ios::binary);
		out.write(dec_buf, decSize);//д���ļ�
		out.close();

		delete[] dec_buf;
		dec_buf = 0;
		delete[] c_buf;
		c_buf = 0;
	}
	return 0;
}
//���ع�ϣ��
bool load_hashes(const char* filePath)
{
	ifstream in(filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "load hashes error ��ȡ��ϣ�ļ�����" << endl;
		return false;
	}
	in.seekg(0, ios::end);//ȡ�ļ���С
	long long fileSize = in.tellg();
	in.seekg(0, ios::beg);
	char c_lineBuf[4096];//��һ����������һ�е�����
	CString s_line, s_Left, s_Right;
	in.seekg(0, ios::beg);
	while (!in.eof())
	{
		in.getline(c_lineBuf, sizeof(c_lineBuf));
		s_line = c_lineBuf;
		s_Left = s_line.Left(16);//��ϣ�����߲���
		s_Left.MakeLower();//תСд
		//cout << s_Left << endl;
		s_Right = s_line.Right(s_line.GetLength() - 17);//��ϣ����ұ߲���		
		s_Right.Replace("\r\n", "");//ȥ�� Windos linux ���ֻ��з�
		s_Right.Replace("\r", "");
		//cout << s_Right << endl;
		mp.insert(pair<CString, CString>(s_Left, s_Right));//�ѹ�ϣ���������ζ���map��������
	}
	in.close();//�ر��ļ�
	return true;
}