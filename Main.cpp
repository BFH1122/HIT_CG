#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <math.h>
#include <time.h>


#define ID_Line 1000
#define ID_Fill 1001
#define ID_Clear 1002
#define ID_SUB 1003
#define ID_Y 1004
#define ID_X 1005
#define ID_O 1006
#define ID_MOVE 1007
#define ID_ROLLL 1008
#define ID_ROLLR 998
#define ID_SCLL 1009
#define ID_BIG 1010
#define ID_SMALL 1011
#define ID_Play 1012

using namespace std;

int POINTNUM = 0;//��¼��ĸ���

typedef struct XET
{
	float x;//��ET���бߵ��¶˵��x���꣬��AET������Ϊ����ɨ���ߵĽ��������
	float dx;//dxΪ�ߵ�б�ʵĵ�����
	float ymax;//�ߵ��϶˵��y������
	XET* next;
} AET, NET;

struct point
{
	float x;
	float y;
};
point Points[100];
point window[4];
point boundary[4][2];
int flag = 0;

//ȫ�ֱ���
static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("�����ͼ��ѧ");

HINSTANCE hInst;
UINT WIDTH = 800;//���ڿ��
UINT HEIGHT = 600;//���ڸ߶�
COLORREF bkg_clr = RGB(255, 255, 255);//����ɫ


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);//��Ϣ����
void display(HDC hDC);//WM_PAINT��Ϣ��Ӧ�����ƺ�
void All_Contro(HDC hdc, HWND hWnd);
void Fill_Poly(HDC hdc, COLORREF newcolor);
void drawPoly(HDC hdc);
void InitWindow();
void updateBound();
int encode(point point);
void line(HDC hDC, int x_beg, int y_beg, int x_end, int y_end, COLORREF color = RGB(255, 255, 255), int line_width = 1, int line_style = PS_SOLID);//����
void cohenSutherland(HDC hdc, point start, point end);
void Y_symmetry(HDC hdc);
void X_symmetry(HDC hdc);
void O_symmetry(HDC hdc);
void MOVE_P(HDC hdc);
void ROLL(HDC hdc, float angle);
void B_S(HDC hdc, float xa, float xb);
void Play(HDC hdc, HWND hWnd);

void ellipse(HDC hDC, int x_left, int y_top, int x_right, int y_bottom, COLORREF color = RGB(255, 255, 255), int line_width = 1, int line_style = PS_SOLID);//��Բ

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wcex;//��ʼ��������
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(bkg_clr);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex)) {//ע�ᴰ����
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Application"), NULL);
		return 1;
	}
	hInst = hInstance;
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 200, 100, WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);//��������
	if (!hWnd) {//�������ʧ��
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Win32 Application"), NULL);
		return 1;
	}

	HMENU MainMenu = CreateMenu();
	AppendMenu(MainMenu, MF_STRING, ID_Line, "����");
	SetMenu(hWnd, MainMenu);
	HMENU MainMenu1 = GetMenu(hWnd);
	AppendMenu(MainMenu1, MF_STRING, ID_Fill, "���");
	
	HMENU PopMenu1 = CreatePopupMenu();
	MainMenu1 = GetMenu(hWnd);
	AppendMenu(PopMenu1, MF_STRING, ID_Y, "Y��");
	AppendMenu(PopMenu1, MF_STRING, ID_X, "X��");
	AppendMenu(PopMenu1, MF_STRING, ID_O, "ԭ��");
	AppendMenu(PopMenu1, MF_STRING, ID_MOVE, "ƽ��");
	/*AppendMenu(PopMenu1, MF_STRING, ID_MOVE, "ƽ��");
	AppendMenu(PopMenu1, MF_STRING, ID_MOVE, "ƽ��");*/

	AppendMenu(MainMenu1, MF_STRING | MF_POPUP, UINT(PopMenu1), "ͼ�α任");
	

	HMENU PopMenu2 = CreatePopupMenu();
	MainMenu1 = GetMenu(hWnd);
	AppendMenu(PopMenu2, MF_STRING, ID_ROLLL, "����");
	AppendMenu(PopMenu2, MF_STRING, ID_ROLLR, "����");
	AppendMenu(MainMenu1, MF_STRING | MF_POPUP, UINT(PopMenu2), "��ת");


	HMENU PopMenu3 = CreatePopupMenu();
	MainMenu1 = GetMenu(hWnd);
	AppendMenu(PopMenu3, MF_STRING, ID_BIG, "�Ŵ�");
	AppendMenu(PopMenu3, MF_STRING, ID_SMALL, "��С");
	AppendMenu(MainMenu1, MF_STRING | MF_POPUP, UINT(PopMenu3), "����");
	
	
	MainMenu1 = GetMenu(hWnd);
	AppendMenu(MainMenu1, MF_STRING, ID_SUB, "ֱ�߶ν�ȡ");
	MainMenu1 = GetMenu(hWnd);
	AppendMenu(MainMenu1, MF_STRING, ID_Play, "����");
	

	AppendMenu(MainMenu1, MF_STRING, ID_Clear, "����");
	

	ShowWindow(hWnd, nCmdShow);//��ʾ
	UpdateWindow(hWnd);//����ʾǰ�ٴθ���
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {//��ȡ��Ϣ�����е���Ϣ
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = GetDC(hWnd);
	int x = LOWORD(lParam);//���ĺ�����  
	int y = HIWORD(lParam);//����������  
	switch (message) {
	case WM_COMMAND:
		switch (wParam)
		{
			case ID_Line:
				flag = 1;
				break;
			case ID_Fill:
				flag = 2;
				break;
			case ID_Clear:
				InvalidateRect(hWnd, NULL, true);
				UpdateWindow(hWnd);
				break;
			case ID_SUB:
				flag = 3;
				InitWindow();
				updateBound();
				display(hdc);
				POINTNUM = 0;
				break;
			case ID_Y:
				flag = 4;
				break;
			case ID_X:
				flag = 5;
				break;
			case ID_O:
				flag = 6;
				break;
			case ID_MOVE:
				flag = 7;
				break;
			case ID_ROLLL:
				flag = 8;
				break;
			case ID_ROLLR:
				flag = 9;
				break;
			case ID_BIG:
				flag = 10;
				break;
			case ID_SMALL:
				flag = 11;
				break;
			case ID_Play:
				flag = 12;
				break;
		}
		break;
	case WM_LBUTTONDOWN:
		Points[POINTNUM].x = x;
		Points[POINTNUM].y = y;
		POINTNUM++;
		break;
	case WM_RBUTTONDOWN:
		All_Contro(hdc,hWnd);
		break;
	case WM_PAINT://�ػ�
		hdc = BeginPaint(hWnd, &ps);
		display(hdc);
		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_CHAR://��Ӧ����
		switch (wParam) {
		case 27:    //Esc
			PostQuitMessage(0);
			break;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
void All_Contro(HDC hdc, HWND hWnd)
{
	if (flag == 1) {
		line(hdc, Points[0].x, Points[0].y, Points[1].x, Points[1].y, RGB(0, 255, 0), 3);
		POINTNUM = 0;
	}
	else if (flag == 2)
	{
		display(hdc);
		srand((unsigned)time(NULL));
		int rgb_a = rand() % 256;
		int rgb_b = rand() % 256;
		int rgb_c = rand() % 256;
		Fill_Poly(hdc, RGB(rgb_a, rgb_b, rgb_c));
		POINTNUM = 0;
	}
	else if (flag == 3)
	{
		cohenSutherland(hdc, Points[0], Points[1]);
		POINTNUM = 0;
	}
	else if (flag == 4)
	{
		Y_symmetry(hdc);
		POINTNUM = 0;
	}
	else if (flag == 5)
	{
		X_symmetry(hdc);
		POINTNUM = 0;
	}
	else if (flag == 6)
	{
		O_symmetry(hdc);
		POINTNUM = 0;
	}
	else if (flag == 7)
	{
		MOVE_P(hdc);
		POINTNUM = 0;
	}
	else if (flag == 8)
	{
		ROLL(hdc, 0.3);
		POINTNUM = 0;
	}
	else if (flag == 9)
	{
		ROLL(hdc, -0.3);
		POINTNUM = 0;
	}
	else if (flag == 10)
	{
		B_S(hdc, 2, 3);
		POINTNUM = 0;
	}
	else if (flag == 11)
	{
		B_S(hdc, 0.5, 0.5);
		POINTNUM = 0;
	}
	else if (flag == 12)
	{
		Play(hdc,hWnd);
	}
	else
		POINTNUM = 0;

}

void display(HDC hdc) {
	drawPoly(hdc);
}

void line(HDC hDC, int x_beg, int y_beg, int x_end, int y_end, COLORREF color, int line_width, int line_style) {
	HPEN pen_old;
	pen_old = (HPEN)SelectObject(hDC, CreatePen(line_style, line_width, color));//ѡ�񻭱�
	MoveToEx(hDC, x_beg, y_beg, NULL);
	LineTo(hDC, x_end, y_end);
	SelectObject(hDC, pen_old);
}

void drawPoly(HDC hdc)
{
	for (int i = 0; i<POINTNUM; i++)
	{
		line(hdc,Points[i].x, Points[i].y, Points[(i + 1) % POINTNUM].x, Points[(i + 1) % POINTNUM].y,RGB(255,0,0),3);
	}
}

void Fill_Poly(HDC hdc,COLORREF newcolor)
{
	int MaxY = 0;
	int MinY = 1000000;
	int i;
	for (i = 0; i < POINTNUM; i++)
	{
		//printf("point��x=%f,y=%f\n", Points[i].x, Points[i].y);
		if (Points[i].y > MaxY)
			MaxY = Points[i].y;
		if (Points[i].y < MinY)
			MinY = Points[i].y;
	}

	//printf("MinY=%d    MaxY=%d\n", MinY, MaxY);

	//��ʼ��AET
	AET *pAET = new AET;
	pAET->next = NULL;

	//��ʼ��ET
	NET *pNET[1024];
	for (i = 0; i <= MaxY; i++)
	{
		pNET[i] = new NET;
		pNET[i]->next = NULL;
	}
	//ET��Ľ���
	for (i = 0; i <= MaxY; i++)
	{
		for (int j = 0; j<POINTNUM; j++)
			if (Points[j].y == i)
			{
				if (Points[(j - 1 + POINTNUM) % POINTNUM].y>Points[j].y)//��֤������߶ε���͵�
				{
					NET *p = new NET;
					p->x = Points[j].x;
					p->ymax = Points[(j - 1 + POINTNUM) % POINTNUM].y;
					p->dx = (Points[(j - 1 + POINTNUM) % POINTNUM].x - Points[j].x) / (Points[(j - 1 + POINTNUM) % POINTNUM].y - Points[j].y);
					p->next = pNET[i]->next;
					pNET[i]->next = p;
				}
				if (Points[(j + 1 + POINTNUM) % POINTNUM].y>Points[j].y)
				{
					NET *p = new NET;
					p->x = Points[j].x;
					p->ymax = Points[(j + 1 + POINTNUM) % POINTNUM].y;
					p->dx = (Points[(j + 1 + POINTNUM) % POINTNUM].x - Points[j].x) / (Points[(j + 1 + POINTNUM) % POINTNUM].y - Points[j].y);
					p->next = pNET[i]->next;
					pNET[i]->next = p;
				}
			}
	}
	for (i = 0; i <= MaxY; i++)
	{
		//��AEL��ɾ������y=ymax�ıߣ���ʱ��y=i
		AET *q = pAET;
		NET *p = q->next;
		while (p)
		{
			if (p->ymax <= i)
			{
				q->next = p->next;
				delete p;
				p = q->next;
			}
			else
			{
				q = q->next;
				p = q->next;
			}
		}
		//��AEL��ʣ�µ�ÿһ���ߵ�x+dx
		p = pAET->next;
		while (p)
		{
			p->x = p->x + p->dx;
			p = p->next;
		}
		//���º���AET������
		AET *tq = pAET;
		p = pAET->next;
		tq->next = NULL;
		while (p)
		{
			while (tq->next && p->x >= tq->next->x)
				tq = tq->next;
			NET *s = p->next;
			p->next = tq->next;
			tq->next = p;
			p = s;
			tq = pAET;
		}
		//��ET���е����ݼ���AEL������
		p = pNET[i]->next;
		q = pAET;
		while (p)
		{
			while (q->next && p->x >= q->next->x)
				q = q->next;
			NET *s = p->next;
			p->next = q->next;
			q->next = p;
			p = s;
			q = pAET;
		}
		p = pAET->next;
		while (p && p->next)
		{
			for (float j = p->x; j <= p->next->x; j++)
			{
				SetPixel(hdc, j, i, newcolor);
			}
			p = p->next->next;
		}
	}
}

void InitWindow()
{
	// ���Ͻ�
	window[0].x = 200;
	window[0].y = 200;

	// ���½�
	window[1].x = 200;
	window[1].y = 400;

	// ���½�
	window[2].x = 500;
	window[2].y = 400;

	// ���Ͻ�
	window[3].x = 500;
	window[3].y = 200;

	Points[0] = window[0];
	Points[1] = window[1];
	Points[2] = window[2];
	Points[3] = window[3];
	POINTNUM = 4;
}

void updateBound(void)
{
	//��߽�
	boundary[0][0] = window[0];
	boundary[0][1] = window[1];

	// �ұ߽�
	boundary[1][0] = window[2];
	boundary[1][1] = window[3];

	// �±߽�
	boundary[2][0] = window[1];
	boundary[2][1] = window[2];

	// �ϱ߽�
	boundary[3][0] = window[0];
	boundary[3][1] = window[3];
}

int encode(point point)
{
	int code = 0x0;
	if (point.x < window[1].x)
		code = code | 0x1;
	if (point.x > window[3].x)
		code = code | 0x2;
	if (point.y > window[1].y)
		code = code | 0x4;
	if (point.y < window[3].y)
		code = code | 0x8;
	return code;
}

point getIntersection(point line1[2], point line2[2])
{
	float dx1 = line1[1].x - line1[0].x, dy1 = line1[1].y - line1[0].y;
	float dx2 = line2[1].x - line2[0].x, dy2 = line2[1].y - line2[0].y;
	point intersection;

	if (dx1 != 0 && dx2 != 0)   // �������ֱ�߶���б��
	{
		// ��ֱ�ߵĲ�����y = ax + b
		float a1 = dy1 / dx1, b1 = line1[0].y - a1 * line1[0].x;
		float a2 = dy2 / dx2, b2 = line2[0].y - a2 * line2[0].x;

		intersection.x = (b2 - b1) / (a1 - a2);
		intersection.y = a1 * intersection.x + b1;
	}
	else if (dx1 == 0 && dx2 != 0)   // ���line1��ֱ��x��
	{
		float a2 = dy2 / dx2, b2 = line2[0].y - a2 * line2[0].x;

		intersection.x = line1[0].x;
		intersection.y = a2 * intersection.x + b2;
	}
	else if (dx1 != 0 && dx2 == 0)   // ���line2��ֱ��x��
	{
		float a1 = dy1 / dx1, b1 = line1[0].y - a1 * line1[0].x;

		intersection.x = line2[0].x;
		intersection.y = a1 * intersection.x + b1;
	}
	else   // �������ֱ�߶���ֱ��x�ᣬҲ����ƽ�У���ô�޽��㡣��NAN = not a number��
	{
		intersection.x = NAN;
		intersection.y = NAN;
	}
	return intersection;
}

void cohenSutherland(HDC hdc,point start,point end)
{
	int code0 = encode(start);
	int code1 = encode(end);

	if (code0 == 0 && code1 == 0)  
		line(hdc, start.x, start.y, end.x, end.y, RGB(0, 255, 0), 3);
	else
	{
		point in_Point[2];   
		in_Point[0] = start;
		in_Point[1] = end;
		for (int i = 0; i < 4; i++)
		{
			int temp = (int)pow(2, i);
			int current0 = (code0 & temp) >> i;
			int current1 = (code1 & temp) >> i;

			if (current0 == current1)
			{
				if(current0==1)
				{
					line(hdc, in_Point[0].x, in_Point[0].y, in_Point[1].x, in_Point[1].y, RGB(255, 0, 0), 3);
					return;
				}
				else  
					continue;
			}
			else   
			{
				point p = getIntersection(in_Point, boundary[i]);
				if (p.x != NAN && p.y != NAN)
				{
					if (current0 == 1)   
					{
						line(hdc, p.x, p.y, in_Point[0].x, in_Point[0].y, RGB(255, 0, 0), 3);
						in_Point[0] = p;
						code0 = encode(in_Point[0]);
					}
					else  
					{
						line(hdc, p.x, p.y, in_Point[1].x, in_Point[1].y, RGB(255, 0, 0), 3);
						in_Point[1] = p;
						code1 = encode(in_Point[1]);
					}
				}
			}
		}
		line(hdc, in_Point[0].x, in_Point[0].y, in_Point[1].x, in_Point[1].y, RGB(0, 255, 0), 3);
	}
}

point mulmatrix(point pre,float matrix[3][3]) 
{
	pre.x = pre.x*matrix[0][0] + pre.y*matrix[1][0] + matrix[2][0];
	pre.y = pre.x*matrix[0][1] + pre.y*matrix[1][1] + matrix[2][1];
	return pre;
}

void Y_symmetry(HDC hdc)
{
	drawPoly(hdc);
	float matrix[3][3] = { {-1,0,0} ,{0,1,0} ,{800,0,1} };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	drawPoly(hdc);
}
void X_symmetry(HDC hdc)
{
	drawPoly(hdc);
	float matrix[3][3] = { { 1,0,0 } ,{ 0,-1,0 } ,{ 0,600,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	drawPoly(hdc);
}
void O_symmetry(HDC hdc)
{
	drawPoly(hdc);
	float matrix[3][3] = { { -1,0,0 } ,{ 0,-1,0 } ,{ 800,600,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	drawPoly(hdc);
}

void MOVE_P(HDC hdc) {
	int x = Points[POINTNUM - 1].x - Points[POINTNUM - 2].x;
	int y = Points[POINTNUM - 1].y - Points[POINTNUM - 2].y;
	POINTNUM = POINTNUM - 2;
	display(hdc);
	float matrix[3][3] = { { 1,0,0 } ,{ 0,1,0 } ,{ x,y,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	drawPoly(hdc);
}

void ROLL(HDC hdc,float angle)
{
	int x = Points[0].x;
	int y = Points[0].y;
	display(hdc);
	float matrix[3][3] = { { 1,0,0 } ,{ 0,1,0 } ,{ -x,-y,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	float matrix1[3][3] = { { cos(angle),sin(angle),0 } ,{ -sin(angle),cos(angle),0 } ,{ 0,0,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix1);
	}
	float matrix2[3][3] = { { 1,0,0 } ,{ 0,1,0 } ,{ x,y,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix2);
	}
	drawPoly(hdc);
}

void B_S(HDC hdc,float xa,float xb)
{
	int x = Points[0].x;
	int y = Points[0].y;
	display(hdc);
	float matrix[3][3] = { { 1,0,0 } ,{ 0,1,0 } ,{ -x,-y,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix);
	}
	float matrix1[3][3] = { {xa,0,0 } ,{0,xb,0 } ,{ 0,0,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix1);
	}
	float matrix2[3][3] = { { 1,0,0 } ,{ 0,1,0 } ,{ x+100,y+100,1 } };
	for (int i = 0; i < POINTNUM; i++)
	{
		Points[i] = mulmatrix(Points[i], matrix2);
	}
	drawPoly(hdc);
}

void setTimer()
{
	for (int i = 0; i < 500000; i++) {

	}
}

void rect(HDC hDC, int x_left, int y_top, int x_right, int y_bottom, COLORREF color, int line_width, int line_style) {
	HPEN pen_old;
	pen_old = (HPEN)SelectObject(hDC, CreatePen(line_style, line_width, color));//ѡ�񻭱�
	SelectObject(hDC, GetStockObject(NULL_BRUSH));
	Rectangle(hDC, x_left, y_top, x_right, y_bottom);
	SelectObject(hDC, pen_old);
}
void ellipse(HDC hDC, int x_left, int y_top, int x_right, int y_bottom, COLORREF color, int line_width, int line_style) {
	HPEN pen_old;
	pen_old = (HPEN)SelectObject(hDC, CreatePen(line_style, line_width, color));//ѡ�񻭱�
	SelectObject(hDC, GetStockObject(NULL_BRUSH));
	Ellipse(hDC, x_left, y_top, x_right, y_bottom);
	SelectObject(hDC, pen_old);
}
void Play(HDC hdc, HWND hWnd)
{
	int x = 0;
	int y = 0;
	int xflag = 0;
	int yflag = 0;
	while (true)
	{
		ellipse(hdc, x-40, y-40, x-30, y-30, RGB(0, 255, 0), 3);
		ellipse(hdc, x, y, x + 10, y + 10, RGB(0, 255, 0), 3);
		if (x + 10 > 800)
			xflag = 1;
		else if (x <= 0)
			xflag = 0;
		if (y + 10 > 600)
			yflag = 1;
		else if (y <= 0)
			yflag = 0;
		if (xflag == 1)
			x--;
		else
			x++;
		if (yflag == 1)
			y--;
		else
			y++;
		setTimer();
		InvalidateRect(hWnd, NULL, true);
		UpdateWindow(hWnd);
	}
}