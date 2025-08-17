#include<vector>
#include<string>
#include<cstdlib>
#include<unistd.h>
#include<iostream>
using namespace std;

#define _Fault 0
#define _Notice_LMD 1	//low matarial drew
#define _Notice_CKM 2	//checkmate
#define _Notice_STM 3	//stalemate
#define _Notice_CHK 4	//check
#define _Notice_CFA 5	//confirm again or not
#define _Notice_UKE	6	//unknown error

#define _WHITE 0
#define _BLACK 1

#define _TP 1
#define _TB 2
#define _TN 3
#define _TR 4
#define _TQ 5
#define _TK 6

#define _CTR_N 0
#define _CTR_W 1
#define _CTR_B 2
#define _CTR_D 3	// (2-COLOR)=OPT_CTRL,(1+COLOR=OWN_CTRL)

#define _DASH 0
#define _CSTL 1
#define _PRMT 2

/*struct MOVE{
  int rd;
  int type;
  int spx,spy;
  int opx,opy;
  bool is_capture;
  int cap_pos;
  bool is_check;
  bool is_promote;
  };*/

struct PIECE{
	int type;
	int cl;
	bool moved;
};

struct POS{	//board[x][y] <==> yx
	int x;	//1-8(0-7)
	int y;	//a-h(0-7)
	bool operator == (const POS a)const{
		return (x==a.x && y==a.y);
	}
};

struct SPMV{
	int type;
	int stp;
	POS dash;
	int prmt;
	int castling;
};

int steps;			//steps%2 0->W 1->B
vector <SPMV> splog;
int board[8][8];
PIECE pieces[32+1];
int ctrlpos[8][8];

void note(int type)
{
	switch(type)
	{
		case _Fault:
//			cout<<"Violation!"<<endl;
			cout<<"你不能这么做:请检查走子是否符合规则或王是否会被攻击"<<endl;
			break;
		case _Notice_LMD:
//			cout<<"low matarial drew"<<endl;
			cout<<"子力不足,和棋"<<endl;
			break;
		case _Notice_CKM:
//			cout<<"Checkmate!"<<endl;
			cout<<"将死!"<<endl;
			break;
		case _Notice_STM:
//			cout<<"Stalemate!"<<endl;
			cout<<"逼和(无子可走)!"<<endl;
			break;
		case _Notice_CHK:
//			cout<<"Check"<<endl;
			cout<<"将军!"<<endl;
			break;
		case _Notice_CFA:
//			cout<<"Again?(press 'y' to start again, press any other key to quit)"<<endl;
			cout<<"重开?(按y重开,任意其他键结束)"<<endl;
			break;
		case _Notice_UKE:
//			cout<<"Unknown Error,press any key to quit"<<endl;
			cout<<"未知错误,即将退出"<<endl;
			sleep(5);
			exit(0);
			break;
	}
}



void init()
{
	steps=0;

	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			board[i][j]=0;
	for(int i=0;i<=32;i++) pieces[i]={0,0,0};

	//R
	pieces[1].type=pieces[8].type=pieces[25].type=pieces[32].type=_TR;
	board[0][0]=1;board[0][7]=8;board[7][0]=25;board[7][7]=32;

	//N
	pieces[2].type=pieces[7].type=pieces[26].type=pieces[31].type=_TN;
	board[0][1]=2;board[0][6]=7;board[7][1]=26;board[7][6]=31;

	//B

	pieces[3].type=pieces[6].type=pieces[27].type=pieces[30].type=_TB;
	board[0][2]=3;board[0][5]=6;board[7][2]=27;board[7][5]=30;

	//Q
	pieces[4].type=pieces[28].type=_TQ;
	board[0][3]=4;board[7][3]=28;

	//K
	pieces[5].type=pieces[29].type=_TK;
	board[0][4]=5;board[7][4]=29;


	for(int i=0;i<8;i++)
	{
		pieces[8+i+1].type=_TP;
		board[1][i]=8+i+1;
	}
	for(int i=0;i<8;i++)
	{
		pieces[16+i+1].type=_TP;
		board[6][i]=16+i+1;
	}

	for(int i=1;i<=16;i++)
		pieces[i].cl=_WHITE;
	for(int i=17;i<=32;i++)
		pieces[i].cl=_BLACK;
}

bool in(int x,int y)
{
	if(x<8 && y<8 && x>=0 && y>=0)
		return true;
	return false;
}

bool _check(int cl)
{
	POS kp;
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			if(pieces[board[i][j]].type==_TK && pieces[board[i][j]].cl==cl)
				kp.x=i,kp.y=j;
	int direct=1-(2*cl);
	if((in(kp.x+direct,kp.y+1) && pieces[board[kp.x+direct][kp.y+1]].cl^cl && pieces[board[kp.x+direct][kp.y+1]].type==_TP)\
	|| (in(kp.x+direct,kp.y-1) && pieces[board[kp.x+direct][kp.y-1]].cl^cl && pieces[board[kp.x+direct][kp.y-1]].type==_TP))
		return true;
	
	for(int i=-1;i<=1;i++)
		for(int j=-1;j<=1;j++)
		{
			if(i==0 && j==0)
				continue;
			if(in(kp.x+i,kp.y+j) && board[kp.x+i][kp.y+j] && pieces[board[kp.x+i][kp.y+j]].type==_TK)
				return true;
		}
	for(int i=1;1;i++)
	{
		if(!in(kp.x+i,kp.y+i))
			break;
		if(board[kp.x+i][kp.y+i])
		{
			if(pieces[board[kp.x+i][kp.y+i]].cl^cl && (pieces[board[kp.x+i][kp.y+i]].type==_TB || pieces[board[kp.x+i][kp.y+i]].type==_TQ))
				return true;
			break;
		}
	}
	for(int i=1;1;i++)
    {
        if(!in(kp.x+i,kp.y-i))
            break;
        if(board[kp.x+i][kp.y-i])
        {
            if(pieces[board[kp.x+i][kp.y-i]].cl^cl && (pieces[board[kp.x+i][kp.y-i]].type==_TB || pieces[board[kp.x+i][kp.y-i]].type==_TQ))
                return true;
            break;
        }
    }
	for(int i=1;1;i++)
    {
        if(!in(kp.x-i,kp.y+i))
            break;
        if(board[kp.x-i][kp.y+i])
        {
            if(pieces[board[kp.x-i][kp.y+i]].cl^cl && (pieces[board[kp.x-i][kp.y+i]].type==_TB || pieces[board[kp.x-i][kp.y+i]].type==_TQ))
                return true;
            break;
        }
    }
	for(int i=1;1;i++)
    {
        if(!in(kp.x-i,kp.y-i))
            break;
        if(board[kp.x-i][kp.y-i])
        {
            if(pieces[board[kp.x-i][kp.y-i]].cl^cl && (pieces[board[kp.x-i][kp.y-i]].type==_TB || pieces[board[kp.x-i][kp.y-i]].type==_TQ))
                return true;
            break;
        }
    }

	for(int i=1;1;i++)
    {
        if(!in(kp.x+i,kp.y))
            break;
        if(board[kp.x+i][kp.y])
        {
			if(pieces[board[kp.x+i][kp.y]].cl^cl && (pieces[board[kp.x+i][kp.y]].type==_TR || pieces[board[kp.x+i][kp.y]].type==_TQ))
				return true;
            break;
        }
    }
	for(int i=1;1;i++)
    {
        if(!in(kp.x-i,kp.y))
            break;
        if(board[kp.x-i][kp.y])
        {
            if(pieces[board[kp.x-i][kp.y]].cl^cl && (pieces[board[kp.x-i][kp.y]].type==_TR || pieces[board[kp.x-i][kp.y]].type==_TQ))
                return true;
            break;
        }
    }
	for(int i=1;1;i++)
    {
        if(!in(kp.x,kp.y+i))
            break;
        if(board[kp.x][kp.y+i])
        {
            if(pieces[board[kp.x][kp.y+i]].cl^cl && (pieces[board[kp.x][kp.y+i]].type==_TR || pieces[board[kp.x][kp.y+i]].type==_TQ))
                return true;
            break;
        }
    }
	for(int i=1;1;i++)
    {
        if(!in(kp.x,kp.y-i))
            break;
        if(board[kp.x][kp.y-i])
        {
            if(pieces[board[kp.x][kp.y-i]].cl^cl && (pieces[board[kp.x][kp.y-i]].type==_TR || pieces[board[kp.x][kp.y-i]].type==_TQ))
                return true;
            break;
        }
    }

	if(in(kp.x+1,kp.y+2) && board[kp.x+1][kp.y+2])
	{
		if(cl^pieces[board[kp.x+1][kp.y+2]].cl && pieces[board[kp.x+1][kp.y+2]].type==_TN)
			return true;
	}
	if(in(kp.x+1,kp.y-2) && board[kp.x+1][kp.y-2])
	{
		if(cl^pieces[board[kp.x+1][kp.y-2]].cl && pieces[board[kp.x+1][kp.y-2]].type==_TN)
			return true;
	}
	if(in(kp.x-1,kp.y+2) && board[kp.x-1][kp.y+2])
	{
		if(cl^pieces[board[kp.x-1][kp.y+2]].cl && pieces[board[kp.x-1][kp.y+2]].type==_TN)
			return true;
	}
	if(in(kp.x-1,kp.y-2) && board[kp.x-1][kp.y-2])
	{
		if(cl^pieces[board[kp.x-1][kp.y-2]].cl && pieces[board[kp.x-1][kp.y-2]].type==_TN)
			return true;
	}

	if(in(kp.x+2,kp.y+1) && board[kp.x+2][kp.y+1])
	{
		if(cl^pieces[board[kp.x+2][kp.y+1]].cl && pieces[board[kp.x+2][kp.y+1]].type==_TN)
			return true;
	}
	if(in(kp.x+2,kp.y-1) && board[kp.x+2][kp.y-1])
	{
		if(cl^pieces[board[kp.x+2][kp.y-1]].cl && pieces[board[kp.x+2][kp.y-1]].type==_TN)
			return true;
	}
	if(in(kp.x-2,kp.y+1) && board[kp.x-2][kp.y+1])
	{
		if(cl^pieces[board[kp.x-2][kp.y+1]].cl && pieces[board[kp.x-2][kp.y+1]].type==_TN)
			return true;
	}
	if(in(kp.x-2,kp.y-1) && board[kp.x-2][kp.y-1])
	{
		if(cl^pieces[board[kp.x-2][kp.y-1]].cl && pieces[board[kp.x-2][kp.y-1]].type==_TN)
			return true;
	}


	return false;
}

bool _trymv(POS fp,POS tp,int cl)
{
	bool ret;
	int cn=board[fp.x][fp.y],tn=board[tp.x][tp.y];
	board[fp.x][fp.y]=0;
	board[tp.x][tp.y]=cn;
	ret=(!(_check(cl)));
	board[fp.x][fp.y]=cn;
	board[tp.x][tp.y]=tn;
	return ret;
}

void _promote(POS cur)
{
	int cn=board[cur.x][cur.y];
	char ctype;
	cout<<"请选择升变类型:";
	cin>>ctype;
	switch(ctype)
	{
		case 'B':
		case 'b':
			pieces[cn].type=_TB;
			break;
		case 'N':
		case 'n':
			pieces[cn].type=_TN;
			break;
		case 'R':
		case 'r':
			pieces[cn].type=_TR;
			break;
		case 'Q':
		case 'q':
			pieces[cn].type=_TQ;
			break;
		default:
			cout<<"无效的输入,请重试"<<endl;
			_promote(cur);
			return;
	}
}

void pawn_list(POS cur,int cl,vector <POS> &list)
{
	int direct;
	if(cl==_WHITE)
		direct=1;
	else
		direct=-1;
	if(in(cur.x+direct,cur.y) && board[cur.x+direct][cur.y]==0)
	{
		int cn=board[cur.x][cur.y];
		int direct2=direct+direct;
		if(_trymv(cur,{cur.x+direct,cur.y},cl))
			list.push_back({cur.x+direct,cur.y});
		if(in(cur.x+direct2,cur.y) && board[cur.x+direct2][cur.y]==0 && pieces[cn].moved==false)
			if(_trymv(cur,{cur.x+direct2,cur.y},cl))
				list.push_back({cur.x+direct2,cur.y});
	}
	if(in(cur.x+direct,cur.y+1))
	{
		if(board[cur.x+direct][cur.y+1] && cl^pieces[board[cur.x+direct][cur.y+1]].cl)
			if(_trymv(cur,{cur.x+direct,cur.y+1},cl))
			list.push_back({cur.x+direct,cur.y+1});
		ctrlpos[cur.x+direct][cur.y+1] |= (1+cl);
	}
	if(in(cur.x+direct,cur.y-1))
	{
		if(board[cur.x+direct][cur.y-1] && cl^pieces[board[cur.x+direct][cur.y-1]].cl)
			if(_trymv(cur,{cur.x+direct,cur.y-1},cl))
			list.push_back({cur.x+direct,cur.y-1});
		ctrlpos[cur.x+direct][cur.y-1] |= (1+cl);
	}
	if(splog.size() && splog[splog.size()-1].stp==steps-1 && (splog[splog.size()-1].dash.x==cur.x)\
			&& (splog[splog.size()-1].dash.y==cur.y+1 || splog[splog.size()-1].dash.y==cur.y-1)\
		   	&& _trymv(cur,{splog[splog.size()-1].dash.x+direct,splog[splog.size()-1].dash.y},cl))
		list.push_back({splog[splog.size()-1].dash.x+direct,splog[splog.size()-1].dash.y});

}

void bishop_list(POS cur,int cl,vector <POS> &list)
{
	for(int i=1;1;i++)
	{
		if(!in(cur.x+i,cur.y+i))
			break;
		ctrlpos[cur.x+i][cur.y+i] |= (1+cl);
		if(board[cur.x+i][cur.y+i])
		{
			if(cl^pieces[board[cur.x+i][cur.y+i]].cl && _trymv(cur,{cur.x+i,cur.y+i},cl))
				list.push_back({cur.x+i,cur.y+i});
			break;
		}
		if(_trymv(cur,{cur.x+i,cur.y+i},cl))
			list.push_back({cur.x+i,cur.y+i});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x+i,cur.y-i))
			break;
		ctrlpos[cur.x+i][cur.y-i] |= (1+cl);
		if(board[cur.x+i][cur.y-i])
		{
			if(cl^pieces[board[cur.x+i][cur.y-i]].cl && _trymv(cur,{cur.x+i,cur.y-i},cl))
				list.push_back({cur.x+i,cur.y-i});
			break;
		}
		if(_trymv(cur,{cur.x+i,cur.y-i},cl))
			list.push_back({cur.x+i,cur.y-i});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x-i,cur.y+i))
			break;
		ctrlpos[cur.x-i][cur.y+i] |= (1+cl);
		if(board[cur.x-i][cur.y+i])
		{
			if(cl^pieces[board[cur.x-i][cur.y+i]].cl && _trymv(cur,{cur.x-i,cur.y+i},cl))
				list.push_back({cur.x-i,cur.y+i});
			break;
		}

		if(_trymv(cur,{cur.x-i,cur.y+i},cl))
			list.push_back({cur.x-i,cur.y+i});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x-i,cur.y-i))
			break;
		ctrlpos[cur.x-i][cur.y-i] |= (1+cl);
		if(board[cur.x-i][cur.y-i])
		{
			if(cl^pieces[board[cur.x-i][cur.y-i]].cl && _trymv(cur,{cur.x-i,cur.y-i},cl))
				list.push_back({cur.x-i,cur.y-i});
			break;
		}
		if(_trymv(cur,{cur.x-i,cur.y-i},cl))
			list.push_back({cur.x-i,cur.y-i});
	}

}

void knight_list(POS cur,int cl,vector <POS> &list)
{
	if(in(cur.x+1,cur.y+2))
	{
		if((!board[cur.x+1][cur.y+2] || cl^pieces[board[cur.x+1][cur.y+2]].cl) && _trymv(cur,{cur.x+1,cur.y+2},cl))
			list.push_back({cur.x+1,cur.y+2});
		ctrlpos[cur.x+1][cur.y+2] |= (1+cl);
	}
	if(in(cur.x+1,cur.y-2))
	{
		if((!board[cur.x+1][cur.y-2] || cl^pieces[board[cur.x+1][cur.y-2]].cl) && _trymv(cur,{cur.x+1,cur.y-2},cl))
			list.push_back({cur.x+1,cur.y-2});
		ctrlpos[cur.x+1][cur.y-2] |= (1+cl);
	}
	if(in(cur.x-1,cur.y+2))
	{
		if((!board[cur.x-1][cur.y+2] || cl^pieces[board[cur.x-1][cur.y+2]].cl) && _trymv(cur,{cur.x-1,cur.y+2},cl))
			list.push_back({cur.x-1,cur.y+2});
		ctrlpos[cur.x-1][cur.y+2] |= (1+cl);
	}
	if(in(cur.x-1,cur.y-2))
	{
		if((!board[cur.x-1][cur.y-2] || cl^pieces[board[cur.x-1][cur.y-2]].cl) && _trymv(cur,{cur.x-1,cur.y-2},cl))
			list.push_back({cur.x-1,cur.y-2});
		ctrlpos[cur.x-1][cur.y-2] |= (1+cl);
	}

	if(in(cur.x+2,cur.y+1))
	{
		if((!board[cur.x+2][cur.y+1] || cl^pieces[board[cur.x+2][cur.y+1]].cl) && _trymv(cur,{cur.x+2,cur.y+1},cl))
			list.push_back({cur.x+2,cur.y+1});
		ctrlpos[cur.x+2][cur.y+1] |= (1+cl);
	}
	if(in(cur.x+2,cur.y-1))
	{
		if((!board[cur.x+2][cur.y-1] || cl^pieces[board[cur.x+2][cur.y-1]].cl) && _trymv(cur,{cur.x+2,cur.y-1},cl))
			list.push_back({cur.x+2,cur.y-1});
		ctrlpos[cur.x+2][cur.y-1] |= (1+cl);
	}
	if(in(cur.x-2,cur.y+1))
	{
		if((!board[cur.x-2][cur.y+1] || cl^pieces[board[cur.x-2][cur.y+1]].cl) && _trymv(cur,{cur.x-2,cur.y+1},cl))
			list.push_back({cur.x-2,cur.y+1});
		ctrlpos[cur.x-2][cur.y+1] |= (1+cl);
	}
	if(in(cur.x-2,cur.y-1))
	{
		if((!board[cur.x-2][cur.y-1] || cl^pieces[board[cur.x-2][cur.y-1]].cl) && _trymv(cur,{cur.x-2,cur.y-1},cl))
			list.push_back({cur.x-2,cur.y-1});
		ctrlpos[cur.x-2][cur.y-1] |= (1+cl);
	}
}

void rook_list(POS cur,int cl,vector <POS> &list)
{
	for(int i=1;1;i++)
	{
		if(!in(cur.x+i,cur.y))
			break;
		ctrlpos[cur.x+i][cur.y] |= (1+cl);
		if(board[cur.x+i][cur.y])
		{
			if(cl^pieces[board[cur.x+i][cur.y]].cl && _trymv(cur,{cur.x+i,cur.y},cl))
				list.push_back({cur.x+i,cur.y});
			break;
		}
		if(_trymv(cur,{cur.x+i,cur.y},cl))
			list.push_back({cur.x+i,cur.y});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x-i,cur.y))
			break;
		ctrlpos[cur.x-i][cur.y] |= (1+cl);
		if(board[cur.x-i][cur.y])
		{
			if(cl^pieces[board[cur.x-i][cur.y]].cl && _trymv(cur,{cur.x-i,cur.y},cl))
				list.push_back({cur.x-i,cur.y});
			break;
		}
		if(_trymv(cur,{cur.x-i,cur.y},cl))
			list.push_back({cur.x-i,cur.y});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x,cur.y+i))
			break;
		ctrlpos[cur.x][cur.y+i] |= (1+cl);
		if(board[cur.x][cur.y+i])
		{
			if(cl^pieces[board[cur.x][cur.y+i]].cl && _trymv(cur,{cur.x,cur.y+i},cl))
				list.push_back({cur.x,cur.y+i});
			break;
		}
		if(_trymv(cur,{cur.x,cur.y+i},cl))
			list.push_back({cur.x,cur.y+i});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x,cur.y-i))
			break;
		ctrlpos[cur.x][cur.y-i] |= (1+cl);
		if(board[cur.x][cur.y-i])
		{
			if(cl^pieces[board[cur.x][cur.y-i]].cl && _trymv(cur,{cur.x,cur.y-i},cl))
				list.push_back({cur.x,cur.y-i});
			break;
		}
		if(_trymv(cur,{cur.x,cur.y-i},cl))
			list.push_back({cur.x,cur.y-i});
	}



}

void queen_list(POS cur,int cl,vector <POS> &list)
{
	bishop_list(cur,cl,list);
	rook_list(cur,cl,list);


/*	for(int i=1;1;i++)
	{
		if(!in(cur.x+i,cur.y))
			break;
		ctrlpos[cur.x+i][cur.y] |= (1+cl);
		if(board[cur.x+i][cur.y])
		{
			if(cl^pieces[board[cur.x+i][cur.y]].cl)
				list.push_back({cur.x+i,cur.y});
			break;
		}
		list.push_back({cur.x+i,cur.y});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x-i,cur.y))
			break;
		ctrlpos[cur.x-i][cur.y] |= (1+cl);
		if(board[cur.x-i][cur.y])
		{
			if(cl^pieces[board[cur.x-i][cur.y]].cl)
				list.push_back({cur.x-i,cur.y});
			break;
		}
		list.push_back({cur.x-i,cur.y});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x,cur.y+i))
			break;
		ctrlpos[cur.x][cur.y+i] |= (1+cl);
		if(board[cur.x][cur.y+i])
		{
			if(cl^pieces[board[cur.x][cur.y+i]].cl)
				list.push_back({cur.x,cur.y+i});
			break;
		}
		list.push_back({cur.x,cur.y+i});
	}
	for(int i=1;1;i++)
	{
		if(!in(cur.x,cur.y-i))
			break;
		ctrlpos[cur.x][cur.y-i] |= (1+cl);
		if(board[cur.x][cur.y-i])
		{
			if(cl^pieces[board[cur.x][cur.y-i]].cl)
				list.push_back({cur.x,cur.y-i});
			break;
		}
		list.push_back({cur.x,cur.y-i});
	}*/


}

void king_list(POS cur,int cl,vector <POS> &list)
{
	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
			if(in(cur.x+i,cur.y+j))
			{
				int tn=board[cur.x+i][cur.y+j];
				ctrlpos[cur.x+i][cur.y+j] |= (1+cl);
				if((!tn || cl^pieces[tn].cl) &&\
					   	/*!( ctrlpos[cur.x+i][cur.y+j] & (2-cl) )) */_trymv(cur,{cur.x+i,cur.y+j},cl))
					list.push_back({cur.x+i,cur.y+j});
			}
	}
}

int castling_list()	//0:X 1:O-O-O 2:O-O 3:both
{
	int ret=0;
	int r1n,r2n,kn,ln;
	int cl=steps%2;
	if(cl==_WHITE)
		ln=0;
	else
		ln=7;
	r1n=board[ln][0],r2n=board[ln][7],kn=board[ln][4];
	if((r1n && kn) && (pieces[r1n].cl==cl && pieces[kn].cl==cl) && (pieces[r1n].moved==false && pieces[kn].moved==false))
	{
		bool flag=true;
		int opctr=2-cl;
		for(int i=1;i<=3;i++)
			if((i!=1 && ctrlpos[ln][i]&opctr) || board[ln][i]!=0)
			{
				flag=false;
				break;
			}
		if(ctrlpos[ln][4]&opctr)
			flag=false;
		if(flag)
			ret|=1;
	}
	if((r2n && kn) && (pieces[r2n].cl==cl && pieces[kn].cl==cl) && (pieces[r2n].moved==false && pieces[kn].moved==false))
	{
		bool flag=true;
		int opctr=2-cl;
		for(int i=5;i<=6;i++)
			if(ctrlpos[ln][i]&opctr || board[ln][i]!=0)
			{
				flag=false;
				break;
			}
		if(ctrlpos[ln][4]&opctr)
			flag=false;
		if(flag)
			ret|=2;
	}
	return ret;

}

void ctr_rfs()
{
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			vector <POS> tmp;
			int cn=board[i][j];
			if(cn)
			{
				int type=pieces[cn].type;
				int cl=pieces[cn].cl;
				switch(type)
				{
					case _TP:
						pawn_list({i,j},cl,tmp);
						break;
					case _TB:
						bishop_list({i,j},cl,tmp);
						break;
					case _TN:
						knight_list({i,j},cl,tmp);
						break;
					case _TR:
						rook_list({i,j},cl,tmp);
						break;
					case _TQ:
						queen_list({i,j},cl,tmp);
						break;
					case _TK:
						king_list({i,j},cl,tmp);
						break;
				}
			}
		}
	}
}

bool gameover()
{
	int cl=steps%2;
	int mvab=false;
	vector <int> pcl1,pcl2;
	vector <POS> ppl1,ppl2;
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
		{
			if(board[i][j])
				if(pieces[board[i][j]].cl==_WHITE)
				{
					pcl1.push_back(pieces[board[i][j]].type);
					ppl1.push_back({i,j});
				}
				else
				{
					pcl2.push_back(pieces[board[i][j]].type);
					ppl2.push_back({i,j});
				}

			if(board[i][j] && pieces[board[i][j]].cl==cl)
			{
				vector <POS> tmp;
                switch(pieces[board[i][j]].type)
                {
                    case _TP:
                        pawn_list({i,j},cl,tmp);
                        break;
                    case _TB:
                        bishop_list({i,j},cl,tmp);
                        break;
                    case _TN:
                        knight_list({i,j},cl,tmp);
                        break;
                    case _TR:
                        rook_list({i,j},cl,tmp);
                        break;
                    case _TQ:
                        queen_list({i,j},cl,tmp);
                        break;
                    case _TK:
                        king_list({i,j},cl,tmp);
                        break;
                }
				if(tmp.size())
					mvab=true;
            }
		}
	if(pcl1.size()<=2 && pcl2.size()<=2)
	{
		int pc1=0;
		POS pp1={-1,-1};
		int pc2=0;
		POS pp2={-1,-1};
		int pd1;int pd2;
		if(pcl1.size()==2)
		{
			if(pcl1[0]==_TK)
				pc1=pcl1[1],pp1=ppl1[1];
			else
				pc1=pcl1[0],pp1=ppl1[0];
			pd1=pp1.x-pp1.y;
			if(pd1<0)
				pd1=(-pd1);
		}
		if(pcl2.size()==2)
		{
			if(pcl2[0]==_TK)
				pc2=pcl2[1],pp2=ppl2[1];
			else
				pc2=pcl2[0],pp2=ppl2[0];
			pd2=pp2.x-pp2.y;
			if(pd2<0)
				pd2=(-pd2);

		}
		
		if((pc1==0 && pc2==0) || \
		   (pc1==0 && (pc2==_TB || pc2==_TN)) || \
		   (pc2==0 && (pc1==_TB || pc1==_TN)) || \
		   (pc1==_TB && pc2==_TB && (pd1%2)==(pd1%2) ) )
		{
			note(_Notice_LMD);
			return true;
		}
	}
	if(mvab)
		return false;
	else if(_check(cl))
	{
		note(_Notice_CKM);
		return true;
	}
	else
	{
		note(_Notice_STM);
        return true;
	}

}

/*
bool _check(int cl)
{
	POS kp;
	ctr_rfs();
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			if(pieces[board[i][j]].type==_TK && pieces[board[i][j]].cl==cl)
				return (ctrlpos[i][j]&(2-cl));
	note(_Notice_UKE);
	exit(1);
	
}
*/

/*
   int game()
   {
   while(1)
   {
   draw();
   input();

 ***		if(illegal(curmv))
 {
 note(_Fault);
 continue;
 }***
 if(checkmate())
 {
 note(_Notice_CKM);
 break;
 }
 else if(stalemate())
 {
 note(_Notice_STM);
 break;
 }
 else if(check())
 note(_Notice_CHK);
 steps++;
 }
 note(_Notice_CFA);
 if(getch()=='y')
 return 1;
 return 0;
 }
 */

int input()
{
	POS fp,tp;
	string ins;
	PIECE curpc;
	int cl=steps%2;
	vector <POS> list;
	cin>>ins;
	if(ins=="O-O-O")
	{
		if(1&castling_list())
		{
			int r1n,kn,ln;
			ln=7*(steps%2);
			r1n=board[ln][0],kn=board[ln][5];
			board[ln][0]=0;board[ln][4]=0;
			board[ln][3]=r1n;board[ln][2]=kn;
			pieces[kn].moved=1;
			return 0;
		}
		else
		{
			note(_Fault);
			return 1;
		}
	}
	if(ins=="O-O")
	{
		if(2&castling_list())
        {
			int r2n,kn,ln;
			ln=7*(steps%2);
			r2n=board[ln][7],kn=board[ln][4];
			board[ln][7]=0;board[ln][4]=0;
            board[ln][5]=r2n;board[ln][6]=kn;
			pieces[kn].moved=1;
			return 0;
		}
		else
		{
			note(_Fault);
			return 1;
		}
	}
	if(ins.size()!=5)
		return 1;
	fp.y=ins[0]-'a';
	fp.x=ins[1]-'1';
	tp.y=ins[3]-'a';
	tp.x=ins[4]-'1';
//////////////	
	curpc=pieces[board[fp.x][fp.y]];
	if(cl^curpc.cl)
	{
		note(_Fault);
		return 1;
	}
	switch(curpc.type)
	{
		case _TP:
			{
				bool Default=false;
				int direct;
				if(cl==0)
					direct=1;
				else
					direct=-1;
				pawn_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
					if(list[i]==tp)
					{
						Default=true;
						break;
					}
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					int tn=board[tp.x][tp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
					if(!tn && fp.y!=tp.y)	//en passant
						board[fp.x][tp.y]=0;
					if((tp.x-fp.x)==2*direct)	//dash
						splog.push_back({_DASH,steps,tp,0,0});
					pieces[cn].moved=1;
					if(tp.x==7*(1-cl))
						_promote(tp);
				}
				else
				{
					note(_Fault);
					return 1;
				}
			}
			break;
		case _TB:
			{
				bool Default=false;
				bishop_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
                    if(list[i]==tp)
                    {
                        Default=true;
                        break;
                    }
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
                    pieces[cn].moved=1;
				}
				else
				{
                    note(_Fault);
                    return 1;
                }
			}
			break;
		case _TN:
			{
				bool Default=false;
				knight_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
                    if(list[i]==tp)
                    {
                        Default=true;
                        break;
                    }
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
                    pieces[cn].moved=1;
				}
				else
				{
                    note(_Fault);
                    return 1;
                }

			}
			break;
		case _TR:
			{
				bool Default=false;
				rook_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
                    if(list[i]==tp)
                    {
                        Default=true;
                        break;
                    }
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
                    pieces[cn].moved=1;
				}
				else
				{
                    note(_Fault);
                    return 1;
                }

			}
			break;
		case _TQ:
			{
				bool Default=false;
				queen_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
                    if(list[i]==tp)
                    {
                        Default=true;
                        break;
                    }
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
                    pieces[cn].moved=1;
				}
				else
				{
                    note(_Fault);
                    return 1;
                }

			}
			break;
		case _TK:
			{
				bool Default=false;
				king_list(fp,cl,list);
				for(int i=0,sz=list.size();i<sz;i++)
                    if(list[i]==tp)
                    {
                        Default=true;
                        break;
                    }
				if(Default)
				{
					int cn=board[fp.x][fp.y];
					board[tp.x][tp.y]=cn;
					board[fp.x][fp.y]=0;
                    pieces[cn].moved=1;
				}
				else
				{
                    note(_Fault);
                    return 1;
                }

			}
			break;
	}
	ctr_rfs();
	return 0;
}

void testinput()
{
	int n;
	steps=27;
	//上一步由白方走出
	cout<<"input n:";
	cin>>n;
	for(int i=1;i<=n;i++)
	{
		int x,y,cl,type;
		cout<<"input x,y,color,type"<<endl;
		cin>>x>>y>>cl>>type;
		pieces[i].type=type;
		pieces[i].cl=cl;
		board[x][y]=i;
	}
	int dashx,dashy;
	cout<<"set dash:";
	cin>>dashx>>dashy;
	splog.push_back({_DASH,steps-1,{dashx,dashy},0,0});

}


void from_fen()
{
	string main_board;
	char ccl;
	string cslab;
	string eppos;//En passant
	int lcstep;//Halfmove clock
	int round=1;
	int cslkey1=0,cslkey2=0;
	cin>>main_board>>ccl>>cslab>>eppos>>lcstep>>round;
	for(int i=0,x=7,y=0,cnt=1,sz=main_board.size();i<sz;i++)
	{
		int ch=main_board[i];
		if(ch>='1' && ch<='8')
		{
			int eptn=(int)(ch-'0');
			while(eptn--)
				board[x][y++]=0;
			continue;
		}
		if(ch=='/')
		{
			x--;
			y=0;
			continue;
		}
		switch(ch)
		{
			case 'P':
				pieces[cnt].cl=_WHITE;
				pieces[cnt].type=_TP;
				board[x][y]=cnt;
				if(x!=1)
					pieces[cnt].moved=1;
				break;
			case 'p':
				pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TP;
                board[x][y]=cnt;
                if(x!=6)
                    pieces[cnt].moved=1;
                break;
			case 'B':
				pieces[cnt].cl=_WHITE;
                pieces[cnt].type=_TB;
                board[x][y]=cnt;
                break;
			case 'b':
                pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TB;
                board[x][y]=cnt;
                break;
			case 'N':
                pieces[cnt].cl=_WHITE;
                pieces[cnt].type=_TN;
                board[x][y]=cnt;
                break;
            case 'n':
                pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TN;
                board[x][y]=cnt;
                break;
			case 'R':
                pieces[cnt].cl=_WHITE;
                pieces[cnt].type=_TR;
                board[x][y]=cnt;
				if(x!=0 || y!=0 || y!=7)
				   pieces[cnt].moved=1;	
                break;
            case 'r':
                pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TR;
                board[x][y]=cnt;
				if(x!=7 || y!=0 || y!=7)
				   pieces[cnt].moved=1;	
                break;
			case 'Q':
                pieces[cnt].cl=_WHITE;
                pieces[cnt].type=_TQ;
                board[x][y]=cnt;
                break;
            case 'q':
                pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TQ;
                board[x][y]=cnt;
                break;
			case 'K':
                pieces[cnt].cl=_WHITE;
                pieces[cnt].type=_TK;
                board[x][y]=cnt;
				if(x!=0 || y!=4)
					pieces[cnt].moved=1;
                break;
            case 'k':
                pieces[cnt].cl=_BLACK;
                pieces[cnt].type=_TK;
                board[x][y]=cnt;
				if(x!=7 || y!=4)
                    pieces[cnt].moved=1;
                break;
		}
		cnt++;
		y++;
	}
	if(ccl=='w')
		steps=(round-1)*2+_WHITE;
	else
		steps=(round-1)*2+_BLACK;
	if(cslab.size()==1 && cslab[0]=='-')
	{
		if(board[0][4] && pieces[board[0][4]].cl==_WHITE && pieces[board[0][4]].type==_TK)
			pieces[board[0][4]].moved=1;
		if(board[7][4] && pieces[board[7][4]].cl==_BLACK && pieces[board[7][4]].type==_TK)
			pieces[board[7][4]].moved=1;
	}

	for(int i=0,sz=cslab.size();i<sz;i++)
	{
		char ch=cslab[i];
		switch(ch)
		{
			case 'Q':
				cslkey1|=1;
			case 'K':
				cslkey1|=2;
			case 'q':
                cslkey2|=1;
            case 'k':
                cslkey2|=2;

		}
	}
	switch(cslkey1)
	{
		case 3:
			break;
		case 1:
			if(board[0][7] && pieces[board[0][7]].cl==_WHITE && pieces[board[0][7]].type==_TR)
				pieces[board[0][7]].moved=1;
			break;
		case 2:
			if(board[0][0] && pieces[board[0][0]].cl==_WHITE && pieces[board[0][0]].type==_TR)
                pieces[board[0][7]].moved=1;
            break;
		default:
			if(board[0][4] && pieces[board[0][4]].cl==_WHITE && pieces[board[0][4]].type==_TK)
				pieces[board[0][4]].moved=1;
			break;
	}
	switch(cslkey2)
    {
        case 3:
            break;
        case 1:
            if(board[7][7] && pieces[board[7][7]].cl==_BLACK && pieces[board[7][7]].type==_TR)
                pieces[board[7][7]].moved=1;
            break;
        case 2:
            if(board[7][0] && pieces[board[7][0]].cl==_BLACK && pieces[board[7][0]].type==_TR)
                pieces[board[7][0]].moved=1;
            break;
        default:
            if(board[7][4] && pieces[board[7][4]].cl==_BLACK && pieces[board[7][4]].type==_TK)
                pieces[board[7][4]].moved=1;
            break;
    }
	if(eppos!="-")
	{
		int dashx,dashy;
		dashx=((char)(eppos[1])-'1');
		dashy=((char)(eppos[0])-'a');
		if(dashx==5)
			dashx=4;
		if(dashx==2)
			dashx=3;
		if(in(dashx,dashy) && board[dashx][dashy] && (steps%2)^pieces[board[dashx][dashy]].cl\
			   	&& pieces[board[dashx][dashy]].type==_TP && dashx-pieces[board[dashx][dashy]].cl==3)
			splog.push_back({_DASH,steps-1,{dashx,dashy},0,0});
	}


}

void testoutput()
{
	ctr_rfs();
	
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<8;j++)
		{
			vector <POS> list;
			int cn=board[i][j];
			if(cn)
			{
				int type=pieces[cn].type;
				int cl=pieces[cn].cl;
				switch(type)
				{
					case _TP:
						pawn_list({i,j},cl,list);
						break;
					case _TB:
						bishop_list({i,j},cl,list);
						break;
					case _TN:
						knight_list({i,j},cl,list);
						break;
					case _TR:
						rook_list({i,j},cl,list);
						break;
					case _TQ:
						queen_list({i,j},cl,list);
						break;
					case _TK:
						king_list({i,j},cl,list);
						break;
				}
				cout<<i<<' '<<j<<" type:"<<pieces[cn].type<<endl;;
				for(int i=0,sz=list.size();i<sz;i++)
					cout<<list[i].x<<" - "<<list[i].y<<endl;
				cout<<endl;
			}
		}

	}
	/*
	cout<<"test control-positions:"<<endl;
	for(int i=7;i>=0;i--)
	{
		for(int j=0;j<8;j++)
			cout<<ctrlpos[i][j]<<' ';
		cout<<endl;
	}

	cout<<"test display:"<<endl;
	*/
	for(int i=7;i>=0;i--)
	{
		cout<<i+1<<' '<<' ';
		for(int j=0;j<8;j++)
		{
			int cn=board[i][j];
			int cl=pieces[cn].cl;
			int type=pieces[cn].type;
			int cos=cl?32:0;//character offset
			switch(type)
			{
				case _TP:
					cout<<(char)('P'+cos)<<' ';
					break;
				case _TB:
					cout<<(char)('B'+cos)<<' ';
					break;
				case _TN:
					cout<<(char)('N'+cos)<<' ';
					break;
				case _TR:
					cout<<(char)('R'+cos)<<' ';
					break;
				case _TQ:
					cout<<(char)('Q'+cos)<<' ';
					break;
				case _TK:
					cout<<(char)('K'+cos)<<' ';
					break;
				default:
					cout<<'+'<<' ';
			}
		}
		cout<<endl;
	}
/*	cout<<' '<<'|';
	for(int i=0;i<8;i++)
		cout<<'_'<<' ';*/
	cout<<endl;
	cout<<' '<<' '<<' ';
	for(int i=0;i<8;i++)
		cout<<(char)('a'+i)<<' ';
	cout<<endl;

	if(steps%2==_WHITE)
		cout<<"白方走棋"<<endl;
	else
		cout<<"黑方走棋"<<endl;

}

void displayboard()
{
	for(int i=7;i>=0;i--)
	{
		cout<<i+1<<' '<<' ';
		for(int j=0;j<8;j++)
		{
			int cn=board[i][j];
			int cl=pieces[cn].cl;
			int type=pieces[cn].type;
			int cos=cl?32:0;//character offset
			switch(type)
			{
				case _TP:
					cout<<(char)('P'+cos)<<' ';
					break;
				case _TB:
					cout<<(char)('B'+cos)<<' ';
					break;
				case _TN:
					cout<<(char)('N'+cos)<<' ';
					break;
				case _TR:
					cout<<(char)('R'+cos)<<' ';
					break;
				case _TQ:
					cout<<(char)('Q'+cos)<<' ';
					break;
				case _TK:
					cout<<(char)('K'+cos)<<' ';
					break;
				default:
					cout<<'+'<<' ';
			}
		}
		cout<<endl;
	}
/*	cout<<' '<<'|';
	for(int i=0;i<8;i++)
		cout<<'_'<<' ';*/
	cout<<endl;
	cout<<' '<<' '<<' ';
	for(int i=0;i<8;i++)
		cout<<(char)('a'+i)<<' ';
	cout<<endl;

}

void game()
{
	while(1)
	{
		displayboard();
		if( gameover() )
			break;
		if(_check(steps%2))
			note(_Notice_CHK);
		do{
			cout<<"输入两个坐标或易位标识,完成走子  ";
			if(steps%2==_WHITE)
				cout<<"当前是白方走子"<<endl;
			if(steps%2==_BLACK)
				cout<<"当前是黑方走子"<<endl;
		}
		while(input());
		
		system("clear");
		steps++;
	}
}

int main()
{

//	testinput();
	
//	testoutput();
	
//	from_position();
//	displayboard();

//	while(1){
//	if(input()==0)
//		break;
//
//	}
//	from_fen();
//	testoutput();
	
//	displayboard();
//	if(!gameover())
//		cout<<"未结束"<<endl;
	int gamemode=0;
	system("clear");
	cout<<"输入游戏模式:  1-从空白局面开始 2-从fen导入局面"<<endl;
	cin>>gamemode;
	if(gamemode==1)
		init();
	else
		from_fen();
	game();
	
	return 0;
}

