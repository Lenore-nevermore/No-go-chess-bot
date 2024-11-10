#include <graphics.h>
#include <conio.h>//图形界面通过easyx实现：https://easyx.cn/
#include <iostream>
#include<cmath>
#include<fstream>
#include<Windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#define HEIGHT 560
#define WIDTH 850
using namespace std;

int chess[11][11] = {};//白 1 ; 黑-1 ; 空 0
int memo[64][3] = {};//记录下子顺序
int steps = 0; //记录下了几步
int tx, ty;
int dx[] = { -1,0,1,0,-1,1,1,-1 }; int dy[] = { 0,-1,0,1,-1,-1,1,1 };
int color = -1;//记录所要下的棋子的颜色
int simcolor = -1;
bool air[11][11] = {};//用以实现air_exist递归算法（遍历四周）
int simulation[11][11] = {};
double ucb[11][11] = {};//记录ucb值
int nj[11][11] = {};//各个子节点被遍历的次数
int win = 0;
int Air = 0;
int my_point = 0, enemy_point = 0;//多少个自己/对手能走的点
int moveX = 0, moveY = 0;
int link_temp = 0;
bool temptemp[10][10] = {};
bool my_map[10][10] = {}, enemy_map[10][10] = {};
int playercolor = 0, botcolor = 0;//人机对弈各自颜色
int flagmain = 0;
bool mode2 = 0;//记录是不是双人模式

void Button(int x1, int x2, int y1, int y2, TCHAR* text)//范佳玥
{
	setbkmode(TRANSPARENT);
	TCHAR s1[] = L"Consolas";
	settextstyle(30, 0, s1);
	int tx = x1 + (x2 - x1 - textwidth(text)) / 2;
	int ty = y1 + (y2 - y1 - textheight(text)) / 2;
	outtextxy(tx, ty, text);
}//用于画出按钮
void rev_last(void) {//范佳玥
	if (steps != 1) {
		int lx = 50 * memo[steps - 1][0]; int ly = 50 * memo[steps - 1][1];
		setcolor(BLACK);
		circle(lx, ly, 15); circle(lx, ly, 16);
	}
}//提示上一步下在了哪里
int regret(int x, int y) {//范佳玥
	int flagr = 0;
	ExMessage mr;
	while (true) {
		mr = getmessage(EX_MOUSE);
		if (mr.message == WM_LBUTTONDOWN) {
			if (mr.x >= 650 && mr.x <= 800 && mr.y >= 495 && mr.y <= 530) {
				return 0;
			}
			else if (mr.x >= 370 && mr.x <= 580 && mr.y >= 495 && mr.y <= 530) {
				chess[x][y] = 0; steps--; color = color * (-1);
				x *= 50; y *= 50;
				setfillcolor(RGB(242, 199, 149));
				fillrectangle(x - 16, y - 16, x + 16, y + 16);
				setcolor(RGB(242, 199, 149));
				line(x - 16, y - 16, x + 16, y - 16); line(x - 16, y - 16, x - 16, y + 16);
				line(x - 16, y + 16, x + 16, y + 16); line(x + 16, y - 16, x + 16, y + 16);
				setcolor(BLACK);
				int sx = x - 20, ex = x + 20; int sy = y - 20, ey = y + 20;
				if (x - 20 < 50) sx = 50;
				else if (x + 20 > 450) ex = 450;
				if (y - 20 < 50) sy = 50;
				else if (y + 20 > 450) ey = 450;
				line(sx, y, ex, y); line(x, sy, x, ey);
				//setcolor(RED);
				//circle(memo[steps][0] * 50, memo[steps][1] * 50, 15); circle(memo[steps][0] * 50, memo[steps][1] * 50, 16);
				ExMessage mrr;
				while (true) {
					mrr = getmessage(EX_MOUSE);
					if (mrr.message == WM_LBUTTONDOWN) {
						for (int t = 35; t <= 435; t += 50) {
							for (int s = 35; s <= 435; s += 50) {
								if (mrr.x >= t && mrr.x <= t + 30 && mrr.y >= s && mrr.y <= s + 30) {
									tx = (t + 15) / 50, ty = (s + 15) / 50;
									flagr = 1;
								}
							}
							if (flagr) break;
						}
					}
					if (flagr) break;
				}
				return 1;
				break;
			}
		}
	}
}//悔棋
void black(int x, int y) {//范佳玥
	chess[x][y] = -1;
	color = color * (-1);
	steps++; memo[steps][0] = x; memo[steps][1] = y;
	setcolor(BLACK);
	x = x * 50;
	y = y * 50;
	setfillcolor(BLACK);
	fillcircle(x, y, 15);
	setcolor(RED);
	circle(x, y, 15); circle(x, y, 16);
	rev_last();
}//下黑子，并且改变相应位置chess的值
void white(int x, int y) {//范佳玥
	chess[x][y] = 1;
	color = color * (-1);
	steps++; memo[steps][0] = x; memo[steps][1] = y;
	setcolor(BLACK);
	x = x * 50;
	y = y * 50;
	setfillcolor(WHITE);
	fillcircle(x, y, 15);
	setcolor(RED);
	circle(x, y, 15); circle(x, y, 16);
	rev_last();
}//下白子，并且改变相应位置chess的值
void draw_chess() {//范佳玥
	initgraph(WIDTH, HEIGHT);
	memset(chess, 0, sizeof(chess)); memset(memo, 0, sizeof(memo));  memset(air, 0, sizeof(air));
	my_point = 0, enemy_point = 0; color = -1;
	moveX = 0, moveY = 0;
	link_temp = 0;
	for (int y = 0; y < 600; y++)//绘制渐变背景
	{
		int c = y * 235 / 600;
		setcolor(RGB(234, 121, c));
		line(0, y, 850, y);
	}
	setcolor(BLACK);//绘制棋盘
	setfillcolor(RGB(242, 199, 149));
	fillrectangle(20, 20, 480, 480);
	rectangle(20, 20, 480, 480);
	for (int i = 50; i <= 450; i += 50)
	{
		line(50, i, 450, i);

	}
	for (int i = 50; i <= 450; i += 50)
	{
		line(i, 50, i, 450);
	}
	TCHAR s3[50] = L"Start";//绘制主菜单
	TCHAR s1[50] = L"End";
	TCHAR s2[50] = L"Save";
	TCHAR s4[50] = L"Read";
	TCHAR s9[50] = L"Change to: Two Players";
	Button(630, 730, 120, 170, s3);
	Button(630, 730, 190, 240, s4);
	Button(630, 730, 265, 315, s2);
	Button(630, 730, 345, 395, s1);
	Button(620, 720, 420, 470, s9);
	TCHAR s7[] = L"Berlin Sans FB";
	settextstyle(30, 0, s7);
	TCHAR s5[50] = L"Welcome to";
	TCHAR s6[50] = L"the world of NOGO!";
	outtextxy(610, 20, s5); outtextxy(565, 60, s6);
	settextstyle(30, 0, s7);
	TCHAR t1[50] = L"Confirm";
	TCHAR t2[50] = L"Second Chance";
	setfillcolor(RGB(238, 213, 210));
	fillrectangle(370, 495, 580, 530); fillrectangle(650, 495, 800, 530);
	outtextxy(675, 498, t1); outtextxy(390, 498, t2);

}//画出棋盘以及菜单
void start(void) {//范佳玥
	draw_chess();
	setbkmode(TRANSPARENT);
	TCHAR s1[] = L"Berlin Sans FB";
	settextstyle(25, 0, s1);
	TCHAR s2[50] = L"Choose color: ";
	outtextxy(50, 500, s2);
	setfillcolor(BLACK);
	fillcircle(200, 510, 10);
	setfillcolor(WHITE);
	fillcircle(230, 510, 10);
	ExMessage ms;
	while (true) {
		ms = getmessage(EX_MOUSE);
		if (ms.message == WM_LBUTTONDOWN) {
			if (ms.x >= 185 && ms.x <= 215 && ms.y >= 495 && ms.y <= 515) {
				setcolor(RED);
				circle(200, 510, 11);
				circle(200, 510, 12);
				playercolor = -1; botcolor = 1;
				break;
			}
			else if (ms.x >= 215 && ms.x <= 245 && ms.y >= 495 && ms.y <= 515) {
				setcolor(RED);
				circle(230, 510, 11);
				circle(230, 510, 12);
				playercolor = 1; botcolor = -1;
				break;
			}
		}
	}
}//按下开始键，选手选择棋子颜色
void play(void);
void read(void) {//范佳玥
	draw_chess();
	ifstream in("nogo.txt", ios::in);
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			in >> chess[i][j];
		}
	}
	in >> steps;
	for (int i = 1; i <= steps; i++) {
		in>>memo[i][0]; in>>memo[i][1];
	}
	in >> color; in >> playercolor; in >> botcolor;
	in >> mode2;
	in.close();
	simcolor = color; int simsteps = steps; steps = 0;
	for (int i = 1; i <=simsteps; i++) {
		int xx = memo[i][0]; int yy = memo[i][1];
		if (i % 2 == 1)black(xx, yy);
		else white(xx, yy);		
	}
	color = simcolor; steps = simsteps;
}//读盘
void save(void) {//范佳玥
	ofstream of("nogo.txt", ios::out);
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			of << " " << chess[i][j];
		}
	}
	of << " " << steps;
	for (int i = 1; i <= steps; i++) {
		of << " " << memo[i][0]; of << " " << memo[i][1];
	}
	of << " " << color; of << " " << playercolor; of << " " << botcolor;
	of << " " << mode2;
	of.close();
}//存盘
void bot_chess(int x, int y) {//范佳玥
	if (botcolor == -1) {
		black(x, y);
	}
	else if (botcolor == 1) {
		white(x, y);
	}
}//bot下棋
bool border(int x, int y) { return x > 0 && y > 0 && x <= 9 && y <= 9; }
int air_exist(int x, int y, int copyX, int copyY)// return 还有多少气 (注意：非传统意义上的气，此处指对手棋界内剩余多少空点(包括自身))   杨子涵
{
	if (x == copyX && y == copyY && air[x][y] == false) Air = 0;
	if (air[x][y] || !border(x, y)) return 0;//判断未曾遍历 + 引用border函数:判断非越界
	if (chess[x][y] == 0) { Air++; }
	air[x][y] = true;
	for (int i = 0; i < 4; i++)
		if (chess[x + dx[i]][y + dy[i]] + chess[copyX][copyY] || (chess[x + dx[i]][y + dy[i]] == 0 && chess[x][y] == 0))// 即如果与原来chess[i][j]并非一黑一白 or 连着的两空
			air_exist(x + dx[i], y + dy[i], copyX, copyY);
	if (x == copyX && y == copyY) for (int i = 1; i <= 9; i++) for (int j = 1; j <= 9; j++) air[i][j] = 0;
	return Air;
}
bool legal(int tx, int ty) {//范佳玥
	if (!border(tx, ty)) {
		return false;
	}
	else if (steps == 1 && tx == 5 && ty == 5) {
		return false;
	}
	else {
		return true;
	}
}//下子合法，即不在边界内且第一个黑子不在中间格
bool notsuicide(int x, int y) {//范佳玥
	int tem = chess[x][y];
	chess[x][y] = color;

	if (!air_exist(x, y, x, y)) //不可以下完此点没气了
	{
		chess[x][y] = 0;
		return false;
	}

	for (int i = 0; i < 4; ++i) //不可以让周围的棋子没有气
	{
		int cx = x + dx[i], cy = y + dy[i];
		if (border(cx, cy))
		{
			if (chess[cx][cy])
				if (!air_exist(cx, cy, cx, cy))
				{
					chess[x][y] = tem;
					return false;
				}
		}
	}
	chess[x][y] = tem;
	return true;
}//下此步不会导致自己输-1；会输-0；
bool whether_end()//杨子涵
{
	for (int i = 1; i <= 9; i++)
		for (int j = 1; j <= 9; j++)
			if (!chess[i][j] && notsuicide(i, j))//存在位置为空 and notsuicide
				return false;
	return true;
}
int ends(int x, int y) {//范佳玥
	color = color * (-1);
	if ((!notsuicide(x, y)) || (!legal(x, y))) return 1;
	color = color * (-1);
	return 0;
}//判断下完这一步游戏是否结束
void player_lose(void) {
	setfillcolor(RGB(255, 161, 161));
	fillrectangle(20, 200, 480, 300);
	TCHAR ss1[50] = L"YOU LOSE!";
	TCHAR s1[] = L"Consolas";
	settextstyle(50, 0, s1);
	outtextxy(150, 227, ss1);
	_getch();
	closegraph();
}
void player_win(void) {
	setfillcolor(RGB(255, 161, 161));
	fillrectangle(20, 200, 480, 300);
	TCHAR ss2[50] = L"YOU WIN!";
	TCHAR s1[] = L"Consolas";
	settextstyle(50, 0, s1);
	outtextxy(150, 227, ss2);
	_getch();
	closegraph();
}
int link(int x, int y, int copyX, int copyY) {//递归数连着多少我方棋子   杨子涵

	if (x == copyX && y == copyY && !temptemp[x][y]) link_temp = 0;
	link_temp++;
	temptemp[x][y] = 1;
	for (int i = 0; i < 8; i++)
	{
		if (border(x + dx[i], y + dy[i]) && !temptemp[x + dx[i]][y + dy[i]] && chess[x + dx[i]][y + dy[i]] == color)
		{
			link(x + dx[i], y + dy[i], copyX, copyY);
		}
	}
	if (x == copyX && y == copyY) for (int i = 1; i <= 9; i++) for (int j = 1; j <= 9; j++) temptemp[i][j] = 0;
	if (x == copyX && y == copyY && temptemp[x][y]) return link_temp;
	return 0;
}
int count_point(int x, int y, int z) //判断我/对方（z:1/-1）占该点后，增加了多少个自己点+减少了多少对手点   杨子涵
{
	bool flag = 0;//截枝：判断有无眼生成
	int tempM = 0, tempE = 0;//temp my_point,temp enemy_point
	chess[x][y] = color * z;
	for (int i = 0; i < 4; i++)
		if (border(x + dx[i], y + dy[i]) && chess[x + dx[i]][y + dy[i]] != color * z)
			if (air_exist(x + dx[i], y + dy[i], x + dx[i], y + dy[i]) == 1) { flag = true; }

	if (flag)
	{
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
			{
				if (!chess[i][j] && notsuicide(i, j)) tempM++;
				color *= -1;
				if (!chess[i][j] && notsuicide(i, j)) tempE++;
				color *= -1;
			}
		chess[x][y] = 0;
		return tempM - my_point + enemy_point - tempE;
	}
	else
	{
		chess[x][y] = 0;
		return 0;
	}
}
void draw_map() {//数我方/对方能落的位置  杨子涵
	my_point = 0; enemy_point = 0;
	for (int i = 1; i <= 9; i++)
		for (int j = 1; j <= 9; j++)
		{
			if (!chess[i][j] && notsuicide(i, j)) { my_point++; my_map[i][j] = true; }
			else my_map[i][j] = false;
			color *= -1;

			if (!chess[i][j] && notsuicide(i, j)) { enemy_point++; enemy_map[i][j] = true; }
			else enemy_map[i][j] = false;
			color *= -1;
		}
	return;
}
void efficiency()//判断下哪步划算（步法搜索）杨子涵
{
	int temp = 0;
	int my_Max = -1, AmoveX = 0, AmoveY = 0;//attack
	int enemy_Max = 1, DmoveX = 0, DmoveY = 0;//defense
	int temp2_map[10][10] = {}, temp3_map[10][10] = {}, temp4_map[10][10] = {};// 有数(非零)意味gas=2/3/4;数字代表周围连起来的己方数+1
	moveX = 0, moveY = 0, my_point = 0, enemy_point = 0;

	draw_map();
	for (int i = 1; i <= 9; i++)
		for (int j = 1; j <= 9; j++)
		{
			if (my_map[i][j] && my_Max < count_point(i, j, 1))
				my_Max = count_point(i, j, 1), AmoveX = i, AmoveY = j;
			if (my_map[i][j] && enemy_map[i][j] && enemy_Max > count_point(i, j, -1))
				enemy_Max = count_point(i, j, -1), DmoveX = i, DmoveY = j;
		}
	if (enemy_Max || my_Max) {
		if (enemy_Max + my_Max < 0)
			moveX = DmoveX, moveY = DmoveY;
		else
			moveX = AmoveX, moveY = AmoveY;
	}
	else
	{

		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
			{
				if (!chess[i][j])
					for (int t = 0; t < 4; t++)
						if (border(i + dx[t], j + dy[t]) && !chess[i + dx[t]][j + dy[t]])
							temp++;
				if (temp == 2) temp2_map[i][j] = link(i, j, i, j) + 1;
				else if (temp == 3) temp3_map[i][j] = link(i, j, i, j) + 1;
				else if (temp == 4) temp4_map[i][j] = link(i, j, i, j) + 1;
				temp = 0;
			}
		int cmp = 0, TempX = 0, TempY = 0;
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
				if (temp2_map[i][j] and temp2_map[i][j] > cmp)
					cmp = temp2_map[i][j], TempX = i, TempY = j;
		if (TempX && TempY)
			for (int s = 0; s < 4; s++)
				if (border(TempX + dx[s], TempY + dy[s]) && my_map[TempX + dx[s]][TempY + dy[s]])
				{
					moveX = TempX + dx[s], moveY = TempY + dy[s]; return;
				}
		cmp = 0, TempX = 0, TempY = 0;
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
				if (temp3_map[i][j] and temp3_map[i][j] > cmp)
					cmp = temp3_map[i][j], TempX = i, TempY = j;
		if (TempX && TempY)
			for (int s = 0; s < 4; s++)
				if (border(TempX + dx[s], TempY + dy[s]) && my_map[TempX + dx[s]][TempY + dy[s]])
				{
					moveX = TempX + dx[s], moveY = TempY + dy[s]; return;
				}
		cmp = 0, TempX = 0, TempY = 0;
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
				if (temp4_map[i][j] and temp4_map[i][j] > cmp)
					cmp = temp4_map[i][j], TempX = i, TempY = j;
		if (TempX && TempY)
			for (int s = 0; s < 4; s++)
				if (border(TempX + dx[s], TempY + dy[s]) && my_map[TempX + dx[s]][TempY + dy[s]])
				{
					moveX = TempX + dx[s], moveY = TempY + dy[s]; return;
				}
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
				if (my_map[i][j] && enemy_map[i][j])
				{
					moveX = i, moveY = j;
					return;
				}
		for (int i = 1; i <= 9; i++)
			for (int j = 1; j <= 9; j++)
				if (my_map[i][j])
				{
					moveX = i, moveY = j;
					return;
				}
	}
	return;
}
void copytosim(void) {//范佳玥
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			simulation[i][j] = chess[i][j];
		}
	}
}//将现实棋盘复制到模拟棋盘上,以便回溯
void copytochess(void) {//范佳玥
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			chess[i][j] = simulation[i][j];
		}
	}
}
void calculate_nj(int x, int y) {//范佳玥
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			if (!chess[i][j] && notsuicide(i, j)) {
				nj[x][y]++;
			}
		}
	}
}//计算可以下的格子数目
void playout(void) {//范佳玥
	if (!whether_end()) {
		for (int i = 1; i <= 9; i++) {
			for (int j = 1; j <= 9; j++) {
				if (!chess[i][j] && notsuicide(i, j)) {
					chess[i][j] = color;
					int temp = color;
					color = color * (-1);
					if (whether_end()) {
						if (color == 1) win++;
					}
					else {
						playout();
					}
					color = temp;
					chess[i][j] = 0;
				}
			}
		}
	}
}//继续模拟比赛进行
void UCB(void) {//范佳玥
	memset(ucb, 0, sizeof(ucb));
	memset(nj, 0, sizeof(nj));
	int totalN = 0;
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			if (!chess[i][j] && notsuicide(i, j)) {
				totalN++;
			}
		}
	}
	;
	copytosim(); simcolor = color;//sim保存真实棋盘
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			if (!chess[i][j] && notsuicide(i, j)) {

				chess[i][j] = color; color = color * (-1);
				if (whether_end()) {
					ucb[i][j] = 1.0 + sqrt(2.0 * log(totalN));
				}
				else {
					calculate_nj(i, j);
					playout();
					ucb[i][j] = win * 1.0 / nj[i][j] + sqrt(2.0 * log(totalN) / nj[i][j]);

				}
				copytochess();
				win = 0; color = simcolor;
			}
		}
	}

}//MCTS算法，计算ucb值
int algorithm(void) {//范佳玥
	int num = 0;
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			if (!chess[i][j] && notsuicide(i, j)) {
				num++;
			}
		}
	}
	if (num <= 7) return 1;
	else return 0;
}//两种算法转换边界判断
int input_chess(void) {//范佳玥
	ExMessage ms;
	while (true) {
		int flagms = 0;
		ms = getmessage(EX_MOUSE);
		if (ms.message == WM_LBUTTONDOWN) {
			if (ms.x >= 630 && ms.x <= 730 && ms.y >= 120 && ms.y <= 170) {
				draw_chess();
				return 0;
			}
			else if (ms.x >= 630 && ms.x <= 730 && ms.y >= 200 && ms.y <= 250) {
				read(); play(); return 0;
			}
			else if (ms.x >= 630 && ms.x <= 730 && ms.y >= 280 && ms.y <= 330) {
				save(); play(); return 0;
			}
			else if (ms.x >= 630 && ms.x <= 730 && ms.y >= 360 && ms.y <= 410) {
				closegraph(); return 0;
			}
			else {
				for (int t = 35; t <= 435; t += 50) {
					for (int s = 35; s <= 435; s += 50) {
						if (ms.x >= t && ms.x <= t + 30 && ms.y >= s && ms.y <= s + 30) {
							if (playercolor == -1) {
								tx = (t + 15) / 50, ty = (s + 15) / 50;
								int flagend = 0;
								if (chess[tx][ty] != 0) flagend = 1;
								black((t + 15) / 50, (s + 15) / 50);
								if (regret(tx, ty)) {
									flagend = 0;
									if (chess[tx][ty] != 0) flagend = 1;
									black(tx, ty);
								}
								if (ends(tx, ty) || flagend) {
									if (mode2)
										return 2;
									else player_lose();
								}
								flagms++;
								return 1;
								break;
							}
							else {
								tx = (t + 15) / 50, ty = (s + 15) / 50;
								int flagend = 0;
								if (chess[tx][ty] != 0) flagend = 1;
								white((t + 15) / 50, (s + 15) / 50);
								if (regret(tx, ty)) {
									flagend = 0;
									if (chess[tx][ty] != 0) flagend = 1;
									white(tx, ty);
								}
								if (ends(tx, ty) || flagend) {
									if (mode2)
										return 2;
									else player_lose();
								}
								flagms++;
								return 1;
								break;
							}
						}
					}
					if (flagms) break;
				}
			}
		}
		if (flagms) break;
	}
	return 0;
}//控制鼠标下棋
void play(void) {//范佳玥
	while (1) {
		if (color == botcolor) {
			if (!flagmain) {
				efficiency();
				bot_chess(moveX, moveY);
				if (ends(moveX, moveY)) {
					player_win();
				}
				if (algorithm()) flagmain = 1;//判断是否需要转入MCTS
			}
			else {
				UCB();
				double ans = 0; int ansx = 0, ansy = 0;
				for (int i = 1; i <= 9; i++) {
					for (int j = 1; j <= 9; j++) {
						if (ucb[i][j] > ans) {
							ans = ucb[i][j];
							ansx = i; ansy = j;
						}
					}
				}
				if (chess[ansx][ansy] == 0 && border(ansx, ansy)) {
					bot_chess(ansx, ansy);
					if (ends(ansx, ansy)) {
						player_win();
					}
				}
				else {
					int flagpar = 0;
					for (int i = 1; i <= 9; i++) {
						for (int j = 1; j <= 9; j++) {
							if (!chess[i][j]) {
								bot_chess(i, j);
								if (ends(i, j)) {
									player_win();
								}
								flagpar = 1;
								break;
							}
						}
						if (flagpar) break;
					}
				}
			}
		}
		else if (color == playercolor) {
			int s = input_chess();
		}

	}
}//人机博弈
void two_players(void) {// 双人对战  杨子涵
	mode2 = true;
	TCHAR s1[] = L"Berlin Sans FB";
	settextstyle(27, 0, s1);
	TCHAR s2[50] = L"Next: ";
	outtextxy(40, 495, s2);
	setfillcolor(BLACK); fillcircle(125, 510, 12);
	setfillcolor(WHITE); fillcircle(155, 510, 12);
	setcolor(RED); circle(125, 510, 11); circle(125, 510, 12);
	setcolor(WHITE); circle(155, 510, 11);
	setcolor(BLACK); circle(155, 510, 12);
	while (true) {
		if (input_chess() == 2)
		{
			setfillcolor(RGB(255, 161, 161));
			fillrectangle(20, 200, 480, 300);
			if (playercolor == -1)
			{
				TCHAR win[50] = L"White Wins!";
				TCHAR s1[] = L"Consolas";
				settextstyle(50, 0, s1);
				outtextxy(145, 225, win);
			}
			else {
				TCHAR win[50] = L"Black Wins!";
				TCHAR s1[] = L"Consolas";
				settextstyle(50, 0, s1);
				outtextxy(145, 225, win);
			}
			_getch();
			closegraph();
		}
		playercolor *= -1;
		if (playercolor == -1) {
			setcolor(RED); circle(125, 510, 11); circle(125, 510, 12);
			setcolor(WHITE); circle(155, 510, 11);
			setcolor(BLACK); circle(155, 510, 12);
		}
		else {
			setcolor(RED); circle(155, 510, 11); circle(155, 510, 12);
			setcolor(BLACK); circle(125, 510, 11); circle(125, 510, 12);
		}
	}
}


int main(void)
{//范佳玥
	draw_chess();
	mciSendString(TEXT("open music.mp3 alias BGM"), 0, 0, 0);
	mciSendString(TEXT("play BGM"), 0, 0, 0);//bgm
	ExMessage m;
	while (true) {
		m = getmessage(EX_MOUSE);
		if (m.message == WM_LBUTTONDOWN) {//主菜单的选择
			if (m.x >= 630 && m.x <= 730 && m.y >= 120 && m.y <= 170) {
				start();
				play();
			}
			else if (m.x >= 630 && m.x <= 730 && m.y >= 190 && m.y <= 240) {
				read();
				if (mode2) two_players();
				else { 
					setbkmode(TRANSPARENT);
					TCHAR s1[] = L"Berlin Sans FB";
					settextstyle(25, 0, s1);
					TCHAR s2[50] = L"Choose color: ";
					outtextxy(50, 500, s2);
					setfillcolor(BLACK);
					fillcircle(200, 510, 10);
					setfillcolor(WHITE);
					fillcircle(230, 510, 10);
					if (playercolor == -1) {
						setcolor(RED);
						circle(200, 510, 11);
						circle(200, 510, 12);
					}
					else if (playercolor == 1) {
						setcolor(RED);
						circle(230, 510, 11);
						circle(230, 510, 12);						
					}
					play(); 
				}
			}
			else if (m.x >= 630 && m.x <= 730 && m.y >= 265 && m.y <= 315) {
				save();
				if (mode2) two_players();
				else play();
			}
			else if (m.x >= 630 && m.x <= 730 && m.y >= 360 && m.y <= 410) {
				closegraph();
				return 0;
			}
			else if (m.x >= 620 && m.x <= 720 && m.y >= 420 && m.y <= 470) {
				playercolor = -1;
				two_players();
				return 0;
			}
		}
	}
	_getch();
	closegraph();
	return 0;
}

