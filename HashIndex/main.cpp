
#include<string>
#include<cstdio>
#include<fstream>
#include<stack> 
#include<cstring>
#include<time.h>
#include<iostream>
using namespace std;

#define num  2147483648;
int blockNum = 1;
int BSzie=1;

struct myData
{
	unsigned  Index;

	char Data[124];
}; 

struct myHashBlock
{
	int DataNUM;

	int CheckNUM;

	myData *DataArray;

	int nextBlockNum;
};

struct myHashNode
{
	int BlockNum;

	myHashNode *Parent,*rChild,*lChild;
};

class myHashIndex
{
private:
	myHashNode *root;
	int bufferSize;
public:

	char TreeFile[10],DataFile[10];
	//char *buffer;

	myHashIndex(int bSize)
	{
		root=NULL;
		bufferSize = bSize;
		strcpy(TreeFile ,"sTree.txt");
		strcpy(DataFile , "sData.dat");
	}

	~myHashIndex()
	{
		//saveTree();
		//delete []buffer;
		if(root!=NULL)
			deleteTree(root);
	}

	void deleteTree(myHashNode *current)
	{
		if(current->lChild!=NULL)
			deleteTree(current->lChild);
		if(current->rChild!=NULL)
			deleteTree(current->rChild);
		delete current;
	}
	//保存二叉排序树
	bool saveTree()
	{
		stack<myHashNode*>s;  //开一个栈用于保存右节点地址
		myHashNode *p=root;
		s.push(NULL);
		ofstream out(TreeFile,ios::out);
		if(!out)
		{
			cout<<"Output file open failed";
			return false;
		}
		while(p!=NULL||!s.empty())
		{
			while(p!=NULL)
			{
				out<<p->BlockNum<<"   ";
				s.push(p);
				p=p->lChild;
			}
			out<<"#";
			if(!s.empty())
			{
				p=s.top();s.pop();
				p=p->rChild;
				while(p==NULL&&!s.empty())
				{
					out<<"#";
					p=s.top();s.pop();
					if(p!=NULL)
						p=p->rChild;
				}
			}
		}
		out<<"#\n";
		out.close();
		return true;
	}
	//读取保存于外存的二叉排序树
	bool readTree()
	{
		ifstream inData(TreeFile, ios::in);//打开文件.txt
		if (!inData)//判断打开文件是否成功
			return false;
		readTree(root,NULL,inData);
		inData.close();
		return true;
	}

	bool readTree(myHashNode *&current,myHashNode *parent,ifstream &inData )
	{
		char w;
		int bNum;
		if(!inData.eof())
		{
			inData>>w;
			if(w!='#')
			{
				inData.seekg(-1,ios::cur);
				inData>>bNum;
				current = new myHashNode;
				current->Parent = parent;
				current->BlockNum = bNum;
				readTree(current->lChild,current,inData);
				readTree(current->rChild,current,inData);
			}
			else
			{
				current = NULL;
			}
		}

		return true;

	}
	//读入块
	myHashBlock*& readBlock(int bNUM)
	{
		char buffer[1024*2];
		char *p = buffer;
		//新建桶
		myHashBlock *pB = NULL;
		pB = new myHashBlock;
		pB->CheckNUM = 0; 
		pB->DataNUM = bufferSize;
		pB->DataArray = new myData[bufferSize];
		pB->nextBlockNum = -1;
		//读入对应块
		ifstream in(DataFile,ios::in||ios::binary);
		if(!in)
		{
			cerr<<"Input file open failed";
			return pB;
		}
		in.seekg((bNUM-1)*1024*BSzie,ios::beg);
		in.read(buffer,1024*BSzie);
		in.close();
		//解析二进制块
		memcpy(&(pB->CheckNUM),p+8,4);
		memcpy(&(pB->DataNUM),p+12,4);
		memcpy(&(pB->nextBlockNum),p+16,4);
		p = p+20;
		int i,max = pB ->CheckNUM; 
		for( i=0;i<max;i++)
		{
			memcpy(&(pB->DataArray[i].Index),p,4);
			memcpy((pB->DataArray[i].Data),p+4,124);
			p = p + 128;
		}
		//返回解析完数据
		return pB;
	}
	//写出块
	bool writeBlock(int bNUM,myHashBlock *pB)
	{
		char buffer[1024*2];
		char *P;
		//将数据以二进制形式写入缓冲区
		P = buffer;
		memcpy(P+8,&(pB->CheckNUM),4);
		memcpy(P+12,&(pB->DataNUM),4);
		memcpy(P+16,&(pB->nextBlockNum),4);
		P = P+20;
		int i,max = pB ->CheckNUM; 
		for(i=0;i<max;i++)
		{
			memcpy(P,&(pB->DataArray[i].Index),4);
			memcpy(P+4,(pB->DataArray[i].Data),124);
			P = P + 128;
		}
		//写出块
		ofstream out(DataFile,ios::out||ios::binary||ios::ate);
		if(!out)
		{
			cout<<"Output file open failed";
			return false;
		}
		out.seekp((bNUM-1)*1024*BSzie,ios::beg);
		out.write(buffer,1024*BSzie);
		out.close();
		return true;

	}

	bool Insert(myData data)
	{
		myHashBlock *pB;
		myHashNode *current;
		int i;
		//判断跟节点是否为空，新建第一个Hash块
		if(root==0)
		{
			pB = new myHashBlock;
			pB->CheckNUM = 0;
			pB->DataNUM = bufferSize;
			pB->DataArray = new myData[bufferSize];
			pB->nextBlockNum = -1;
			root = new myHashNode;
			root->BlockNum = blockNum;
			blockNum++;
			root->lChild=root->rChild=root->Parent=NULL;
			pB->DataArray[pB->CheckNUM]=data;
			pB->CheckNUM++;
			writeBlock(root->BlockNum,pB);
			delete []pB->DataArray;
			delete pB;
			return true;
		}

		//当根节点指向Hash块时，在根节点所指向的块中填入数据知道满，则新建新左右节点
		unsigned index = num;
		if(root->BlockNum >=0 )
		{
			pB =readBlock(root->BlockNum);
			//未满，持续插入
			if(pB->CheckNUM<(pB->DataNUM))
			{
				pB->DataArray[pB->CheckNUM]=data;
				pB->CheckNUM++;
				writeBlock(root->BlockNum,pB);
			}
			else
			{
				//满则调用分拆函数，新建其新的左右结点
				bool f1 = setNewNode(root,data,index);
				delete []pB->DataArray;
				delete pB;
				return f1;
			}

			delete []pB->DataArray;
			delete pB;
			return true;
		}

		//当根节点不指向Hash时，扫描Data的index，搜索root所指向的树，找到Data的归属Hash块
		current = root;
		
		for(i=0;i<32;i++,index=index>>1)
		{
			if((data.Index & index)==0)
				if(current->lChild!=NULL)
					current = current->lChild;
				else
				{
					//为空的时候新建块
					pB = new myHashBlock;
					pB->CheckNUM = 0;
					pB->DataNUM = bufferSize;
					pB->DataArray = new myData[bufferSize];
					pB->nextBlockNum = -1;
					current->lChild = new myHashNode;
					current->lChild->BlockNum = blockNum;
					blockNum++;
					current->lChild->lChild=current->lChild->rChild=NULL;
					current->lChild->Parent = current;
					current = current->lChild;
					break;
				}
			else
				if(current->rChild!=NULL)
					current = current->rChild;
				else
				{
					//为空的时候新建块
					pB = new myHashBlock;
					pB->CheckNUM = 0;
					pB->DataNUM = bufferSize;
					pB->DataArray = new myData[bufferSize];
					pB->nextBlockNum = -1;
					current->rChild = new myHashNode;
					current->rChild->BlockNum = blockNum;
					blockNum++;
					current->rChild->lChild=current->rChild->rChild=NULL;
					current->rChild->Parent = current;
					current = current->rChild;
					break;
				}
			if(current->BlockNum>=0)
			{
				//不为空的时候读入块
				pB = readBlock(current->BlockNum);
				break;
			}
		}
		
		if(pB->CheckNUM<(pB->DataNUM))
		{
			pB->DataArray[pB->CheckNUM]=data;
			pB->CheckNUM++;
			writeBlock(current->BlockNum,pB);
		}
		else
		{
			//满则调用分拆函数，新建其新的左右结点
			index = index>>1;
			bool f2 = setNewNode(current,data,index);
			delete []pB->DataArray;
			delete pB;
			return f2;
		}
		delete []pB->DataArray;
		delete pB;
		return true;
	
	}

	bool DeleteAll(unsigned Index)
	{
		myHashNode *current;
		myHashBlock *pB,*p ;
		unsigned index = num;
		int i;
		//判断根节点是否为空
		if(root == NULL)
			return false;
		bool Flag1 =false;
		bool Flag2 = false;
		//当根节点指向Hash块时，扫描该块，得到是否有删除数据
		if(root->BlockNum >=0 )
		{
			pB = readBlock( root->BlockNum);
			for(i=0;i<=pB->CheckNUM;i++)
				if(pB->DataArray[i].Index == Index)
				{
					deleteData(i,pB);
					writeBlock(root->BlockNum,pB);
					Flag1 = true;
					//return true;
				}
			if(Flag1)
			{
				delete []pB->DataArray;
				delete pB;
				return true;
			}
			else
			{
				delete []pB->DataArray;
				delete pB;
				return false;
			}
			
		}

		//当根节点不指向Hash时，扫描Data的index，搜索root所指向的树，找到Data的归属Hash块
		current = root;
		
		for(i=0;i<32;i++,index=index>>1)
		{
			if((Index & index)==0)
				if(current->lChild!=NULL)
					current = current->lChild;
				else
					return false;
			else
				if(current->rChild!=NULL)
					current = current->rChild;
				else
					return false;
			if(current->BlockNum>=0)
				break;
		}
		//持续扫描所得块，及其后续块，删除所有
		pB = readBlock(current->BlockNum) ;
		if(pB->nextBlockNum != -1)
			Flag2 = true;
		while (pB != NULL)
		{
			for(i=0;i<=pB->CheckNUM;i++)
			if(pB->DataArray[i].Index == Index)
			{
				deleteData(i,pB);
				writeBlock(current->BlockNum,pB);
				if(pB->CheckNUM<0)
					current->BlockNum = pB->nextBlockNum;
				if(!Flag2)
					checkCombine(current->Parent);
				delete []pB->DataArray;
				delete pB;
				return true;
			}

			if(pB->nextBlockNum!=-1)
			{
				p = pB;
				pB=readBlock(pB->nextBlockNum);
				delete []p->DataArray;
				delete p;
			}
			else
			{
				delete []pB->DataArray;
				delete pB;
				pB = NULL;
			}
		}

	}

	myData* Check(unsigned Index)
	{
		myHashNode *current;
		myData *pD = new myData;
		myHashBlock *pB ,*p;
		unsigned index = num;
		int i;
		//判断根节点是否为空
		if(root == NULL)
			return NULL;
		//当根节点指向Hash块时，扫描该块，得到是否有所检查数据
		if(root->BlockNum >=0 )
		{
			pB =readBlock( root->BlockNum );
			for(i=0;i<=pB->CheckNUM;i++)
				if(pB->DataArray[i].Index == Index)
				{
					*pD = pB->DataArray[i];
					delete []pB->DataArray;
					delete pB;
					return pD;
				}
			delete []pB->DataArray;
			delete pB;
			return NULL;
		}

		//当根节点不指向Hash时，扫描Data的index，搜索root所指向的树，找到Data的归属Hash块
		current = root;
		
		for(i=0;i<32;i++,index=index>>1)
		{
			if((Index & index)==0)
				if(current->lChild!=NULL)
					current = current->lChild;
				else
					return NULL;
			else
				if(current->rChild!=NULL)
					current = current->rChild;
				else
					return NULL;
				
			if(current->BlockNum>=0)
				break;
		}
		//扫描所得块，判断index，如果相同，则返回第一个找到值
		pB = readBlock(current->BlockNum) ;
		while (pB != NULL)
		{
			for(i=0;i<=pB->CheckNUM;i++)
			if(pB->DataArray[i].Index == Index)
			{
				*pD = pB->DataArray[i];
				delete []pB->DataArray;
				delete pB;
				return pD;
			}
			if(pB->nextBlockNum!=-1)
			{
				p = pB;
				pB=readBlock(pB->nextBlockNum);
				delete []p->DataArray;
				delete p;
			}
			else
			{
				delete []pB->DataArray;
				delete pB;
				pB = NULL;
			}
		}
		return NULL;
	}

	bool setNewNode(myHashNode *&node,myData data,unsigned index)
	{
		myHashBlock *pL,*pR,*p;
		if(index!=0)
		{
			//创建新子树Hash块
			pL = new myHashBlock;
			pL->CheckNUM = 0;
			pL->DataNUM = bufferSize;
			pL->DataArray = new myData[bufferSize];
			pL->nextBlockNum = -1;
			pR = new myHashBlock;
			pR->CheckNUM = 0;
			pR->DataNUM = bufferSize;
			pR->DataArray = new myData[bufferSize]();
			pR->nextBlockNum = -1;
			//创建新Node指向性Hash块，并将父节点的Hash块指针置为NULL
			node->lChild = new myHashNode;
			node->rChild = new myHashNode;
			node->lChild->BlockNum = node->BlockNum;
			node->rChild->BlockNum = blockNum;
			blockNum++;
			p = readBlock( node->BlockNum);
			node->BlockNum = -1;

			node->lChild->Parent=node->rChild->Parent=node;
			node->lChild->lChild=node->rChild->lChild=node->lChild->rChild=node->rChild->rChild=NULL;
			//重新分配原Hash中数据，即pR指向的块
			myData *pD;
			
			for(int i=0;i<p->DataNUM;i++)
			{
				pD=&p->DataArray[i];
				//cout<<pD->Index<<"   ";
				if((pD->Index & index)==0)
				{
					pL->DataArray[pL->CheckNUM] = *pD;
					pL->CheckNUM++;
				}
				else
				{
					pR->DataArray[pR->CheckNUM] = *pD;
					pR->CheckNUM++;
				}
			}
			delete []p->DataArray;
			delete p;

			//为新数据在其所属的Hash块中填入，如果偏向某一边，则递归实现，直到最后
			if((data.Index & index)==0)
			{
				if(pL->CheckNUM<pL->DataNUM)
				{
					pL->DataArray[pL->CheckNUM++] = data;
				}
				else
				{
					index = index>>1;
					delete []pR->DataArray;
					delete pR;
					blockNum--;
					delete node->rChild;
					node->rChild = NULL;
					return setNewNode(node->lChild,data,index);
				}

			}
			else
			{
				if(pR->CheckNUM<pR->DataNUM)
				{
					pR->DataArray[pR->CheckNUM++] = data;
				}
				else
				{

					index = index>>1;
					delete []pL->DataArray;
					delete pL;
					node->rChild->BlockNum = node->lChild->BlockNum;
					blockNum--;
					delete node->lChild;
					node->lChild = NULL;
					return setNewNode(node->rChild,data,index);
				}
				
			}
			writeBlock(node->lChild->BlockNum,pL);
			writeBlock(node->rChild->BlockNum,pR);
			delete []pL->DataArray;
			delete pL;
			delete []pR->DataArray;
			delete pR;
		}
		else
		{
			//当所利用的散列位数等于N的位数，建立链表块
			p = new myHashBlock;
			p->CheckNUM = 0;
			p->DataNUM = bufferSize;
			p->DataArray = new myData[bufferSize];
			p->nextBlockNum = node->BlockNum;
			node->BlockNum = blockNum;
			blockNum++;
			p->DataArray[p->CheckNUM++] = data;
			writeBlock(node->BlockNum,p);
			delete []p->DataArray;
			delete p;
		}
	return true;
	}
	//删除某一项数据
	void deleteData(int current,myHashBlock *&pB)
	{
		int i;
		for(i=current;i<pB->CheckNUM;i++)
			if(i<pB->DataNUM-1)
				pB->DataArray[i]= pB->DataArray[i+1];
		pB->CheckNUM--;
		return ;
	}
	//判断删除某一项以后，其父节点下两个节点是否可以合并
	void checkCombine(myHashNode *parent)
	{
		int i;
		myHashBlock *pBL,*pBR ;
		pBL = readBlock(parent->lChild->BlockNum);
		pBR = readBlock(parent->rChild->BlockNum);
		if((pBL->CheckNUM+pBR->CheckNUM+2)>pBL->DataNUM)
			return ;
		parent->lChild = parent->rChild = NULL;
		parent->BlockNum = parent->lChild->BlockNum;
		for(i=0;i<pBR->CheckNUM;i++)
		{
			pBL->DataArray[pBL->CheckNUM+1] = pBR->DataArray[i];
			pBL->CheckNUM++;
		}
		writeBlock(parent->BlockNum,pBL);
		delete []pBR->DataArray;
		delete pBR;
		return ;
	}
};

int main()
{
	int choose,buSize=8;
	unsigned i,randNum,delIndex,cheIndex,j;
	string randData,s1,s2;
	bool flag,flag1 = false,flag2 = false;
	myData data1,data2,data3,*data4;
	myHashIndex myHash(buSize);
	BSzie = 2;
	ofstream out;
	char buf[10];
	clock_t start,finish;
	double duration;
	while(11)
	{
		system("cls");
		cout<<"Please choose the operation of the HashIndex:\n";
		cout<<"1.Set small number data test from newly created data\n";
		cout<<"2.Set small number data test from exit data\n";
		cout<<"3.Set great number data test from newly created data\n";
		cout<<"4.Set great number data test from exit data\n";
		cout<<"5.Save tree and Exit\n";
		cin>>choose;
		
		switch(choose)
		{
		case 1: 
			out.open("sData.dat",ios::out);
			if(!out)
			{
				cout<<"Output file open failed";
			
			}
			out.close();
			srand( (unsigned)time(NULL));
			myHash.TreeFile[0] = 's';
			myHash.DataFile[0] = 's';
			for(i=1,j=-1;i<500;i++,j--)
			{
				randNum = rand();
				data1.Index = randNum;
				sprintf(buf,"%d",randNum);
				strcpy(data1.Data ,"cai+"); 
				strcat(data1.Data,buf);
				myHash.Insert(data1);
			}
			break;
		case 2:
			myHash.TreeFile[0] = 's';
			myHash.DataFile[0] = 's';
			myHash.readTree();
			break;
		case 3:
			out.open("gData.dat",ios::out);
			if(!out)
			{
				cout<<"Output file open failed";
			
			}
			out.close();
			srand( (unsigned)time(NULL));
			myHash.TreeFile[0] = 'g';
			myHash.DataFile[0] = 'g';
			for(i=1;i<50000;i++)
			{
				randNum = i;
				data1.Index = randNum;
				sprintf(buf,"%d",randNum);
				strcpy(data1.Data ,"cai+"); 
				strcat(data1.Data,buf);
				myHash.Insert(data1);
			}
			break;
		case 4:
			myHash.TreeFile[0] = 'g';
			myHash.DataFile[0] = 'g';
			myHash.readTree();
			break;
		case 5:
			flag1 = true ;
			break;
		default:
			cout<<"Input Error!";
			continue;
		}
		if(flag1)
			break;
		flag2 = false;
		while(22)
		{
			system("cls");
			cout<<"Please choose the detail of the operation:\n";
			cout<<"1.Insert data\n";
			cout<<"2.Delete data\n";
			cout<<"3.Check data\n";
			cout<<"4.Save and exit\n";
			cin>>choose;
			switch (choose)
			{
			case 1:
				cout<<"Please input the index and the data:\n";
				cin>>data3.Index>>data3.Data;
				start=clock();
				flag = myHash.Insert(data3);
				if(flag)
					cout<<"Insert successed!\n";
				else 
					cout<<"Insert failed!\n";
				finish=clock();
				break;
			case 2:
				cout<<"Please input the index of the data which will be deleted:\n";
				cin>>delIndex;
				start=clock();
				flag = myHash.DeleteAll(delIndex);
				if(flag)
					cout<<"Delete successed!\n";
				else 
					cout<<"Delete failed!\n";
				finish=clock();
				break;
			case 3:
				cout<<"Please input the index of the data which will be checked:\n";
				cin>>cheIndex;
				start=clock();
				data4 = myHash.Check(cheIndex);
				if(data4!=NULL)
				{
					cout<<"The detail data of the checked is :\n";
					cout<<"Index is "<<data4->Index<<", data is "<<data4->Data<<endl;
				}
				else
				{
					cout<<"Check failed!\n";
				}
				finish=clock();
				break;
			case 4:
				start=clock();
				myHash.saveTree();
				flag2 = true;
				finish=clock();
				break;
			default:
				cout<<"Input Error!\n";
				break;
			}
			if(flag2)
				break;
			duration=(double)(finish-start);
			cout<<"The used of the time is "<<duration <<" ms!"<<endl;
			system("pause");
		}
	}
	return 0;
}