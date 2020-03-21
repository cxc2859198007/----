#include"Linux_ext2.h"
#include"TestData.h"
using namespace std;


bool FindDisk() {//���ļ�ϵͳ�Ķ������ļ�
	//step1: ���ļ�
	f.open(FileName, ios::in | ios::binary);
	//step2: ���ؽ�� ���ܴ򿪣���Ҫ�ر�
	if (!f) return false;
	f.close();
	return true;
}
void FindOrder(Order& ord) {//�����������ĺ���
	//step1: ��ն���
	ord.clear();
	
	//step2: ���ո�ֽ����������ַ���
	ReadShareMemory();
	
	ord.cnt = smi.cnt;
	for (int i = 0; i < ord.cnt; i++) {
		ord.od[i] = smi.str[i];
	}

	//step3: ���ݵ�һ���ַ���ȷ����������ͣ������������ΪINF
	if (order.od[0] == "exit" || order.od[0] == "EXIT" || order.od[0] == "Exit") order.type = 0;
	else if (order.od[0] == "info")                                              order.type = 1;
	else if (order.od[0] == "cd")                                                order.type = 2;
	else if (order.od[0] == "dir")                                               order.type = 3;
	else if (order.od[0] == "md")                                                order.type = 4;
	else if (order.od[0] == "rd")                                                order.type = 5;
	else if (order.od[0] == "newfile")                                           order.type = 6;
	else if (order.od[0] == "cat")                                               order.type = 7;
	else if (order.od[0] == "copy<host>" || order.od[0] == "copy<lxfs>")         order.type = 8;
	else if (order.od[0] == "del")                                               order.type = 9;
	else if (order.od[0] == "check")                                             order.type = 10;
	else if (order.od[0] == "ls")                                                order.type = 11;
	else                                                                         order.type = INF;

	return;
}
unsigned int FindFirstZero(unsigned int x) {//��xת��Ϊ�����ƺ�ĵ�һ��0��λ�� [1,2,...32]
	//step1: �ӵ�λ����λ�жϣ����и���
	unsigned int ans = INF;
	for (unsigned int i = 0; i < 32; i++) {
		unsigned int y = (unsigned int)1 << i;
		if ((y & x) == 0) ans = i;
	}

	//step2: ���ؽ��
	if (ans == INF) return INF;
	return ((unsigned int)32 - ans);
}
unsigned int FindFreeBlock() {//�ҵ�һ�����е����ݿ飬��ռ�� [��֤һ���п���]
	//step1: ����FindFirstZero�����ҵ���һ��0���±겢��λ������¿�λͼ
	unsigned int pos1 = 0, pos2 = 0;
	for (int i = 0; i < 4096; i++) {
		unsigned int pos = FindFirstZero(blockmp.use[i]);
		if (pos != INF) {
			blockmp.use[i] = (blockmp.use[i] | ((unsigned int)1 << ((unsigned int)32 - pos)));
			pos1 = i;
			pos2 = pos;
			break;
		}
	}

	//step2: ���³����飬�������ݿ��±�
	superblock.use_datablock++;
	return pos1 * 32 + pos2 - 1;//���ݿ��±�
}
unsigned int FindFreeINode() {//�ҵ�һ�����е�iNode����ռ�� [��֤һ���п���]
	//step1: ����FindFirstZero�����ҵ���һ��0���±겢��λ�������iNodeλͼ
	unsigned int pos1 = 0, pos2 = 0;
	for (int i = 0; i < 1024; i++) {
		unsigned int pos = FindFirstZero(inodemp.use[i]);
		if (pos != INF) {
			inodemp.use[i] = (inodemp.use[i] | ((unsigned int)1 << ((unsigned int)32 - pos)));
			pos1 = i;
			pos2 = pos;
			break;
		}
	}

	//step2: ���³����飬����iNode��
	superblock.use_inode++;
	return pos1 * 32 + pos2 - 1;//iNode�±�
}
void FindAbsolutePath(string& relpath) {//��·��ת��Ϊ����·��
	/*
	��Ŀ¼��/
	����·����/xxx/yyy/zzz
	���·����./yyy/zzz
	*/
	//step1: ����Ǿ���·���������������ֱ�ӷ���
	if (relpath[0] == '/') return;

	//step2: ��������·������Ҫ��������һ�����Ŀ¼������Ŀ¼
	relpath = relpath.substr(1);//��.ȥ��
	unsigned int nowinode = CurrentPath;
	while (1) {
		string str = inodetb.inode[nowinode].name;
		if (str == "/") break;
		else {
			string tmp = inodetb.inode[nowinode].name;
			relpath = "/" + tmp + relpath;
			nowinode = inodetb.inode[nowinode].last_pos;
		}
	}

	return;
}
unsigned int FindFileINode(unsigned int nowdir, string filename) {//����Ŀ¼iNode�ţ��Ҵ�Ŀ¼���ļ���Ϊfilename��iNode��
	//step1: ����Ŀ¼��iNode�ţ�ȷ����Ŀ¼����Ϣ�����ĸ����̿���
	unsigned int blockpos = inodetb.inode[nowdir].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;

	//step2: ��ȡ������̿�
	Block db;
	ReadBlock(blockpos, db);

	//step3: ������Ŀ¼�������ļ����ļ������Ҳ����ͷ���INF
	for (int i = 0; i < inodetb.inode[nowdir].files_num; i++) {
		unsigned int nowfile = db.data[i];
		if (inodetb.inode[nowfile].name == filename) {
			return nowfile;
		}
	}
	return INF;
}
unsigned int FindFileINode(string s) {//���ݾ���·����iNode��
	//step1: �Ƚ�·���ֲ� /�����Ŀ¼ /xxx/yyy/zzz�ڸ�Ŀ¼����xxx����xxxĿ¼����yyy����yyyĿ¼����zzz�ļ�
	if (s == "/") return 0; //��Ŀ¼��Ҫ����
	path.clear();
	string::size_type posl, posr;
	posl = 1;
	posr = s.find("/", posl);
	while (1) {
		if (posr == s.npos) break;
		path.ph[path.cnt++] = s.substr(posl, posr - posl);
		posl = posr + 1;
		posr = s.find("/", posl + 1);
	}
	path.ph[path.cnt++] = s.substr(posl, s.length() - posl);

	//step2: �����ң��м�ĳһ��Ŀ¼�������ļ��Ҳ���������·������
	unsigned nowinode = RootDir;//����Ǹ�Ŀ¼
	for (int i = 0; i < path.cnt; i++) {
		nowinode = FindFileINode(nowinode, path.ph[i]);
		if (nowinode == INF) return INF;//·������
	}

	return nowinode;
}


void ReadFileSystem() {//��ȡ�ļ�ϵͳ�ĳ����顢����������λͼ��iNode����Ϣ
	//step1: ���ļ�ϵͳ�Ķ������ļ�
	f.open(FileName, ios::in | ios::binary);

	//step2: ��ȡ
	f.read((char*)&bootblock, sizeof(BootBlock));       //��ȡ������
	f.read((char*)&superblock, sizeof(SuperBlock));     //��ȡ������
	f.read((char*)&groupdes, sizeof(GroupDescription)); //��ȡ��������
	f.read((char*)&blockmp, sizeof(BlockBitmap));       //��ȡ��λͼ
	f.read((char*)&inodemp, sizeof(iNodeBitmap));       //��ȡiNodeλͼ
	f.read((char*)&inodetb, sizeof(iNodeTable));        //��ȡiNode��

	//step3: �ر��ļ�
	f.close();
	return;
}
void ReadBlock(unsigned int pos, Block& bk) {//��ȡ�±�Ϊpos�Ĵ��̿���Ϣ
	//step1: ���ļ�ϵͳ�Ķ������ļ�
	f.open(FileName, ios::in | ios::binary);

	//step2: ����ָ��ƫ���������ƶ���ָ��
	unsigned int offset = pos * sizeof(Block);
	f.seekg(offset, ios::beg);

	//step3: ��ȡ���̿�
	f.read((char*)&bk, sizeof(Block));
	
	//step4: �ر��ļ�
	f.close();
	return;
}
void ReadShareMemory() {//��ȡһ�й����ڴ��е�����
	//֪ͨshell��ϣ������������
	pBufIoo->toshell = 1;

	//��ȡ
	while (true) {//�ȴ�����
		if (pBufIoo->tosimdisk == 1) {//shell��֪ͨҪ�����������ȫ�����빲���ڴ�
			// �����������ڴ�
			hMapFileIn = OpenFileMapping(
				FILE_MAP_READ,
				FALSE,
				NameIn);
			if (hMapFileIn == NULL) {
				int error = GetLastError();
				_tprintf(TEXT("Could not create file mapping object (%d).\n"), error);
				return;
			}

			// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬��ȡ���������
			pBufIn = (ShareMemory*)MapViewOfFile(hMapFileIn,
				FILE_MAP_READ,
				0,
				0,
				BUF_SIZE);
			if (pBufIn == NULL) {
				int error = GetLastError();
				_tprintf(TEXT("Could not map view of file (%d).\n"), error);
				CloseHandle(hMapFileIn);
				return;
			}

			//�����ݷ������
			smi.cnt = pBufIn->cnt;
			for (int i = 0; i < 20; i++) {
				for (int j = 0; j < 300; j++) {
					smi.str[i][j] = pBufIn->str[i][j];
				}
			}

			break;
		}
	}

	//֪ͨshell�������Ѷ�����ɣ�����Ȳ�Ҫ����Ҳ��Ҫ���
	pBufIoo->toshell = 0;
	
	UnmapViewOfFile(pBufIn);
	CloseHandle(hMapFileIn);

	Sleep(10);
	return;
}

void WriteFileSystem() {//�������顢������������λͼ��iNodeλͼ��iNode��д���ļ�ϵͳ
	//step1: ���ļ�ϵͳ�Ķ������ļ�,�ƶ�дָ��
	f.open(FileName, ios::in | ios::out | ios::binary);
	f.seekp(0, ios::beg);

	//step2: д��
	f.write((char*)&bootblock, sizeof(BootBlock));       //д��������
	f.write((char*)&superblock, sizeof(SuperBlock));     //д�볬����
	f.write((char*)&groupdes, sizeof(GroupDescription)); //д����������
	f.write((char*)&blockmp, sizeof(BlockBitmap));       //д���λͼ
	f.write((char*)&inodemp, sizeof(iNodeBitmap));       //д��iNodeλͼ
	f.write((char*)&inodetb, sizeof(iNodeTable));        //д��iNode��

	//step3: �ر��ļ�
	f.close();
	return;
}
void WriteBlock(unsigned int pos, Block& bk) {//��bk�鸲��д���±�Ϊpos�Ĵ��̿���
	//step1: ���ļ�ϵͳ�Ķ������ļ�
	f.open(FileName, ios::in | ios::out | ios::binary);

	//step2: ����ָ��ƫ���������ƶ�дָ��
	unsigned int offset = pos * sizeof(Block);
	f.seekp(offset, ios::beg);

	//step3: д����̿�
	f.write((char*)&bk, sizeof(Block));

	//step4: �ر��ļ�
	f.close();
	return;
}
void WriteShareMemory() {//�����빲���ڴ���һ������
	//���������ļ�
	hMapFileOut = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BUF_SIZE,
		NameOut);
	if (hMapFileOut == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not create file mapping object (%d).\n"), error);
		return;
	}

	// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
	pBufOut = (ShareMemory*)MapViewOfFile(hMapFileOut,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);
	if (pBufOut == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not map view of file (%d).\n"), error);
		CloseHandle(hMapFileOut);
		return;
	}

	//��ջ�����
	pBufOut->cnt = 0;
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 300; j++) {
			pBufOut->str[i][j] = '\0';
		}
	}

	//��ȡ���ݣ�д�빲���ڴ�
	pBufOut->cnt = smo.cnt;
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 300; j++) {
			pBufOut->str[i][j] = smo.str[i][j];
		}
	}
	
	//֪ͨshell��ϣ���ܰѹ����ڴ��е��������
	pBufIoo->toshell = 2;
	
	//��shell��ȫ�����
	while (true) {
		if (pBufIoo->tosimdisk == 2) {//shell��֪ͨ�Ѱѹ����ڴ��е�����ȫ�����
			pBufIoo->toshell = 0;//֪ͨshell�����ڼȲ�Ҫ����Ҳ��Ҫ���
			break;
		}
	}
	
	//��ն���
	smo.clear();

	UnmapViewOfFile(pBufOut);
	CloseHandle(hMapFileOut);

	Sleep(10);
	return;
}

void CreateFileSystem() {//�����ļ�ϵͳ
	//step1: �򿪶������ļ�
	f.open(FileName, ios::out | ios::binary);

	//step2: д����������ݣ��������ݿ鲿��̫�󣬲��˴������󣬲���д��98281�����̿�ķ�ʽ
	f.write((char*)&bootblock, sizeof(BootBlock));       //д��������
	f.write((char*)&superblock, sizeof(SuperBlock));     //д�볬����
	f.write((char*)&groupdes, sizeof(GroupDescription)); //д����������
	f.write((char*)&blockmp, sizeof(BlockBitmap));       //д���λͼ
	f.write((char*)&inodemp, sizeof(iNodeBitmap));       //д��iNodeλͼ
	f.write((char*)&inodetb, sizeof(iNodeTable));        //д��iNode��
	Block block;
	for (int i = 0; i < DataBlockNum; i++) {              //д�����ݿ�
		f.write((char*)&block, sizeof(Block));
	}

	//step3: �ر��ļ�
	f.close();
	return;
}
unsigned int CreateNewFile(unsigned int fathinode, string filename) {//��iNodeΪfathinode��Ŀ¼�´������ļ�filename
	//step1: �鿴iNodeʣ�����������ݿ�ʣ�������Ƿ�����½��ļ�
	if (superblock.use_inode == superblock.tot_inode) return INF;
	if (superblock.use_datablock == superblock.tot_datablock) return INF;
	if (inodetb.inode[fathinode].files_num == superblock.file_num2) return INF;

	//step2: ��iNodeλͼ���ҵ���һ��0��ȷ��Ҫ�����iNode��,������
	unsigned int soninode = FindFreeINode();

	//step3: ���¸�Ŀ¼iNode�����ݿ��е�����
	inodetb.inode[fathinode].files_num++;
	unsigned int blockpos = inodetb.inode[fathinode].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db;
	ReadBlock(blockpos, db);
	db.data[inodetb.inode[fathinode].files_num - 1] = soninode;
	WriteBlock(blockpos, db);

	//step4: �������ļ�iNode�е�����
	char tmpname[NameLen]; memset(tmpname, '\0', sizeof(tmpname));
	strcpy_s(tmpname, filename.c_str());
	strcpy_s(inodetb.inode[soninode].name, tmpname);   //�ļ�������Ҫ��stringת��Ϊ�ַ�����
	inodetb.inode[soninode].last_pos = fathinode;      //��Ŀ¼��iNode��
	inodetb.inode[soninode].next_pos = INF;		       //��һ��iNode�ţ����ڳ���17KB���ļ����½����ļ�Ϊ�������ò���
	inodetb.inode[soninode].type = 1;				   //ָ������ͨ�ļ�
	inodetb.inode[soninode].files_num = 0;			   //��ͨ�ļ������������ļ�������Ϊ0
	inodetb.inode[soninode].user_id = root;		       //�ȼ�����root������
	inodetb.inode[soninode].mode = 0;				   //�ȼ����ǿɶ���д��Ŀ¼
	inodetb.inode[soninode].block_num = 0;			   //�յ���ͨ�ļ���ռ�����ݿ�

	//step5: ����DataBlock�е����ݣ������ǿ��ļ������Բ����κβ���

	//step6: �����������Ϣ���ļ�ϵͳ����֤����һ����
	WriteFileSystem();

	//step7:���µ�ǰ·��
	CurrentPath = fathinode;

	//step8: �������ļ���iNode
	return soninode;
}
unsigned int CreateNewDir(unsigned int fathinode, string dirname) {//������Ŀ¼��fathinode��֤������Ч
	//step1: �鿴iNodeʣ�����������ݿ�ʣ�������Ƿ�����½��ļ� ����Ŀ¼����Ŀ¼�����Ƿ�ﵽ����
	if (superblock.use_inode  == superblock.tot_inode) return INF;
	if (superblock.use_datablock  == superblock.tot_datablock) return INF;
	if (inodetb.inode[fathinode].files_num == superblock.file_num2) return INF;

	//step2: ��iNodeλͼ���ҵ���һ��0��ȷ��Ҫ�����iNode��,������
	unsigned int soninode = FindFreeINode();

	//step3: �ڿ�λͼ���ҵ���һ��0��ȷ��Ҫ��������ݿ飬������
	unsigned int datablock = FindFreeBlock();
	

	//step4: ���¸�Ŀ¼iNode�����ݿ��е�����
	inodetb.inode[fathinode].files_num++;
	unsigned int blockpos = inodetb.inode[fathinode].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db;
	ReadBlock(blockpos, db);
	db.data[inodetb.inode[fathinode].files_num - 1] = soninode;
	WriteBlock(blockpos, db);

	//step5: ������Ŀ¼iNode�е�����
	char tmpname[NameLen]; memset(tmpname, '\0', sizeof(tmpname));
	strcpy_s(tmpname, dirname.c_str());
	strcpy_s(inodetb.inode[soninode].name, tmpname);   //�ļ�������Ҫ��stringת��Ϊ�ַ�����
	inodetb.inode[soninode].last_pos = fathinode;      //��Ŀ¼��iNode��
	inodetb.inode[soninode].next_pos = INF;		       //��һ��iNode�ţ����ڳ���17KB���ļ���Ŀ¼�ļ����ò��ϵ�
	inodetb.inode[soninode].type = 0;				   //ָ����Ŀ¼�ļ�
	inodetb.inode[soninode].files_num = 0;			   //��Ŀ¼�»�û����Ŀ¼���ļ�
	inodetb.inode[soninode].user_id = root;		       //�ȼ�����root������
	inodetb.inode[soninode].mode = 0;				   //�ȼ����ǿɶ���д��Ŀ¼
	inodetb.inode[soninode].block_num = 1;			   //Ŀ¼�ļ�ֻռһ�����ݿ����ڴ洢��Ŀ¼���ļ���iNode
	inodetb.inode[soninode].block_pos[0] = datablock;  //��¼��Ŀ¼�����ݿ�iNode��

	//step6: ����DataBlock�е����ݣ������ǿ�Ŀ¼�����Բ����κβ���

	//step7:�����������Ϣ���ļ�ϵͳ����֤����һ����
	WriteFileSystem();

	//step8: ���µ�ǰ·��
	CurrentPath = soninode;

	//step9: ������Ŀ¼��iNode
	return soninode;
}


void RemoveFileINodeBitmap(unsigned int nowinode) {//��iNode��Ϊnowinode���ļ���ռ�õ�iNodeλͼȫ���ͷ�
	//step1: ��iNodeλͼ�ͷ�
	unsigned int pos1 = 0, pos2 = 0;
	pos1 = nowinode / 32;
	pos2 = nowinode % 32;
	unsigned int h = (1 << ((unsigned int)31 - pos2));
	h = (~h);
	inodemp.use[pos1] = (inodemp.use[pos1] & h);
	superblock.use_inode--;
	
	//step2: ������ļ�ռ�ö��iNode��ҲҪ���
	if (inodetb.inode[nowinode].next_pos != INF) {
		RemoveFileINodeBitmap(inodetb.inode[nowinode].next_pos);
	}

	return;
}
void RemoveFileBlockBitmap(unsigned int nowinode) {//��iNode��Ϊnowinode���ļ���ռ�õĿ�λͼȫ���ͷ�
	//step1: �ѿ�λͼ�ͷ�
	for (int i = 0; i < inodetb.inode[nowinode].block_num; i++) {
		unsigned int pos1 = 0, pos2 = 0;
		unsigned int datablockpos = inodetb.inode[nowinode].block_pos[i];
		pos1 = datablockpos / 32;
		pos2 = datablockpos % 32;
		unsigned int h = (1 << ((unsigned int)31 - pos2));
		h = (~h);
		blockmp.use[pos1] = (blockmp.use[pos1] & h);
		superblock.use_datablock--;
	}
	//step2: ������ļ�ռ�ö��iNode����Ӧ�����ݿ�ҲҪ���
	if (inodetb.inode[nowinode].next_pos != INF) {
		RemoveFileBlockBitmap(inodetb.inode[nowinode].next_pos);
	}

	return;
}
void RemoveFileDataBlock(unsigned int nowinode) {//��iNode��Ϊnowinode���ļ���ռ�õ����ݿ�ȫ���ͷ�
	//step1: ��ÿһ�����ݿ鶼���
	Block db;
	for (int i = 0; i < inodetb.inode[nowinode].block_num; i++) {
		unsigned int blockpos = inodetb.inode[nowinode].block_pos[i];
		blockpos = groupdes.data_begin + blockpos;
		WriteBlock(blockpos, db);
	}

	//step2: ������ļ�ռ�ö��iNode��ҲҪ���
	if (inodetb.inode[nowinode].next_pos != INF) {
		RemoveFileDataBlock(inodetb.inode[nowinode].next_pos);
	}

	return;
}
void RemoveFile(unsigned int nowinode) {//ɾ��iNodeΪnowinode����ͨ�ļ�
	//step1: �����ݿ��е������ͷ�,Ҳ���Ǵ��Ŀ¼��Ϣ�����ݿ�����ȫ����0
	RemoveFileDataBlock(nowinode);

	//step2: ���¸�Ŀ¼iNode�����ݿ��е�����
	unsigned int fathinode = inodetb.inode[nowinode].last_pos;
	unsigned int blockpos = inodetb.inode[fathinode].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db;
	ReadBlock(blockpos, db);
	for (int i = 0, jud = 0; i < inodetb.inode[fathinode].files_num; i++) {
		if (jud == 1) {
			db.data[i] = db.data[i + 1];
		}
		else if (db.data[i] == nowinode) {
			jud = 1;
			db.data[i] = db.data[i + 1];
		}
	}
	WriteBlock(blockpos, db);
	inodetb.inode[fathinode].files_num--;

	//step3: ���¿�λͼ
	RemoveFileBlockBitmap(nowinode);

	//step4: ����iNodeλͼ
	RemoveFileINodeBitmap(nowinode);

	//step5: �����Լ���iNodeTable�����
	inodetb.inode[nowinode].clear1();

	//step6: �����������Ϣ���ļ�ϵͳ����֤����һ����
	WriteFileSystem();

	return;
}
void RemoveEmptyDir(unsigned int nowinode) {//ɾ��iNodeΪnowinode��Ŀ¼����Ŀ¼��֤�ǿ�Ŀ¼
	//step1: �����ݿ��е������ͷ�,Ҳ���Ǵ��Ŀ¼��Ϣ�����ݿ�����ȫ����0
	unsigned int blockpos = inodetb.inode[nowinode].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db1;
	WriteBlock(blockpos, db1);

	//step2: ���¸�Ŀ¼iNode�����ݿ��е�����
	unsigned int fathinode = inodetb.inode[nowinode].last_pos;
	blockpos= inodetb.inode[fathinode].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db2;
	ReadBlock(blockpos, db2);
	for (int i = 0, jud = 0; i < inodetb.inode[fathinode].files_num; i++) {
		if (jud == 1) {
			db2.data[i] = db2.data[i + 1];
		}
		else if (db2.data[i] == nowinode) {
			jud = 1;
			db2.data[i] = db2.data[i + 1];
		}
	}
	WriteBlock(blockpos, db2);
	inodetb.inode[fathinode].files_num--;

	//step3: ���¿�λͼ
	unsigned int pos1 = 0, pos2 = 0;
	unsigned int datablockpos = inodetb.inode[nowinode].block_pos[0];
	pos1 = datablockpos / 32;
	pos2 = datablockpos % 32;
	unsigned int h = (1 << ((unsigned int)31 - pos2));
	h = (~h);
	blockmp.use[pos1] = (blockmp.use[pos1] & h);
	superblock.use_datablock--;

	//step4: ����iNodeλͼ
	unsigned int pos3 = 0, pos4 = 0;
	pos3 = nowinode / 32;
	pos4 = nowinode % 32;
	h = (1 << ((unsigned int)31 - pos4));
	h = (~h);
	inodemp.use[pos3] = (inodemp.use[pos3] & h);
	superblock.use_inode--;

	//step5: �����Լ���iNodeTable�����
	inodetb.inode[nowinode].clear1();

	//step6: �����������Ϣ���ļ�ϵͳ����֤����һ����
	WriteFileSystem();

	return;
}
void RemoveDir(unsigned int nowinode) {//ɾ��iNodeΪnowinode��Ŀ¼
	//step1: �������ͨ�ļ���ֱ�ӵ��ú���ɾ���������Ŀ¼�ļ�������ֱ��ɾ�����ǿյݹ�ɾ��
	if (inodetb.inode[nowinode].type == 1) {//��ͨ�ļ�
		RemoveFile(nowinode);
		return;
	}
	else {//Ŀ¼�ļ�
		if (inodetb.inode[nowinode].files_num == 0) {
			RemoveEmptyDir(nowinode);
			return;
		}
		else {
			unsigned int blockpos = inodetb.inode[nowinode].block_pos[0];
			blockpos = groupdes.data_begin + blockpos;
			Block db;
			ReadBlock(blockpos, db);

			while (inodetb.inode[nowinode].files_num != 0) {
				ReadBlock(blockpos, db);
				unsigned int nowfile = db.data[0];
				RemoveDir(nowfile);
			}
			RemoveEmptyDir(nowinode);
		}
	}
	return;
}


void CatRead(unsigned int nowinode) {
	Block db;
	for (;;) {
		//step1: ö�ٵ�ǰiNode�����п������
		for (int i = 0; i < inodetb.inode[nowinode].block_num; i++) {
			unsigned int blockpos = inodetb.inode[nowinode].block_pos[i];
			blockpos = groupdes.data_begin + blockpos;
			ReadBlock(blockpos, db);
			for (int j = 0; j < 256; j++) {
				char c1, c2, c3, c4;
				c1 = char((db.data[j] & (unsigned int)4278190080) >> 24);
				c2 = char((db.data[j] & (unsigned int)16711680) >> 16);
				c3 = char((db.data[j] & (unsigned int)65280) >> 8);
				c4 = char(db.data[j] & (unsigned int)255);
				if (c1 == '\0' && c2 == '\0' && c3 == '\0' && c4 == '\0') {//����4���ո��ʾ���ݽ���
					break;
				}
				
				smo.cnt = 1;
				smo.str[0][0] = c1; smo.str[0][1] = c2; smo.str[0][2] = c3; smo.str[0][3] = c4;
				WriteShareMemory();
			}
		}

		//step2: ���Ǵ��ļ�����ռ�ж��iNode
		if (inodetb.inode[nowinode].next_pos == INF) break;
		nowinode = inodetb.inode[nowinode].next_pos;
	}
	smo.cnt = 1;
	smo.str[0][0] = '\n';
	WriteShareMemory();
	
	return;
}
void CatReadToHost(unsigned int nowinode, string path) {
	Block db;
	path = path + "/" + inodetb.inode[nowinode].name;
	fout.open(path, ios::out);
	for (;;) {
		//step1: ö�ٵ�ǰiNode�����п������
		for (int i = 0; i < inodetb.inode[nowinode].block_num; i++) {
			unsigned int blockpos = inodetb.inode[nowinode].block_pos[i];
			blockpos = groupdes.data_begin + blockpos;
			ReadBlock(blockpos, db);
			for (int j = 0; j < 256; j++) {
				char c1, c2, c3, c4;
				c1 = char((db.data[j] & (unsigned int)4278190080) >> 24);
				c2 = char((db.data[j] & (unsigned int)16711680) >> 16);
				c3 = char((db.data[j] & (unsigned int)65280) >> 8);
				c4 = char(db.data[j] & (unsigned int)255);
				if (c1 == '\0' && c2 == '\0' && c3 == '\0' && c4 == '\0') {//����4���ո��ʾ���ݽ���
					break;
				}
				fout << c1 << c2 << c3 << c4;
			}
		}

		//step2: ���Ǵ��ļ�����ռ�ж��iNode
		if (inodetb.inode[nowinode].next_pos == INF) break;
		nowinode = inodetb.inode[nowinode].next_pos;
	}
	
	f.close();
	return;
}
void CatWrite(unsigned int nowinode) {
	//step1: ����һ���ַ���
	smo.cnt = 1;
	smo.str[0][0] = '\0'; smo.str[0][1] = '\0';
	WriteShareMemory();

	string str;
	ReadShareMemory();
	for (int i = 0; i < smi.cnt - 1; i++) {
		str = str + smi.str[i] + " ";
	}
	str = str + smi.str[smi.cnt - 1];

	//step2: �ж�iNode�������ݿ鹻����
	unsigned int Size = sizeof(str);
	unsigned int needblock = 0, needinode = 0;
	needblock = Size / sizeof(Block);
	if (Size % sizeof(Block) != 0) needblock++;
	needinode = needblock / 17;
	if (needblock % 17 != 0) needinode++;
	
	if (superblock.use_datablock + needblock > superblock.tot_datablock) {//���ݿ��������㣬����ʧ��
		string tmps = "  Datablock has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	if (superblock.use_inode + needinode > superblock.tot_inode) {//iNode�������㣬����ʧ��
		string tmps = "  iNode has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}

	//step3: ���ַ�������4��������������д�뻺����
	unsigned int len = str.length();
	if (len % 4 == 1) str = str + "   ";
	else if (len % 4 == 2) str = str + "  ";
	else if (len % 4 == 3) str = str + " ";
	len = str.length();

	//step3: д�뻺����
	Block db;
	int cnt = 0;
	for (int i = 0; i < needblock; i++) {
		db.clear();
		for (int j = 0; j < 256; j++) {
			db.data[j] = (unsigned int)(str[cnt++] << 24);
			db.data[j] = (db.data[j] | ((unsigned int)(str[cnt++] << 16)));
			db.data[j] = (db.data[j] | ((unsigned int)(str[cnt++] << 8)));
			db.data[j] = (db.data[j] | ((unsigned int)(str[cnt++])));
			if (cnt == len) break;
		}
		buffer.push(db);
	}

	//step4: ��������׷��д���ļ�ϵͳ
	CopyBufferToLinux(nowinode);
	string tmps = "  CatWrite order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();
	
	//step5: �����ļ�ϵͳ����
	WriteFileSystem();

	return;
}


void CopyHostToBuffer(string hostpath) {//��Ҫ�������ļ��ŵ���������
	//step1: ��ջ�����
	while (!buffer.empty()) buffer.pop();

	//step2: ����ļ��ܴ�С����Ҫռ�õ��������̿�������Ҫռ�õ��ܿ���
	unsigned int filesize = 0, blockneed1 = 0, blockneed2 = 0;
	f.open(hostpath, ios::in);
	f.seekg(0, ios::end);
	filesize = f.tellg();
	blockneed1 = filesize / sizeof(Block);
	if (filesize % sizeof(Block) == 0) blockneed2 = blockneed1;
	else blockneed2 = blockneed1 + 1;
	f.seekg(0, ios::beg);

	//step3: �������Ĵ��̿�
	Block db1;
	for (int i = 0; i < blockneed1; i++) {
		for (int j = 0; j < 256; j++) {
			char c1, c2, c3, c4;
			f.read((char*)&c1, sizeof(char));
			f.read((char*)&c2, sizeof(char));
			f.read((char*)&c3, sizeof(char));
			f.read((char*)&c4, sizeof(char));
			db1.data[j] = (((unsigned int)c1 << 24) | ((unsigned int)c2 << 16) | ((unsigned int)c3 << 8) | (unsigned int)c4);
		}
		buffer.push(db1);
	}

	//step4: ʣ������ݰ�unsigned int���Ͷ���
	Block db2;
	unsigned int remain = filesize % sizeof(Block);
	unsigned int intneed = remain / sizeof(unsigned int), charneed = remain % sizeof(unsigned int);
	for (int i = 0; i < intneed; i++) {
		char c1, c2, c3, c4;
		f.read((char*)&c1, sizeof(char));
		f.read((char*)&c2, sizeof(char));
		f.read((char*)&c3, sizeof(char));
		f.read((char*)&c4, sizeof(char));
		db2.data[i] = (((unsigned int)c1 << 24) | ((unsigned int)c2 << 16) | ((unsigned int)c3 << 8) | (unsigned int)c4);
	}

	if (charneed != 0) {
		char c1, c2, c3;
		if (charneed >= 1) f.read((char*)&c1, sizeof(char));
		if (charneed >= 2) f.read((char*)&c2, sizeof(char));
		if (charneed >= 3) f.read((char*)&c3, sizeof(char));

		if (charneed == 1) db2.data[intneed] = ((unsigned int)c1 << 24);
		else if (charneed == 2) db2.data[intneed] = (((unsigned int)c1 << 24) | ((unsigned int)c2 << 16));
		else if (charneed == 3) db2.data[intneed] = (((unsigned int)c1 << 24) | ((unsigned int)c2 << 16) | ((unsigned int)c3 << 8));
	}
	buffer.push(db2);

	//step5: �ر��ļ�
	f.close();
}
void CopyBufferToLinux(unsigned int nowinode) {//��������׷��д���ļ�ϵͳ
	//step1: ԭ�ļ�����ռ�ж��iNode����Ҫ�ҵ����һ��
	while (inodetb.inode[nowinode].next_pos != INF)
		nowinode = inodetb.inode[nowinode].next_pos;

	//step2: д���ļ�ϵͳ
	Block db;
	while (!buffer.empty()) { //��������Ϊ��
		if (inodetb.inode[nowinode].block_num == 17) {//��iNode����
			unsigned int nextinode = FindFreeINode();//��һ���ļ���iNode�±�
			inodetb.inode[nextinode].clear2();
			inodetb.inode[nowinode].next_pos = nextinode;//�γ�����ṹ
			
			return CopyBufferToLinux(nextinode);
		}

		//ȡ����
		db = buffer.front();  //ȡ����
		buffer.pop();         //��������
		
		//�������ݿ�
		unsigned int datablock = FindFreeBlock();        //��������ݿ��±�
		WriteBlock(groupdes.data_begin + datablock, db); //д�����ݿ�
		
		//����iNode��
		unsigned int pos = inodetb.inode[nowinode].block_num++;
		inodetb.inode[nowinode].block_pos[pos] = datablock;
	}

	return;
}
void CopyHost(string filename, string hostpath, unsigned int dirinode) {//������·��Ϊhostpath,�ļ���Ϊfilename���ļ�������iNodeΪdirinode��Ŀ¼��
	//step1: �������ļ����ݷ��뻺������
	CopyHostToBuffer(hostpath);

	//step2: �ж�iNode�������ݿ鹻����
	unsigned int needblock = buffer.size();
	unsigned int needinode = needblock / 17;
	if (needblock % 17 != 0) needinode++;

	if (superblock.use_datablock + needblock > superblock.tot_datablock) {//���ݿ��������㣬����ʧ��
		string tmps = "  Datablock has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	if (superblock.use_inode + needinode > superblock.tot_inode) {//iNode�������㣬����ʧ��
		string tmps = "  iNode has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	
	//step3: ��dirinode���½�һ���ļ�
	unsigned int nowinode = CreateNewFile(dirinode, filename);

	//step4: ���������Ŀ�����ļ�ϵͳ
	CopyBufferToLinux(nowinode);
	string tmps = "  CopyHost order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();

	//step5: �����ļ�ϵͳ����
	WriteFileSystem();

	return;
}
void CopyLinuxToLinux(unsigned int inode1, unsigned int inode2) {//�ļ�ϵͳ�ڲ��Ķ������ļ�����
	//step1: ռ�����ݿ����һ��
	inodetb.inode[inode2].block_num = inodetb.inode[inode1].block_num;
	
	//step2: ���ƾ�������
	Block db;
	for (int i = 0; i < inodetb.inode[inode1].block_num; i++) {
		unsigned int blockpos = inodetb.inode[inode1].block_pos[i];// ��ȡ����
		blockpos = groupdes.data_begin + blockpos;
		ReadBlock(blockpos, db);

		unsigned int datablock = FindFreeBlock();  //��������ݿ��±�
		inodetb.inode[inode2].block_pos[i] = datablock; //����iNode��
		WriteBlock(groupdes.data_begin + datablock, db);//д�����ݿ�
	}

	if (inodetb.inode[inode1].next_pos != INF) {
		unsigned int nextinode = FindFreeINode();  //��һ���ļ���iNode�±�
		inodetb.inode[nextinode].clear2();
		inodetb.inode[inode2].next_pos = nextinode;

		return CopyLinuxToLinux(inodetb.inode[inode1].next_pos, inodetb.inode[inode2].next_pos);
	}

	return;
}
void CopyLxfs(string filename, unsigned int fileinode, unsigned int dirinode) {//LinuxFileSystem���ڲ�����
	//step1: �ж�iNode�������ݿ鹻����
	unsigned int needblock = 0, needinode = 0;
	unsigned int nowinode = fileinode;
	for (;;) {
		needblock += inodetb.inode[nowinode].block_num;
		needinode += 1;
		if (inodetb.inode[nowinode].next_pos == INF) break;
		nowinode = inodetb.inode[nowinode].next_pos;
	}

	if (superblock.use_datablock + needblock > superblock.tot_datablock) {//���ݿ��������㣬����ʧ��
		string tmps = "  Datablock has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	if (superblock.use_inode + needinode > superblock.tot_inode) {//iNode�������㣬����ʧ��
		string tmps = "  iNode has been used up, copy failed!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}

	//step2: ��dirinode���½�һ���ļ�
	nowinode = CreateNewFile(dirinode, filename);

	//step3: ����
	CopyLinuxToLinux(fileinode, nowinode);
	string tmps = "  CopyLxfs order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();

	//step4: �����ļ�ϵͳ����
	WriteFileSystem();
	
	return;
}


void ChangeDir(string newpath) {//�ı䵱ǰ����Ŀ¼��newpath�Ǿ���·��
	unsigned int x = FindFileINode(newpath);
	if (x != INF) {
		CurrentPath = x;
		string tmps = "  ChangeDir order executed successfully!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	else {
		string tmps = "  ChangeDir order executed unsuccessfully!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
}
bool Exist(unsigned int fathinode, string sonname) {//�ж���iNode��Ϊfathinode��Ŀ¼����û����Ϊsonname���ļ�
	//step1: ֱ�ӵ��ú�����iNode���Ҳ������ǲ�����
	unsigned int x = FindFileINode(fathinode, sonname);
	if (x != INF) return true;
	return false;
}


void ShowHelp() {//������ʾ
	string tmps[20]; 
	tmps[0] =  "***************************Linux ext2 FileSystem***************************\n\n";
	tmps[1] =  "    info                                          Show FileSystem Information\n";
	tmps[2] =  "    cd path                                       Change Directory\n";
	tmps[3] =  "    dir <path> <s>                                Show Directory\n";
	tmps[4] =  "    md dirname <path>                             Create Directory\n";
	tmps[5] =  "    rd path                                       Remove Directory\n";
	tmps[6] =  "    newfile filename <path>                       Create File\n";
	tmps[7] =  "    cat path <r,w>                                Open File\n";
	tmps[8] =  "    copy<host> HostPath LinuxPath <0,1>           Copy Host File\n";
	tmps[9] =  "    copy<lxfs> LinuxPath1 LinuxPath2              Copy Linux File\n";
	tmps[10] = "    del path                                      Delete File\n";
	tmps[11] = "    check                                         Check&Recovery System\n";
	tmps[12] = "    ls                                            Show File List\n";
	tmps[13] = "\n";
	
	smo.cnt = 14;
	for (int i = 0; i < 14; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();

	return;
}
void ShowInfo() {//��ʾ����ϵͳ��Ϣ
	string tmps[30];
	tmps[0] =  "************************Linux ext2 FileSystem************************\n\n";
	tmps[1] =  "Information:\n";
	tmps[2] =  "    Total Disk Size:                          100MB\n";
	tmps[3] =  "    Block Size:                               " + to_string(superblock.block_size) + "B\n";
	tmps[4] =  "    iNode Size:                               " + to_string(superblock.inode_size) + "B\n";
	tmps[5] =  "    Group Number:                             1\n";
	tmps[6] =  "    Total Blocks Number:                      " + to_string(superblock.tot_block) + "\n";
	tmps[7] =  "    BootBlock Blocks Number:                  " + to_string(groupdes.super_begin) + "\n";
	tmps[8] =  "    SuperBlock Blocks Number:                 " + to_string(groupdes.super_end - groupdes.super_begin + 1) + "\n";
	tmps[9] =  "    GroupDescriptionBlock Block Number:       " + to_string(groupdes.groupdes_end - groupdes.groupdes_begin + 1) + "\n";
	tmps[10] = "    DataBlockBitMap Blocks Number:            " + to_string(groupdes.blockbm_end - groupdes.blockbm_begin + 1) + "\n";
	tmps[11] = "    iNodeBitMap Blocks Number:                " + to_string(groupdes.inodebm_end - groupdes.inodebm_begin + 1) + "\n";
	tmps[12] = "    iNodeTable Blocks Number:                 " + to_string(groupdes.inodetb_end - groupdes.inodetb_begin + 1) + "\n";
	tmps[13] = "    DataBlock Blocks Number:                  " + to_string(groupdes.data_end - groupdes.data_begin + 1) + "\n";
	tmps[14] = "\n";
	tmps[15] = "    Used iNode Number:                        " + to_string(superblock.use_inode) + "\n";
	tmps[16] = "    Used DataBlock Number:                    " + to_string(superblock.use_datablock) +  "\n";
	tmps[17] = "\n";

	smo.cnt = 18;
	for (int i = 0; i < 18; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();
	

	return;
}
void ShowDir(unsigned int nowdir) {//��ʾiNode��Ϊnowdir��Ŀ¼��Ϣ
	string tmps[10];
	tmps[0] = "   Directory Name:    " + string(inodetb.inode[nowdir].name) + "\n";
	tmps[1] = "   Block Position:    " + to_string(groupdes.data_begin + inodetb.inode[nowdir].block_pos[0]) + "\n";
	tmps[2] = "   File Length:       128B(iNode) + 1024B(DataBlock) = 1152B\n\n";
	
	smo.cnt = 3;
	for (int i = 0; i < 3; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();
	
	CurrentPath = nowdir;

	return;
}
void ShowDir(unsigned int nowdir, bool sonfile) {//��ʾiNodeΪnowdir��Ŀ¼��Ϣ��������Ŀ¼��
	string tmps[10];
	tmps[0] = "    Directory Name:          " + string(inodetb.inode[nowdir].name) + "\n";
	tmps[1] = "    Block Position:          " + to_string(groupdes.data_begin + inodetb.inode[nowdir].block_pos[0]) + "\n";
	tmps[2] = "    File Length:             128B + 1024B = 1152B\n";
	
	smo.cnt = 3;
	for (int i = 0; i < 3; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();
	

	unsigned int blockpos = inodetb.inode[nowdir].block_pos[0];
	blockpos = groupdes.data_begin + blockpos;
	Block db;
	ReadBlock(blockpos, db);

	tmps[0] = "    SubDirectory/SubFile Number:    " + to_string(inodetb.inode[nowdir].files_num) + "\n";
	tmps[1] = "    SubDirectory Name:  ";
	smo.cnt = 2;
	for (int i = 0; i < 2; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();
	

	for (int i = 0; i < inodetb.inode[nowdir].files_num; i++) {
		unsigned int nowfile = db.data[i];
		if (inodetb.inode[nowfile].type == 0) {
			tmps[0] = string(inodetb.inode[nowfile].name) + " <DIR>   ";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
		}
		else {
			tmps[0] = string(inodetb.inode[nowfile].name) + " <FILE>   ";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
		}
	}
	tmps[0] = "\n\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps[0].c_str());
	WriteShareMemory();

	CurrentPath = nowdir;

	return;
}
void ShowList() {
	Block db;
	while (!qdirinode.empty()) qdirinode.pop();
	qdirinode.push(RootDir);

	char tmps[10][300]; 
	memset(tmps, '\0', sizeof(tmps));
	sprintf_s(tmps[0], "  %-25s%-22s%-10s%-12s%-25s\n", "Directory/File Name", "Parent Directory", "Type", "Blocks", "SubDirectory/SubFile");
	smo.cnt = 1;
	for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
	WriteShareMemory();

	while (!qdirinode.empty()) {
		unsigned int nowinode = qdirinode.front();
		unsigned int fathinode = inodetb.inode[nowinode].last_pos;
		qdirinode.pop();

		if (inodetb.inode[nowinode].type == 0) {//Ŀ¼�ļ�
			
			if (nowinode == RootDir) {//��Ŀ¼
				memset(tmps[0], '\0', sizeof(tmps[0]));
				sprintf_s(tmps[0], "  %-25s%-22s%-10s%-12s", inodetb.inode[nowinode].name, " ", "<DIR>", "1");
				smo.cnt = 1;
				for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
				WriteShareMemory();
			}
			else {
				memset(tmps[0], '\0', sizeof(tmps[0]));
				sprintf_s(tmps[0], "  %-25s%-22s%-10s%-12s", inodetb.inode[nowinode].name, inodetb.inode[fathinode].name, "<DIR>", "1");
				smo.cnt = 1;
				for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
				WriteShareMemory();
			}
			unsigned int blockpos = inodetb.inode[nowinode].block_pos[0];
			blockpos = groupdes.data_begin + blockpos;
			ReadBlock(blockpos, db);
			for (int i = 0, cnt = 0; i < inodetb.inode[nowinode].files_num; i++, cnt++) {
				unsigned int soninode = db.data[i];
				qdirinode.push(soninode);
				if (cnt == 0) {
					memset(tmps[0], '\0', sizeof(tmps[0]));
					sprintf_s(tmps[0], "%-12s\n", inodetb.inode[soninode].name);
					smo.cnt = 1;
					for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
					WriteShareMemory();
				}
				else {
					memset(tmps[0], '\0', sizeof(tmps[0]));
					sprintf_s(tmps[0], "%-71s%s\n", " ", inodetb.inode[soninode].name);
					smo.cnt = 1;
					for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
					WriteShareMemory();
				}
			}
			if (inodetb.inode[nowinode].files_num == 0) {
				memset(tmps[0], '\0', sizeof(tmps[0]));
				tmps[0][0] = '\n';
				smo.cnt = 1;
				for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
				WriteShareMemory();
			}
		}
		else {//��ͨ�������ļ�
			memset(tmps[0], '\0', sizeof(tmps[0]));
			sprintf_s(tmps[0], "  %-25s%-22s%-10s", inodetb.inode[nowinode].name, inodetb.inode[fathinode].name, "<FILE>");
			smo.cnt = 1;
			for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
			WriteShareMemory();

			unsigned int blocknum = 0;
			unsigned int tmpinode = nowinode;
			for (;;) {
				blocknum += inodetb.inode[tmpinode].block_num;
				if (inodetb.inode[tmpinode].next_pos == INF) break;
				tmpinode = inodetb.inode[tmpinode].next_pos;
			}
			memset(tmps[0], '\0', sizeof(tmps[0]));
			_itoa_s(blocknum, tmps[0], 10);
			sprintf_s(tmps[0], "%-12s\n", tmps[0]);
			smo.cnt = 1;
			for (int i = 0; i < 300; i++) smo.str[0][i] = tmps[0][i];
			WriteShareMemory();
		}
	}

	return;
}
void ShowPath() {
	string str = inodetb.inode[CurrentPath].name;
	str = "  " + str + ">$ ";
	smo.cnt = 1;
	strcpy_s(smo.str[0], str.c_str());
	WriteShareMemory();

	return;
}

void Info() {//info
	Reader1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	ShowInfo();

	Reader2();
	return;
}
void Cd() {//cd path
	Reader1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	FindAbsolutePath(order.od[1]);
	ChangeDir(order.od[1]);
	
	Reader2();
	return;
}
void Dir() {//dir �� dir s �� dir path �� dir path s
	Reader1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	if (order.cnt == 1) {//��ǰĿ¼������ʾ��Ŀ¼
		ShowDir(CurrentPath);
	}
	else if (order.cnt == 2) {
		if (order.od[1] == "s")//��ǰĿ¼����ʾ��Ŀ¼
			ShowDir(CurrentPath, true);
		else {//ָ��Ŀ¼������ʾ��Ŀ¼
			FindAbsolutePath(order.od[1]);
			unsigned int inode = FindFileINode(order.od[1]);
			if (inode == INF) {
				string tmps;
				tmps = "  Failed: The path is not exist!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
			}
			else ShowDir(inode);
		}
	}
	else if (order.cnt == 3) {//ָ��Ŀ¼����ʾ��Ŀ¼
		FindAbsolutePath(order.od[1]);
		unsigned int inode = FindFileINode(order.od[1]);
		if (inode == INF) {
			string tmps;
			tmps = "  Failed: The path is not exist!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
		}
		else ShowDir(inode, true);
	}

	Reader2();
	return;
}
void Md() {//md dirname �� md dirname path
	Writer1();
	
	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	if (order.cnt == 2) {//md dirname �ڵ�ǰĿ¼���½�Ŀ¼
		if (Exist(CurrentPath, order.od[1]) == true) {
			string tmps;
			tmps = "  Failed: The Directory already existed!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
			return;
		}
		CreateNewDir(CurrentPath, order.od[1]);
	}
	else {//md dirname path ��path���ڵ�Ŀ¼���½�Ŀ¼
		FindAbsolutePath(order.od[2]);
		unsigned int inode = FindFileINode(order.od[2]);
		if (inode == INF) {
			string tmps;
			tmps = "  Failed: The path is not exist!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
		}
		else {
			if (Exist(inode, order.od[1]) == true) {
				string tmps;
				tmps = "  Failed: The Directory already existed!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
				return;
			}
			CreateNewDir(inode, order.od[1]);
		}
	}
	string tmps;
	tmps = "  Md order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();

	Writer2();
	return;
}
void Rd() {//rd path
	Writer1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	FindAbsolutePath(order.od[1]);
	unsigned int inode = FindFileINode(order.od[1]);
	if (inode == INF) {
		string tmps;
		tmps = "  Failed: The path is not exist!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	else {
		unsigned int tmp = inodetb.inode[inode].last_pos;
		RemoveDir(inode);
		string tmps;
		tmps = "  Rd order executed successfully!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
		CurrentPath = tmp;
	}

	Writer2();
	return;
}
void Newfile() {//newfile filename �� newfile filename path
	Writer1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	if (order.cnt == 2) {//newfile filename �ڵ�ǰĿ¼���½��ļ�
		if (Exist(CurrentPath, order.od[1]) == true) {
			string tmps;
			tmps = "  Failed: The File already existed!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
			return;
		}
		CreateNewFile(CurrentPath, order.od[1]);
	}
	else {//newfile filename path ��path���ڵ�Ŀ¼���½�Ŀ¼
		FindAbsolutePath(order.od[2]);
		unsigned int inode = FindFileINode(order.od[2]);
		if (inode == INF) {
			string tmps;
			tmps = "  Failed: The path is not exist!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
		}
		else {
			if (Exist(inode, order.od[1]) == true) {
				string tmps;
				tmps = "  Failed: The File already existed!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
				return;
			}
			CreateNewFile(inode, order.od[1]);
		}
	}
	string tmps;
	tmps = "  Newfile order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();

	Writer2();
	return;
}
void Cat() {//cat path <r, w>
	Reader1();
	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	FindAbsolutePath(order.od[1]);
	unsigned int inode = FindFileINode(order.od[1]);
	Reader2();

	if (inode == INF) {
		string tmps;
		tmps = "  Failed: The path is not exist!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	else {
		if (order.od[2] == "r") {
			Reader1();
			CatRead(inode);
			CurrentPath = inodetb.inode[inode].last_pos;
			Reader2();
		}
		else if (order.od[2] == "w") {
			Writer1();
			CatWrite(inode);
			CurrentPath = inodetb.inode[inode].last_pos;
			Writer2();
		}
	}

	return;
}
void Copy() {//copy<host> D:\xxx\yyy\zzz /aaa/bbb <0,1> �� copy<lxfs> /xxx/yyy/zzz /aaa/bbb
	Writer1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	FindAbsolutePath(order.od[2]);
	if (order.od[0] == "copy<host>") {//host-->Linux
		if (order.od[3] == "0") {//�����������ļ�ϵͳ
			unsigned int lastpos = order.od[1].find_last_of("\\");//��ȡ�ļ���
			string filename = order.od[1].substr(lastpos + 1);
			unsigned int inode = FindFileINode(order.od[2]);
			if (inode == INF) {
				string tmps;
				tmps = "  Failed: The path is not exist!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
			}
			else {
				if (Exist(inode, filename) == true) {
					string tmps;
					tmps = "  Failed: The File already existed!\n";
					smo.cnt = 1;
					strcpy_s(smo.str[0], tmps.c_str());
					WriteShareMemory();
					return;
				}

				CopyHost(filename, order.od[1], inode);
			}
		}
		else {//�ļ�ϵͳ����������
			unsigned int inode = FindFileINode(order.od[2]);
			if (inode == INF) {
				string tmps;
				tmps = "  Failed: The path is not exist!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
			}
			else {
				CatReadToHost(inode, order.od[1]);
			}
		}

	}
	else {//Linux-->Linux
		unsigned int lastpos = order.od[1].find_last_of("/");//��ȡ�ļ���
		string filename = order.od[1].substr(lastpos + 1);
		FindAbsolutePath(order.od[1]);
		unsigned int inode1 = FindFileINode(order.od[1]);
		unsigned int inode2 = FindFileINode(order.od[2]);
		if (inode1 == INF || inode2 == INF) {
			string tmps;
			tmps = "  Failed: The path is not exist!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps.c_str());
			WriteShareMemory();
		}
		else {
			if (Exist(inode2, filename) == true) {
				string tmps;
				tmps = "  Failed: The File already existed!\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
				return;
			}
			CopyLxfs(filename, inode1, inode2);
		}
	}

	Writer2();
	return;
}
void Del() {//del path
	Writer1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	FindAbsolutePath(order.od[1]);
	unsigned int inode = FindFileINode(order.od[1]);
	if (inode == INF) {
		string tmps;
		tmps = "  Failed: The path is not exist!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
	}
	else {
		unsigned int tmp = inodetb.inode[inode].last_pos;
		RemoveFile(inode);
		string tmps;
		tmps = "  Del order executed successfully!\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps.c_str());
		WriteShareMemory();
		CurrentPath = tmp;
	}

	Writer2();
	return;
}
void Check() {//check
	Writer1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	WriteFileSystem();
	string tmps;
	tmps = "  Check order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps.c_str());
	WriteShareMemory();

	Writer2();
	return;
}
void Ls() {//ls
	Reader1();

	ReadFileSystem();//��ǰ���̴洢���ڴ��е����ݿ�����ϵͳ��һ�£��������̿��ܶ��ļ���ɾ�ģ�
	ShowList();

	Reader2();
	return;
}

void GetUser() {
	hMapFileUser = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		NameUser);
	if (hMapFileUser == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not create file mapping object (%d).\n"), error);
		return;
	}

	// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬��ȡ���������
	pBufUser = (User*)MapViewOfFile(hMapFileUser,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);
	if (pBufUser == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not map view of file (%d).\n"), error);
		CloseHandle(hMapFileUser);
		return;
	}

	//����û���
	char tmpc[50] = "GET";
	while (true) {
		string tmps = string(pBufUser->name);
		if (tmps != "NONE") {
			strcat_s(NameIn, pBufUser->name);
			strcat_s(NameOut, pBufUser->name);
			strcat_s(NameIoo, pBufUser->name);
			strcpy_s(CurrentUser, pBufUser->name);
			Sleep(100);//�ȴ�0.1s, ��shell�˼����NameIn��NameOut��NameIoo
			for (int i = 0; i < 50; i++)
				pBufUser->name[i] = tmpc[i];

			break;
		}
		else {//����Ҫ�ȴ�һ��ʱ�䣬����shell���������ֻ���ȡ����һ���ַ��ı�ͽ���if
			Sleep(50);
		}
	}

	//�ر�
	UnmapViewOfFile(pBufUser);
	CloseHandle(hMapFileUser);

	Sleep(10);
	return;
}

void InitRW() {
	hMapFileRW = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BUF_SIZE,
		NameRW);
	if (hMapFileRW == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not create file mapping object (%d).\n"), error);
		return;
	}

	pBufRW = (ReaderWriter*)MapViewOfFile(hMapFileRW,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);
	if (pBufRW == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not map view of file (%d).\n"), error);
		CloseHandle(hMapFileRW);
		return;
	}

	if (pBufRW->first == true) return;


	pBufRW->first = true;

	pBufRW->rw = 1;
	pBufRW->mutex = 1;
	pBufRW->count = 0;
	
	for (int i = 0; i < 50; i++) {
		pBufRW->wakeup1[i] = '\0';
		pBufRW->wakeup2[i] = '\0';
	}
	pBufRW->cnt1 = 0; pBufRW->cnt2 = 0;
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 50; j++) {
			pBufRW->wait1[i][j] = '\0';
			pBufRW->wait2[i][j] = '\0';
		}
	}

	return;
}

void Prw() {
	if (pBufRW->rw > 0) {//�ź�����Ч�����Լ���ִ��
		pBufRW->rw = pBufRW->rw - 1;
	}
	else {//�ź�����Ч
		//�˽��̽���ȴ��б�
		for (int i = 0; i < 50; i++) {
			pBufRW->wait1[pBufRW->cnt1][i] = CurrentUser[i];
		}
		pBufRW->cnt1++;

		//�ȴ�����
		while (true) {
			if (strcmp(pBufRW->wakeup1, CurrentUser) == 0) {//����
				pBufRW->rw = pBufRW->rw - 1;
				for (int i = 0; i < 50; i++) {
					pBufRW->wakeup1[i] = '\0';
				}
				return;
			}
			else {
				Sleep(1000);
				string tmps;
				tmps = "  waiting...\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
			}
		}
	}

	return;
}
void Vrw() {
	if (pBufRW->cnt1 == 0) {//û�еȴ��Ľ���
		pBufRW->rw = pBufRW->rw + 1;
	}
	else {//�еȴ��Ľ��̣�����һ�������µȴ��б�
		pBufRW->rw = pBufRW->rw + 1;
		for (int i = 0; i < 50; i++) {
			pBufRW->wakeup1[i] = pBufRW->wait1[0][i];
		}
		pBufRW->cnt1 = pBufRW->cnt1 - 1;
		for (int i = 0; i < pBufRW->cnt1; i++) {
			for (int j = 0; j < 50; j++) {
				pBufRW->wait1[i][j] = pBufRW->wait1[i + 1][j];
			}
		}
	}

	return;
}

void Pmutex() {
	if (pBufRW->mutex > 0) {//�ź�����Ч�����Լ���ִ��
		pBufRW->mutex = pBufRW->mutex - 1;
	}
	else {//�ź�����Ч
		//�˽��̽���ȴ��б�
		for (int i = 0; i < 50; i++) {
			pBufRW->wait2[pBufRW->cnt2][i] = CurrentUser[i];
		}
		pBufRW->cnt2++;

		//�ȴ�����
		while (true) {
			if (strcmp(pBufRW->wakeup2, CurrentUser) == 0) {//����
				pBufRW->mutex = pBufRW->mutex - 1;
				for (int i = 0; i < 50; i++) {
					pBufRW->wakeup2[i] = '\0';
				}
				return;
			}
			else {
				Sleep(1000);
				string tmps;
				tmps = "  waiting...\n";
				smo.cnt = 1;
				strcpy_s(smo.str[0], tmps.c_str());
				WriteShareMemory();
			}
		}
	}
}
void Vmutex() {
	if (pBufRW->cnt2 == 0) {//û�еȴ��Ľ���
		pBufRW->mutex = pBufRW->mutex + 1;
	}
	else {//�еȴ��Ľ��̣�����һ�������µȴ��б�
		pBufRW->mutex = pBufRW->mutex + 1;
		for (int i = 0; i < 50; i++) {
			pBufRW->wakeup2[i] = pBufRW->wait2[0][i];
		}
		pBufRW->cnt2 = pBufRW->cnt2 - 1;
		for (int i = 0; i < pBufRW->cnt2; i++) {
			for (int j = 0; j < 50; j++) {
				pBufRW->wait2[i][j] = pBufRW->wait2[i + 1][j];
			}
		}
	}

	return;
}


void Writer1() {
	Prw();
	return;
}
void Writer2() {
	Vrw();
	return;
}


void Reader1() {
	Pmutex();
	if (pBufRW->count == 0)
		Prw();
	pBufRW->count = pBufRW->count + 1;
	Vmutex();

	return;
}
void Reader2() {
	Pmutex();
	pBufRW->count = pBufRW->count - 1;
	if (pBufRW->count == 0)
		Vrw();
	Vmutex();

	return;
}

void Run() {//���г���
	//step1: ��ʾ������Ϣ
	string tmps[20];
	tmps[0] = "++++++++++++++++++++++++++++++++++++++++++++++\n";
	tmps[1] = "++              OS Course Design            ++\n";
	tmps[2] = "++        Chen Xingchen    201736612486     ++\n";
	tmps[3] = "++            Teacher       Liu Fagui       ++\n";
	tmps[4] = "++++++++++++++++++++++++++++++++++++++++++++++\n";
	smo.cnt = 5;
	for (int i = 0; i < 5; i++) {
		strcpy_s(smo.str[i], tmps[i].c_str());
	}
	WriteShareMemory();

	//step2: �鿴��û�н��������ļ�ϵͳ����û�и�����ʼ��ѡ�����������ϵͳ
	bool check = FindDisk();
	if (check == false) {
		tmps[0] = "  Can not find Linux FileSystem, do you need initialization?  Y/N\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps[0].c_str());
		WriteShareMemory();

		string s;
		ReadShareMemory();
		s = smi.str[0];

		if (s == "y" || s == "Y" || s == "yes" || s == "YES" || s == "Yes") {
			CreateFileSystem();
			tmps[0] = "  Init order executed successfully!\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
		}
		else {
			tmps[0] = "  The program will terminate after 2 seconds...\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
			return;
		}
	}
	else {
		tmps[0] = "  Entering the FileSystem...\n";
		smo.cnt = 1;
		strcpy_s(smo.str[0], tmps[0].c_str());
		WriteShareMemory();
		ReadFileSystem();
	}

	//step3: �����Ļ ����������ʾ ���õ�ǰ·��Ϊ��Ŀ¼
	ShowHelp();
	CurrentPath = RootDir;

	//step4: ѭ���������ֱ������exit
	while (true) {
		ShowPath(); 
		FindOrder(order);//�������
		
		switch (order.type) {
		case 0:		//Exit
			tmps[0] = "  Exiting the FileSystem...\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
			return;

		case 1:		//info
			Info();
			break;
		case 2:		//cd path
			Cd();
			break;
		case 3:		//dir �� dir s �� dir path �� dir path s
			Dir();
			break;
		case 4:     //md dirname �� md dirname path
			Md();
			break;
		case 5:     //rd path
			Rd();
			break;
		case 6:     //newfile filename �� newfile filename path
			Newfile();
			break;
		case 7:
			Cat();
			break;
		case 8:     //copy<host> D:\xxx\yyy\zzz /aaa/bbb �� copy<lxfs> /xxx/yyy/zzz /aaa/bbb
			Copy();
			break;
		case 9:     //del path
			Del();
			break;
		case 10:    //check
			Check();
			break;
		case 11:
			Ls();
			break;

		default:
			tmps[0] = "  The code is wrong, please re-enter the order again!...\n";
			smo.cnt = 1;
			strcpy_s(smo.str[0], tmps[0].c_str());
			WriteShareMemory();
		}
	}

	tmps[0] = "  Exit order executed successfully!\n";
	smo.cnt = 1;
	strcpy_s(smo.str[0], tmps[0].c_str());
	WriteShareMemory();

	return;
}

int main() {
	srand((unsigned int)time(NULL));
	
	/*
	RandMd();
	RandNewfile();
	RandCopyhost();
	RandCopylinux();
	Delete();
	RandMd();
	RandNewfile();
	RandCopyhost();
	RandCopylinux();
	Print();
	*/


	//��������д�߹����ڴ�
	InitRW();

	//��Shell��������
	GetUser();

	//���������ļ�
	hMapFileIoo = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BUF_SIZE,
		NameIoo);
	if (hMapFileIoo == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not create file mapping object (%d).\n"), error);
		return 0;
	}

	// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
	pBufIoo = (InputOrOutput*)MapViewOfFile(hMapFileIoo,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);
	if (pBufIoo == NULL) {
		int error = GetLastError();
		_tprintf(TEXT("Could not map view of file (%d).\n"), error);
		CloseHandle(hMapFileIoo);
		return 0;
	}

	//���г���
	Run();

	//ж���ڴ�ӳ���ļ���ַָ��
	UnmapViewOfFile(pBufIoo);
	//�ر��ڴ�ӳ���ļ�
	CloseHandle(hMapFileIoo);

	return 0;
}