#ifndef Linux_ext2
#define Linux_ext2
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<vector>
#include<queue>
#include<cstring>
#include<string>
#include<string.h>
#include<fstream>
#include<istream>
#include<ostream>
#include<iomanip>
#include<Windows.h>
using namespace std;
typedef double db;
const unsigned int INF = 1e9 + 7;
const unsigned int root = 0;
fstream f;
ofstream fout;

const string FileName = "Linux_ext2";                    //�ļ���
const unsigned int RootDir = 0;                          //��Ŀ¼��iNode��
const unsigned int BlockSize = 1024;                     //���̿��С1024Bytes
const unsigned int iNodeSize = 128;                      //iNode��С128Bytes

const unsigned int TotalBlockNum = 102400;               //100MB�ռ���ܴ��̿���
const unsigned int BootNum = 1;			                 //������
const unsigned int SuperNum = 1;			             //������
const unsigned int GroupDesNum = 1;		                 //��������
const unsigned int BlockBpNum = 16;		                 //��λͼ
const unsigned int iNodeBpNum = 4;		                 //iNodeλͼ
const unsigned int iNodeTbNum = 4096;	                 //iNode��
const unsigned int DataBlockNum = 98281;       		     //���ݿ����

const unsigned int TotaliNodeNum = 32768;                //iNode����

const unsigned int NameLen = 32;			             //�ļ�����������
const unsigned int FileNum1 = 32768;		             //���ļ���������
const unsigned int FileNum2 = 256;		                 //ÿ��Ŀ¼�ļ��µ��ļ���������

unsigned int CurrentPath = RootDir;                      //��ǰ·�����洢iNode��ţ���ʼΪ��Ŀ¼

class BootBlock {//1block 1024B 8192b
public:
	unsigned int standby[256];
	
	BootBlock() {
		memset(standby, 0, sizeof(standby));
	}
};

class SuperBlock {//1block 1024B 8192b
public:
	unsigned int tot_block;
	unsigned int tot_datablock, use_datablock;
	unsigned int tot_inode, use_inode;
	unsigned int block_size;
	unsigned int inode_size;
	unsigned int file_num1;
	unsigned int file_num2;
	unsigned int name_len;

	unsigned int standby[246];

	SuperBlock() {
		tot_block = TotalBlockNum;
		tot_datablock = DataBlockNum;
		use_datablock = 1;//��Ŀ¼ռ��һ�����ݿ�
		tot_inode = TotaliNodeNum;
		use_inode = 1;//��Ŀ¼ռ��һ��iNode
		block_size = BlockSize;
		inode_size = iNodeSize;
		file_num1 = FileNum1;
		file_num2 = FileNum2;
		name_len = NameLen;

		memset(standby, 0, sizeof(standby));
	}
};

class GroupDescription {//1block 1024B 8192b
public:
	unsigned int group_begin, group_end;
	unsigned int super_begin, super_end;
	unsigned int groupdes_begin, groupdes_end;
	unsigned int blockbm_begin, blockbm_end;
	unsigned int inodebm_begin, inodebm_end;
	unsigned int inodetb_begin, inodetb_end;
	unsigned int data_begin, data_end;

	unsigned int standby[242];

	GroupDescription() {
		group_begin = 1; group_end = 102399;//��0�����̿���������
		super_begin = 1; super_end = 1;
		groupdes_begin = 2; groupdes_end = 2;
		blockbm_begin = 3; blockbm_end = 18;
		inodebm_begin = 19; inodebm_end = 22;
		inodetb_begin = 23; inodetb_end = 4118;
		data_begin = 4119; data_end = 102399;
	
		memset(standby, 0, sizeof(standby));
	}
};

class BlockBitmap {//16block 16384B 131072b
public:
	unsigned int use[4096];
	BlockBitmap() {
		memset(use, 0, sizeof(use));
		use[0] = 2147483648;//��Ŀ¼ռ�õ�0��bitmap
	}
};

class iNodeBitmap {//4block 4096B 32768b
public:
	unsigned int use[1024];
	iNodeBitmap() {
		memset(use, 0, sizeof(use));
		use[0] = 2147483648;//��Ŀ¼ռ�õ�0��bitmap
	}
};

class iNode {//128B 1024b
public:
	char name[NameLen];           //�ļ���
	unsigned int last_pos;        //����Ŀ¼��iNode��[0,32767]
	unsigned int next_pos;        //��һ��iNode��[0,32767]
	unsigned int type;            //type==0Ŀ¼ type==1�������ļ�
	unsigned int files_num;       //�����Ŀ¼�ļ�����¼��Ŀ¼���ж����ļ�
	unsigned int user_id;         //������id
	unsigned int mode;		      //ģʽ mode==0��д mode==1ֻ�� 
	unsigned int block_num;       //�ļ�ռ�ÿ�����Ŀ¼�ļ����ռһ������ͨ�ļ�����
	unsigned int block_pos[17];   //�����������Ŀ�� Ŀ¼�ļ����Ŀ¼�µ��ļ�iNode �������ļ�������

	iNode() {//��������Ǹ�Ŀ¼
		memset(name, '\0', sizeof(name)); name[0]='/';
		last_pos = INF;//INF��ʾ��
		next_pos = INF;
		type = 0;
		files_num = 0;
		user_id = root;
		mode = 0;
		block_num = 1;
		block_pos[0] = 0;//���ڵ�0�����ݿ���
		for (int i = 1; i < 17; i++) block_pos[i] = INF;
	}
	void clear1() {
		memset(name, '\0', sizeof(name)); name[0] = '/';
		last_pos = INF;
		next_pos = INF;
		type = 0;
		files_num = 0;
		user_id = root;
		mode = 0;
		block_num = 1;
		block_pos[0] = 0;
		for (int i = 1; i < 17; i++) block_pos[i] = INF;
	}
	void clear2() {
		memset(name, '\0', sizeof(name));
		last_pos = INF;
		next_pos = INF;
		type = 0;
		files_num = 0;
		user_id = root;
		mode = 0;
		block_num = 0;
		for (int i = 0; i < 17; i++) block_pos[i] = INF;
	}
};

class iNodeTable {//4096block 4194304B 33554432b
public:
	iNode inode[32768];
};

class Block {//1024B 8192b
public:
	unsigned int data[256];
	Block() {
		memset(data, 0, sizeof(data));
	}
	void clear() {
		memset(data, 0, sizeof(data));
	}
};

class DataBlock {//98281block 100639744B 805117952b
public:
	Block block[98281];
};

class Path {//����·���ֽ⣬�������
public:
	unsigned int cnt;
	string  ph[3000];

	Path() {
		cnt = 0;
		for (int i = 0; i < 3000; i++) ph[i] = "";
	}
	void clear() {
		cnt = 0;
		for (int i = 0; i < 3000; i++) ph[i] = "";
	}
}path;

class Order {//�����û����������ֽ⣬�����ж�
public:
	unsigned int cnt;
	string od[20];
	unsigned int type;

	Order() {
		cnt = 0;
		for (int i = 0; i < 20; i++) od[i] = "";
		type = INF;
	}
	void clear() {
		cnt = 0;
		for (int i = 0; i < 20; i++) od[i] = "";
		type = INF;
	}
}order;

BootBlock bootblock;
SuperBlock superblock;
GroupDescription groupdes;
BlockBitmap blockmp;
iNodeBitmap inodemp;
iNodeTable inodetb;

queue<Block> buffer;//������������copy<host>����
queue<unsigned int> qdirinode;//����ls����


void Run();//���г���


void FindOrder(Order& ord);
/*
	Description: ���ڻ�ȡ��������
	Input: Orderʵ��������ord���洢���������
	Output: ��
	Return: ��
*/
unsigned int FindFirstZero(unsigned int x);
/*
	Description: ��xת��Ϊ�����ƺ�ĵ�һ��0��λ��
	Input: x
	Output: ��
	Return: �������ҵ�һ��0��λ�ã�[1,32]
*/
unsigned int FindFreeBlock();
/*
	Description: �ҵ�һ�����е����ݿ飬���ڿ�λͼ��ռ�ã����ô˺���Ҫ��֤һ���п��п�
	Input: ��
	Output: ��
	Return: ���ݿ��ţ�[0,131071]
*/
unsigned int FindFreeINode();
/*
	Description: �ҵ�һ�����е�iNode������iNode����ռ��
	Input: ��
	Output: ��
	Return: iNode��ţ�[0,32767]
*/
bool FindDisk();
/*
	Description: ���ļ�ϵͳ�Ķ������ļ�
	Input: ��
	Output: ��
	Return: ����true��������false
*/
void FindAbsolutePath(string& relpath);
/*
	Description: ��·��ת��Ϊ����·��
	Input: ����·�����ַ���������·���������·��
	Output: ��
	Return: ��
*/
unsigned int FindFileINode(unsigned int nowdir, string filename);
/*
	Description: ��iNode��Ϊnowdir��Ŀ¼�£��ļ�����Ŀ¼��������ļ���Ϊfilename��iNode��
	Input: nowdir��Ŀ¼iNode�ţ�filename��Ҫ�ҵ��ļ���
	Output: ��
	Return: �ļ���Ϊfilename��iNode�ţ�INF��ʾ������
*/
unsigned int FindFileINode(string s);
/*
	Description: ���ݾ���·����iNode��
	Input: ����·�����ַ���
	Output: ��
	Return: ·���ļ���iNode�ţ�INF��ʾ·������
*/


void ReadFileSystem();
/*
	Description: ��ȡ�ļ�ϵͳ�ĳ����顢����������λͼ��iNode����Ϣ���������
	Input: ��
	Output: ��
	Return: ��
*/
void ReadBlock(unsigned int pos, Block& bk);
/*
	Description: ��ȡ�±�Ϊpos�Ĵ��̿���Ϣ
	Input: pos�Ǵ��̿��±�[0,102399]��bk��Ҫ����Ĵ��̿����
	Output: ��
	Return: ��
*/


void WriteFileSystem();
/*
	Description: �������顢������������λͼ��iNodeλͼ��iNode��д���ļ�ϵͳ
	Input: ��
	Output: ��
	Return: ��
*/
void WriteBlock(unsigned int pos, Block& bk);
/*
	Description: �����̿�bk����д���±�Ϊpos�Ĵ��̿���
	Input: pos�Ǵ��̿��±�[0,102399]��bk��Ҫд����̵�����
	Output: ��
	Return: ��
*/


void CreateFileSystem();
/*
	Description: �����ļ�ϵͳ
	Input: ��
	Output: ��
	Return: ��
*/
unsigned int CreateNewFile(unsigned int fathinode, string filename);
/*
	Description: ��iNodeΪfathinode��Ŀ¼�´������ļ�filename
	Input: fathinode��Ŀ¼iNode�ţ�filename�����ļ����������ļ������ļ���
	Output: ��
	Return: ���ļ���iNode��INF��ʾ�½�ʧ��
*/
unsigned int CreateNewDir(unsigned int fathinode, string dirname);
/*
	Description: ��iNodeΪfathinode��Ŀ¼�´�����Ŀ¼dirname
	Input: fathinode��Ŀ¼iNode�ţ�dirname��Ŀ¼��
	Output: ��
	Return: ��Ŀ¼��iNode��INF��ʾ�½�ʧ��
*/


void RemoveFileINodeBitmap(unsigned int nowinode);
/*
	Description: ��iNode��Ϊnowinode���ļ���ռ�õ�iNodeλͼȫ���ͷţ����ļ������ͷŶ��iNode��
	Input: nowinode���ļ����������ļ�����iNode��
	Output: ��
	Return: ��
*/
void RemoveFileBlockBitmap(unsigned int nowinode);
/*
	Description: ��iNode��Ϊnowinode���ļ���ռ�õĿ�λͼȫ���ͷ�
	Input: nowinode���ļ����������ļ�����iNode��
	Output: ��
	Return: ��
*/
void RemoveFileDataBlock(unsigned int nowinode);
/*
	Description: ��iNode��Ϊnowinode���ļ���ռ�õ����ݿ�ȫ���ͷ�
	Input: nowinode���ļ����������ļ�����iNode��
	Output: ��
	Return: ��
*/
void RemoveFile(unsigned int nowinode);
/*
	Description: ɾ��iNodeΪnowinode���ļ�
	Input: nowinode���ļ����������ļ�����iNode��
	Output: ��
	Return:  ��
*/
void RemoveEmptyDir(unsigned int nowinode);
/*
	Description: ɾ��iNodeΪnowinode��Ŀ¼����Ŀ¼��֤�ǿ�Ŀ¼
	Input: nowinode��Ŀ¼��iNode��
	Output: ��
	Return: ��
*/
void RemoveDir(unsigned int nowinode);//ɾ��iNodeΪnowinode��Ŀ¼
/*
	Description: ɾ��iNodeΪnowinode��Ŀ¼����Ŀ¼�¿����������ļ�
	Input: nowinode��Ŀ¼��iNode��
	Output: ��
	Return: ��
*/

void CatRead(unsigned int nowinode);
/*
	Description: ֻ����ʽ���ļ�
	Input: nowinode���ļ�iNode��
	Output: ��
	Return: ��
*/
void CatReadToHost(unsigned int nowinode, string path);
/*
	Description: ֻ����ʽ���ļ�,��д��������
	Input: nowinode���ļ�iNode�ţ�path��Ҫд�������·��
	Output: ��
	Return: ��
*/
void CatWrite(unsigned int nowinode);
/*
	Description: ׷��д��ʽ���ļ�����֮ǰ�����ݿ�û�����꣬׷�ӵ�����Ҳ�������д�����¿�һ�����̿�
	Input: nowinode���ļ�iNode��
	Output: ��
	Return: ��
*/


void CopyHostToBuffer(string hostpath);
/*
	Description: ��Ҫ�����������ļ��ŵ���������
	Input: hostpath�������ļ���·��
	Output: ��
	Return: ��
*/
void CopyBufferToLinux(unsigned int nowinode);
/*
	Description: ��������������д���ļ�ϵͳ,���ô˺���Ҫ��֤iNode�����ݿ����
	Input: nowinode��Ҫд���ļ���iNode��
	Output: ��
	Return: ��
*/
void CopyLinuxToLinux(unsigned int inode1, unsigned int inode2);
/*
	Description: �ļ�ϵͳ�ڲ��Ķ������ļ�����,���ô˺���Ҫ��֤iNode�����ݿ����
	Input: inode1�Ǳ��������ļ�iNode��inode2�ǿ������ļ�iNode
	Output: ��
	Return: ��
*/
void CopyHost(string filename, string hostpath, unsigned int dirinode);
/*
	Description: ������·��Ϊhostpath���ļ����Ƶ�iNodeΪdirinode��Ŀ¼��
	Input: filename�������ϵ��ļ�����hostpath�������ļ���·������dirnode�ļ�ϵͳ��Ҫ������ȥ��Ŀ¼��iNode��
	Output: �ɹ���ʧ�ܵ�ԭ��
	Return: ��
*/
void CopyLxfs(string filename, unsigned int fileinode, unsigned int dirinode);
/*
	Description: LinuxFileSystem���ڲ�����
	Input: filename�Ƕ������ļ�����fileinode�Ǳ������ļ���iNode�ţ�dirinode��Ҫ������ȥ��Ŀ¼��iNode��
	Output: �ɹ���ʧ�ܵ�ԭ��
	Return: ��
*/


void ChangeDir(string newpath);
/*
	Description: �ı䵱ǰ����Ŀ¼
	Input: newpath��Ҫ�ı�ɵľ���·��
	Output: �ɹ���ʧ��
	Return: ��
*/
bool Exist(unsigned int fathinode, string sonname);
/*
	Description: �ж���iNode��Ϊfathinode��Ŀ¼����û����Ϊsonname���ļ�
	Input: fathinode��Ŀ¼��iNode�ţ�sonname���ļ���Ŀ¼��������ļ�����iNode��
	Output: ��
	Return: true���ڣ�false������
*/


void ShowHelp();
/*
	Description: ������ʾ
	Input: ��
	Output: ��
	Return:��
*/
void ShowInfo();
/*
	Description: ��ʾ����ϵͳ��Ϣ
	Input: ��
	Output: ��
	Return: ��
*/
void ShowDir(unsigned int nowdir);
/*
	Description: ��ʾiNode��Ϊnowdir��Ŀ¼��Ϣ
	Input: nowdir��Ŀ¼��iNode��
	Output: Ŀ¼��Ϣ
	Return: ��
*/
void ShowDir(unsigned int nowdir, bool sonfile);
/*
	Description: ��ʾiNodeΪnowdir��Ŀ¼��Ϣ��������Ŀ¼��
	Input: nowdir��Ŀ¼��iNode�ţ�sonfile��ʾ��Ҫ��ʾ��Ŀ¼����Ϣ
	Output: Ŀ¼��Ϣ
	Return: ��
*/
void ShowList();
/*
	Description: �����ǰ�ļ�ϵͳ������Ŀ¼��������ļ���Ϣ
	Input: ��
	Output: Ŀ¼���ļ���Ϣ
	Return: ��
*/
void ShowPath();
/*
	Description: �����ǰĿ¼
	Input: ��
	Output: ��ǰĿ¼
	Return: ��
*/


void Info();
/*
	����1��info�����ʾ����ϵͳ��Ϣ
	��ʽ��info
	˵������
*/
void Cd();
/*
	����2��cd����ı�Ŀ¼
	��ʽ��cd path
	˵����path���������·�������·��
*/
void Dir();
/*
	����3��dir�����ʾĿ¼
	��ʽ��dir <path> <s>
	˵����path��ʾָ��·����û��path��ʾ�鿴��ǰĿ¼
	      s��ʾ��ʾ���е���Ŀ¼��û��s��ʾ����ʾ
*/
void Md();
/*
	����4��md�������Ŀ¼
	��ʽ��md dirname <path>
	˵����dirname��Ŀ¼��
	      path��ָ��·���´�����û��path���ڵ�ǰ·���´���
*/
void Rd();
/*
	����5��rd���ɾ��Ŀ¼
	��ʽ��rd path
	˵����path�����Ǿ���·���������·��
*/
void Newfile();
/*
	����6��newfile��������ļ�
	��ʽ��newfile filename <path>
	˵����filename�Ƕ������ļ���
	      path��ָ��·���´�����û��path���ڵ�ǰ·���´���
*/
void Cat();
/*
	����7��cat������ļ�
	��ʽ��cat path <r,w>
	˵����path���������·�������·��
	      r��ʾֻ���򿪣�w��ʾ׷��д��򿪣��Իس�������
*/
void Copy();
/*
	����8��copy��������ļ�
	��ʽ��copy<host> D:\xxx\yyy\zzz /aaa/bbb <0,1> �� copy<lxfs> /xxx/yyy/zzz /aaa/bbb
	˵������һ��0�������ļ��������ļ�ϵͳ
		  ��һ��1���ļ�ϵͳ�����������ļ�
	      �ڶ������ļ�ϵͳ֮�俽��
*/
void Del();
/*
	����9��del���ɾ���ļ�
	��ʽ��del path
	˵����path�����Ǿ���·���������·��
*/
void Check();
/*
	����10��check�����Ⲣ�ָ��ļ�ϵͳ
	��ʽ��check
	˵������
*/
void Ls();
/*
	����11��ls�����ʾ����Ŀ¼�Ͷ������ļ���Ϣ
	��ʽ��ls
	˵������
*/

#endif