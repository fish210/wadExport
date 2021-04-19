#include "wadExport.h"

vector<Wad_Struct> v;
map <CString, CString> mp;

UINT32 wad_count;//WAD�ļ�����
CString wad_filePath;//WAD�ļ�·��

int main()
{
	//*********************************************************** Init
	HMODULE hdll;
	hdll = LoadLibraryA("libzstd.dll");
	if (hdll == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not libzstd.dll ; ģ�����ʧ�� libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_getDecSiz = (ZSTD_getDecompressedSize)GetProcAddress(hdll,"ZSTD_getDecompressedSize");
	if (ZSTD_getDecSiz == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; �Ҳ������� ZSTD_getDecompressedSize\n" << endl;
		return 0;
	}
	ZSTD_dec = (ZSTD_decompress)GetProcAddress(hdll,"ZSTD_decompress");//ZSTD_decompress
	if (ZSTD_dec == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_decompress ; �Ҳ������� ZSTD_decompress\n" << endl;
		return 0;
	}

	//*********************************************************** Start
	cout << "Are you ready\n" << endl;

	load_hashes("hashes.txt");

	int err = openfile("Map22LEVELS.wad.client");//���ļ�

	if (err == 0)
	{
		CreateDirectory("new", NULL);//����Ŀ¼
		int err2 = export_all("new");//�����ļ�
	}else
	{
		cout << err << endl;
	}
	return 0;
}

int openfile(const char * in_filePath)//��WAD�ļ�
{
	ifstream in(in_filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "Open error The file is useing ; ��ʧ�ܣ�WAD�ļ���ռ��" << endl;
		return 1;
	}
		
	int v_DataOffset = 0;//����ƫ��	
	UINT32 ver_header = 0;//�汾ͷ
	UINT32 ver30 = 0x00035752; 	

	in.read((char*)&ver_header, 4);
	if (ver_header == ver30)//�ж��ļ��汾
	{
		v_DataOffset = 268;
	}else
	{
		in.close();
		cout << "Read error no is version 3.0 ; ��ȡʧ�ܣ��ļ�����3.0�汾" << endl;
		return 2;
	}

	wad_filePath = in_filePath;
	in.seekg(0, ios::beg);
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();//ȡ�ļ���С
	//cout << "fileSize:" << fileSize << endl;
	
	if (fileSize > 1024*1024*200)
	{
		in.close();
		cout << "fileSize error of bounds 200mb ; ���� �ļ�����200mb ����Ԥ��" << endl;
		return 3;
	}

	in.seekg(v_DataOffset, ios::beg);
	in.read((char*)&wad_count, 4);//ȡ�ļ�����
	//cout << "wad_count:" << wad_count << endl;

	for (UINT32 i = 0; i < wad_count; i++)
	{
		if (i == 0)
		{
			in.seekg(v_DataOffset + 4 , ios::beg);
		}
		else
		{
			UINT32 Offset = (v_DataOffset + 4) + 32*i;//����ƫ��ֵ
			in.seekg(Offset, ios::beg);
		}

		UINT64 xxhash = 0;				
		in.read((char*)&xxhash, 8);//��Ŀ��	
		char xxhash_tem[64];
		sprintf_s(xxhash_tem,64, "%016llx", xxhash);
		CString xxhash_str = xxhash_tem;		

		UINT32 DataOffset = 0;
		in.read((char*)&DataOffset, 4);//����ƫ��

		UINT32 Compressed = 0;
		in.read((char*)&Compressed, 4);//ѹ����С

		UINT32 Uncompressed = 0;
		in.read((char*)&Uncompressed, 4);//δѹ����С

		UINT8 Ctype = 0;
		in.read((char*)&Ctype, 1);//ѹ������

		//**********************************************************************
		Wad_Struct *p = new Wad_Struct();
		p->xxHash_str = xxhash_str;//16�����ļ���

		CString mp_name;
		if(mp.count(xxhash_str) != 0)
		{
			mp_name = mp[xxhash_str];//���map���������ھ�ʹ�ù�ϣ����ļ���
		}else
		{
			mp_name = xxhash_str;//��������ھ���Ҫ�ı���ϣ��
		}	

		if (mp_name.GetLength() <= MAX_PATH/2)//����ļ�������130���ֽڣ��ļ���ȡ16λ�ı���ϣ
		{
			p->FileName = mp_name;	
		}else
		{
			p->FileName = xxhash_str;
		}
		
		p->DataOffset = DataOffset;
		p->CompressedSize = Compressed;
		p->UncompressedSize = Uncompressed;
		p->Type = Ctype;
		//cout << "��Ŀ��:"<< p->FileName << endl;
		v.push_back(*p);
		delete p;		
	}
	in.close();
	return 0;
}

//����ȫ���ļ�
int export_all(const char * out_directory)
{	
	for (vector <Wad_Struct> ::iterator it = v.begin(); it != v.end(); it++)//��������
	{
		CString tem = out_directory;
		CString FileName = tem + "\\" + it->FileName;//Ŀ¼������Ŀ��
		UINT8 Type = it->Type;
		UINT32 DataOffset = it->DataOffset;
		UINT32 c = it->CompressedSize;
		UINT32 u = it->UncompressedSize;

		UINT32 bufSize;
		char *c_buf = 0;
		if (Type == 3) 
		{
			c_buf = new char[c]();
			bufSize = c;					
		}else if (Type == 1)
		{
			c_buf = new char[c]();
			bufSize = c;
		}else if (Type == 0)
		{
			c_buf = new char[u]();
			bufSize = u;
		}else
		{
			continue;
		}	

		ifstream in(wad_filePath, ios::binary);
		in.seekg(DataOffset, ios::beg);//��ȡ����
		in.read(c_buf, bufSize);
		in.close();

		CString FileName_tem = FileName;
		FileName_tem.Replace("/", "\\" );//ת��б��
		//cout <<FileName_tem + "8" << endl;

		char *Path = FileName_tem.GetBuffer(0);//תΪchar*
		
		CString p1 = Path ;		
		FileName_tem.ReleaseBuffer();

		char* Name =PathFindFileNameA(Path);//ȡ���ļ���
		PathRemoveFileSpecA(Path);//ɾ���ļ���
		PathAddBackslashA(Path);//������б��
			
		MakeSureDirectoryPathExists(Path);//�����༶Ŀ¼

		size_t retSize = ZSTD_getDecSiz(c_buf, bufSize);
		char* dec_buf = new char[retSize]();
		size_t decSize = ZSTD_dec(dec_buf, retSize,c_buf, bufSize);//��ѹ����

		ofstream out(p1, ios::binary);
		out.write(dec_buf, decSize);//д���ļ�
		out.close();
		
		delete dec_buf;
		delete c_buf;
	}
	return 0;
}

//���ع�ϣ��
bool load_hashes(const char * filePath)
{	       	
	ifstream in(filePath, ios::binary);
	if (!in.is_open())
	{
		cout << "hashes load error ��ȡʧ��" << endl;
		return false;
	}	
	//ȡ�ļ���С
	in.seekg(0, ios::end);
	long long fileSize = in.tellg();
	in.seekg(0,ios::beg);

	//��̬�����ڴ棬�����ռ����ݳ�ʼ��Ϊ��
	char* c_buf = new char[(unsigned int)fileSize]();

	//����������ѡ��ǰ�����ı��ķ�ʽһ��һ�еĶ���ȥ�������ȶ��������ļ���Ȼ�����ڴ����洦����ѡ����ߣ������ȽϿ죩
	in.read(c_buf, fileSize);
	
	char c_lineBuf[204800];	//��һ����������һ�е����ݣ�200kb 20�����ֽڣ�һ�㲻����ڳ���������ȵ���
	CString s_line, s_Left, s_Right;
	in.seekg(0,ios::beg);
	while (!in.eof())
	{
		in.getline(c_lineBuf,sizeof(c_lineBuf));	
		s_line = c_lineBuf;
		//cout << s_line << endl;
		s_Left = s_line.Left(16);//��ϣ�����߲���
		s_Left.MakeLower();//תСд
		//cout << s_Left << endl;
		s_Right = s_line.Right(s_line.GetLength() - 17);//��ϣ����ұ߲���					
		s_Right.Replace("\n", "");//ȥ�� Windos linux ���ֻ��з�
		s_Right.Replace("\r", "");
		s_Right.Replace("\r\n", "");
		//cout << s_Right <<"8"<< endl;
		mp.insert(pair<CString, CString>(s_Left, s_Right));//�ѹ�ϣ���������ζ���map��������
	}	
	in.close();//�ر��ļ�
	delete c_buf;//�ͷ��ڴ�
	return true;
}