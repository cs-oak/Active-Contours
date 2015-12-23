
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,400,400,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;

strcpy(filename,"");
OriginalImage=NULL;
ROWS=COLS=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}




LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)

{
HMENU				hMenu;
OPENFILENAME		ofn;
FILE				*fpt;
HDC					hDC;
char				header[320],text[320];
int					BYTES,xPos,yPos;

switch (uMsg)
  {
  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
	  case ID_SHOWPIXELCOORDS:
		//ShowPixelCoords=(ShowPixelCoords+1)%2;
		PaintImage();
		break;
	  case ID_FILE_LOAD:
		if (OriginalImage != NULL)
		  {
		  free(OriginalImage);
		  OriginalImage=NULL;
		  }
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=filename;
		filename[0]=0;
		ofn.nMaxFile=MAX_FILENAME_CHARS;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrFilter = "PNM files\0*.pnm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s\n %d\n %d\n %d\n",header,&COLS,&ROWS,&BYTES);
		if (strcmp(header,"P6") != 0  ||  BYTES != 255)
		  {
		  MessageBox(NULL,"Not a PNM image",filename,MB_OK | MB_APPLMODAL);
		  fclose(fpt);
		  break;
		  }
		rgb_buffer = (unsigned char *)calloc(ROWS*COLS*3, 1);
		OriginalImage=(unsigned char *)calloc(ROWS*COLS,1);
		header[0]=fgetc(fpt);	/* whitespace character after header */
		fread(rgb_buffer,1,ROWS*COLS*3,fpt);
		j = 0;
		for (i = 0; i < (ROWS*COLS * 3); i+=3)
		{
			OriginalImage[j] = (rgb_buffer[i] + rgb_buffer[i + 1] + rgb_buffer[i + 2]) / 3;
			j++;
		}
		fclose(fpt);
		gradient1 = (int *)malloc(ROWS*COLS*sizeof(int));
		gradient2 = (int *)malloc(ROWS*COLS*sizeof(int));
		gradient = (unsigned char *)malloc(ROWS*COLS*sizeof(unsigned char));
		for (r = 1; r<ROWS - 1; r++)
			for (c = 1; c<COLS - 1; c++)
			{
				sum = 0;
				for (r2 = -1; r2 <= 1; r2++)
					for (c2 = -1; c2 <= 1; c2++)
					{
						sum += OriginalImage[(r + r2)*COLS + (c + c2)] * template1[(1 + r2) * 3 + (1 + c2)];
					}
				gradient1[r*COLS + c] = sum;
				sum = 0;
				for (r2 = -1; r2 <= 1; r2++)
					for (c2 = -1; c2 <= 1; c2++)
					{
						sum += OriginalImage[(r + r2)*COLS + (c + c2)] * template2[(1 + r2) * 3 + (1 + c2)];
					}
				gradient2[r*COLS + c] = sum;
			}
		for (r = 0; r<ROWS; r++)
			for (c = 0; c<COLS; c++)
			{
				sum = sqrt((gradient1[r*COLS + c] * gradient1[r*COLS + c]) + (gradient2[r*COLS + c] * gradient2[r*COLS + c]));
				gradient[r*COLS + c] = (unsigned char)sum;
				//if (gradient[r*COLS + c] < 60) gradient[r*COLS + c] = 0;
				//if (gradient[r*COLS + c] > 150) gradient[r*COLS + c] = 255;
			}
		SetWindowText(hWnd,filename);
		PaintImage();
		break;

      case ID_FILE_QUIT:
        DestroyWindow(hWnd);
        break;
      }
    break;
  case WM_SIZE:		  /* could be used to detect when window size changes */
	PaintImage();
	hDC = GetDC(MainWnd);
	for (i = 0; i < 120; i++)
	{
		SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
	}
	for (i = 0; i < 120; i++)
	{
		SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
	}
	for (i = 0; i < buff_count / 5; i++)
	{
		SetPixel(hDC, rbx[i], rby[i], RGB(255, 0, 0));
	}
	for (i = 0; i < cbuff; i++)
	{
		if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
	}
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_PAINT:
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_LBUTTONDOWN:
	  if (wParam == MK_LBUTTON)
	  {
		  hDC = GetDC(MainWnd);
		  xPos = LOWORD(lParam);
		  yPos = HIWORD(lParam);
		  if (GetPixel(hDC, xPos, yPos) == RGB(255, 0, 0))
		  {
			  state = 1;
			  for (i = 0; i < (buff_count / 5); i++)
			  {
				  fgt = 0;
				  if (rbx[i] == xPos) x = i;
				  if (rby[i] == yPos) y = i;
				  if (rbx[i] == xPos && rby[i] == yPos)
				  {
					  cbuff = buff_count / 5;
					  ccx = (int *)malloc((cbuff)*sizeof(int));
					  ccy = (int *)malloc((cbuff)*sizeof(int));
					  for (k = 0; k < cbuff; k++)
					  {
						  ccx[k] = rbx[k];
						  ccy[k] = rby[k];
					  }
					  cav = 0.0;
					  for (k = 0; k < cbuff; k++)
					  {
						  cav += sqrt(abs(ccx[k] - ccx[(k + 1) % cbuff])*abs(ccx[k] - ccx[(k + 1) % cbuff]) + abs(ccy[k] - ccy[(k + 1) % cbuff])*abs(ccy[k] - ccy[(k + 1) % cbuff]));
					  }
					  cav /= cbuff;
					  cox = ccx[x];
					  coy = ccy[x];
				  }
			  }
		  }
		  if (GetPixel(hDC, xPos, yPos) == RGB(0, 0, 255))
		  {
			  state = 2;
			  for (i = 0; i < 120; i++)
			  {
				  fgt = 0;
				  if (baloonx[i] == xPos) x = i;
				  if (baloony[i] == yPos) y = i;
				  if (baloonx[i] == xPos && baloony[i] == yPos)
				  {
					  for (i = 0; i < 120; i++)
					  {
						  ccx1[i] = baloonx[i];
						  ccy1[i] = baloony[i];
					  }
					  cav1 = 0.0;
					  for (k = 0; k < cbuff; k++)
					  {
						  cav1 += sqrt(abs(ccx1[k] - ccx1[(k + 1) % 120])*abs(ccx1[k] - ccx1[(k + 1) % 120]) + abs(ccy1[k] - ccy1[(k + 1) % 120])*abs(ccy1[k] - ccy1[(k + 1) % 120]));
					  }
					  cav1 /= 120;
					  cox1 = ccx1[x];
					  coy1 = ccy1[x];
				  }
			  }
		  }
		  if(state == 0) buff_count = 0;
		  ShowPixelCoords = 1;
	  }
  case WM_LBUTTONUP:
	  if (wParam != MK_LBUTTON)
	  {
		  if (state != 1 && state != 2)
		  {
			  rbx = (int *)malloc((buff_count / 5)*sizeof(int));
			  rby = (int *)malloc((buff_count / 5)*sizeof(int));
			  for (i = 0; i < (buff_count / 5); i++)
			  {
				  rbx[i] = bufferx[i * 5];
				  rby[i] = buffery[i * 5];
			  }
			  PaintImage();
			  hDC = GetDC(MainWnd);
			  for (i = 0; i < buff_count / 5; i++)
			  {
				  SetPixel(hDC, rbx[i], rby[i], RGB(255, 0, 0));
			  }
			  for (i = 0; i < 120; i++)
			  {
				  SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
			  }
			  for (i = 0; i < 120; i++)
			  {
				  SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
			  }
			  for (i = 0; i < cbuff; i++)
			  {
				  if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
			  }
			  circe = 0;
		  }
		  if (state == 1) state = 0;
		  if (state == 2) state = 0;
		  ShowPixelCoords = 0;
	  }
  case WM_RBUTTONDOWN:
	  if (wParam == MK_RBUTTON)
	  {
		  circe = 1;
	  }
  case WM_RBUTTONUP:
	  if (wParam == MK_LBUTTON)
		  break;
	  if (wParam != MK_RBUTTON)
	  {
		  alr = 0;
		  if (circe == 1)
		  {
			  hDC = GetDC(MainWnd);
			  xPos = LOWORD(lParam);
			  yPos = HIWORD(lParam);
			  j = 0;
			  for (i = 0; i < 360; i++)
			  {
				  xt = (int)(10 * cos(i * 3.1415 / 180));
				  yt = (int)(10 * sin(i * 3.1415 / 180));
				  if (i % 3 == 0)
				  {
					  baloonx[j] = xPos+xt;
					  baloony[j] = yPos+yt;
					  j++;
				  }
			  }
			  PaintImage();
			  for (i = 0; i < 120; i++)
			  {
				  SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
			  }
			  for (i = 0; i < 120; i++)
			  {
				  SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
			  }
			  for (i = 0; i < buff_count/5; i++)
			  {
				  SetPixel(hDC, rbx[i], rby[i], RGB(255, 0, 0));
			  }
			  for (i = 0; i < cbuff; i++)
			  {
				  if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
			  }
			  circe = 0;
		  }
	  }
  case WM_MOUSEMOVE:
	  hDC = GetDC(MainWnd);
	  xPos = LOWORD(lParam);
	  yPos = HIWORD(lParam);
	  if (GetPixel(hDC, xPos, yPos) == RGB(255, 0, 0))
	  {
		  sprintf(text, "RED");
		  TextOut(hDC, 0, 0, text, strlen(text));
		  //ReleaseDC(MainWnd,hDC);
	  }
	  if (GetPixel(hDC, xPos, yPos) == RGB(0, 0, 255))
	  {
		  sprintf(text, "BLUE");
		  TextOut(hDC, 50, 0, text, strlen(text));
		  //ReleaseDC(MainWnd,hDC);
	  }
	  if (ShowPixelCoords == 1)
	  {
		  if (state == 1)
		  {
			  if (xPos >= 0 && xPos < COLS  &&  yPos >= 0 && yPos < ROWS)
			  {
				  ccx[x] = xPos;
				  ccy[x] = yPos;
				  PaintImage();
				  for (i = 0; i < 120; i++)
				  {
					  SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
				  }
				  for (i = 0; i < cbuff; i++)
				  {
					  SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
				  }
				  for (i = 0; i < 120; i++)
				  {
					  SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
				  }
			  }
		  }
		  if (state == 2)
		  {
			  if (xPos >= 0 && xPos < COLS  &&  yPos >= 0 && yPos < ROWS)
			  {
				  ccx1[x] = xPos;
				  ccy1[y] = yPos;
				  PaintImage();
				  for (i = 0; i < 120; i++)
				  {
					  SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
				  }
				  for (i = 0; i < cbuff; i++)
				  {
					  if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
				  }
				  for (i = 0; i < buff_count / 5; i++)
				  {
					  SetPixel(hDC, rbx[i], rby[i], RGB(255, 0, 0));
				  }
			  }
		  }
		  else
		  {
			  if (xPos >= 0 && xPos < COLS  &&  yPos >= 0 && yPos < ROWS)
			  {
				  bufferx[buff_count] = xPos;
				  buffery[buff_count] = yPos;
				  SetPixel(hDC, xPos, yPos, RGB(255, 0, 0));
				  buff_count++;
			  }
		  }
	  }
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_KEYDOWN:
	if (wParam == 's'  ||  wParam == 'S')
	  PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
	if (wParam == 'c' || wParam == 'C')
	{
	}
	if ((TCHAR)wParam == '1')
	  {
	  TimerRow=TimerCol=0;
	  SetTimer(MainWnd,TIMER_SECOND,10,NULL);	/* start up 10 ms timer */
	  }
	if ((TCHAR)wParam == '2')
	  {
	  ThreadRunning2 = 1;
	  _beginthread(custom_contour, 0, MainWnd);
	  }
	if ((TCHAR)wParam == '3')
	  {
	  ThreadRunning=1;
	  _beginthread(rb_contour,0,MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
	  }
 	if ((TCHAR)wParam == '4')
	  {
		  ThreadRunning1 = 1;
		  _beginthread(bl_contour,0,MainWnd);	
	  }
	if ((TCHAR)wParam == '5')
	{
		ThreadRunning3 = 1;
		_beginthread(custom_contour1, 0, MainWnd);
	}
	if ((TCHAR)wParam == '6')
	{
		PaintImage();
	}
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
	hDC=GetDC(MainWnd);
	SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
	ReleaseDC(MainWnd,hDC);
	TimerRow++;
	TimerCol+=2;
	break;
  case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
    break;
  }

hMenu=GetMenu(MainWnd);
if (ShowPixelCoords == 1)
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);

DrawMenuBar(hWnd);

return(0L);
}




void PaintImage()

{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
unsigned char		*DisplayImage;

if (OriginalImage == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
  for (c=0; c<COLS; c++)
	DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8; 
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);

free(DisplayImage);
free(bm_info);
}




void rb_contour(HWND AnimationWindowHandle)

{
HDC		hDC;
float		max1 = 18.0, min1 = 0.0;
float		max2, min2;
float		max3 = 65025.0, min3 = 0.0;
float		i1, i2, e1;
float		sum1, dist, mini;
int			ct, j, tempx, tempy;
long int	cx, cy;

ct = 0;
while (ThreadRunning == 1 && state != 1)
  {
  hDC=GetDC(MainWnd);

  sum = 0;
  cx = 0;
  cy = 0;
  for (j = 0; j < (buff_count / 5); j++){
	  cx += rbx[j];
	  cy += rby[j];
  }
  cx /= (buff_count / 5);
  cy /= (buff_count / 5);

  /* calculate energy terms for contours in 7x7 window */
  for (j = 0; j<(buff_count / 5); j++) {
	  for (r2 = -3; r2 <= 3; r2++)
		  for (c2 = -3; c2 <= 3; c2++)
		  {
			  dist = abs(rbx[j]+c2 - cx)*abs(rbx[j]+c2 - cx) + abs(rby[j]+r2 - cy)*abs(rby[j]+r2 - cy);
			  if (r2 == -3 && c2 == -3) min2 = max2 = dist;
			  else{
				  if (dist > max2) max2 = dist;
				  if (dist < min2) min2 = dist;
			  }
		  }

	  for (r2 = -3; r2 <= 3; r2++)
		  for (c2 = -3; c2 <= 3; c2++)
		  {
			  dist = abs(rbx[j] + c2 - cx)*abs(rbx[j] + c2 - cx) + abs(rby[j] + r2 - cy)*abs(rby[j] + r2 - cy);
			  if ((r2 == 0) && (c2 == 0)) i1 = 0;
			  else
				  i1 = (abs(rbx[j] - rbx[j] + c2)*abs(rbx[j] - rbx[j] + c2) + abs(rby[j] - rby[j] + r2)*abs(rby[j] - rby[j] + r2) - min1) / (max1 - min1);
			  i2 = (dist - min2) / (max2 - min2);
			  e1 = (gradient[(rby[j] + r2)*COLS + (rbx[j] + c2)] * gradient[(rby[j] + r2)*COLS + (rbx[j] + c2)] - min3) / (max3 - min3);
			  e1 *= 1.4;
			  if ((r2 == -3) && (c2 == -3)){
				  mini = i1 + i2 - e1;
				  tempx = rbx[j] + c2;
				  tempy = rby[j] + r2;
			  }
			  else {
				  if (((i1 + i2 - e1) < mini)){
					  mini = i1 +i2 - e1;
					  tempx = rbx[j] + c2;
					  tempy = rby[j] + r2;
				  }
			  }
		  }
	  rbx[j] = tempx;
	  rby[j] = tempy;
	}
  /* Draw during iterations */

  PaintImage();
  for (i = 0; i < 120; i++)
  {
	  SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
  }
  for (i = 0; i < 120; i++)
  {
	  SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
  }
  for (i = 0; i < cbuff; i++)
  {
	  if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
  }
  for (i = 0; i < buff_count / 5; i++)
  {
	  for (r2 = -3; r2 <= 3; r2++)
		  for (c2 = -3; c2 <= 3; c2++)
			  SetPixel(hDC, rbx[i] + c2, rby[i] + r2, RGB(255, 0, 0));
  }
  ct++;
  if (ct == 50) ThreadRunning = 0;

  /* Sleep thread 10ms for viewing */
  Sleep(10);
  }
}

void bl_contour(HWND AnimationWindowHandle)

{
	HDC		hDC;
	float		max1 = 18.0, min1 = 0.0;
	float		max2, min2;
	float		max3 = 65025.0, min3 = 0.0;
	float		i1, i2, e1;
	float		sum1, dist, mini;
	int			ct, j, tempx, tempy;
	long int	cx, cy;

	ct = 0;
	while (ThreadRunning1 == 1 && state != 2)
	{
		hDC = GetDC(MainWnd);

		sum = 0;
		cx = 0;
		cy = 0;
		for (j = 0; j < 120; j++){
			cx += baloonx[j];
			cy += baloony[j];
		}
		cx /= 120;
		cy /= 120;

		/* calculate energy terms for contours in 7x7 window */
		for (j = 0; j<120; j++) {
			for (r2 = -5; r2 <= 5; r2++)
				for (c2 = -5; c2 <= 5; c2++)
				{
					dist = abs(baloonx[j] + c2 - cx)*abs(baloonx[j] + c2 - cx) + abs(baloony[j] + r2 - cy)*abs(baloony[j] + r2 - cy);
					if (r2 == -5 && c2 == -5) min2 = max2 = dist;
					else{
						if (dist > max2) max2 = dist;
						if (dist < min2) min2 = dist;
					}
				}

			for (r2 = -5; r2 <= 5; r2++)
				for (c2 = -5; c2 <= 5; c2++)
				{
					dist = abs(baloonx[j] + c2 - cx)*abs(baloonx[j] + c2 - cx) + abs(baloony[j] + r2 - cy)*abs(baloony[j] + r2 - cy);
					if ((r2 == 0) && (c2 == 0)) i1 = 0;
					else
						i1 = (abs(baloonx[j] - baloonx[j] + c2)*abs(baloonx[j] - baloonx[j] + c2) + abs(baloony[j] - baloony[j] + r2)*abs(baloony[j] - baloony[j] + r2) - min1) / (max1 - min1);
					i2 = (dist - max2) / (min2 - max2);
					e1 = (gradient[(baloony[j] + r2)*COLS + (baloonx[j] + c2)] * gradient[(baloony[j] + r2)*COLS + (baloonx[j] + c2)] - min3) / (max3 - min3);
					i1 *= 0.5;
					i2 *= 1.3;
					e1 *= 0.4;
					if ((r2 == -5) && (c2 == -5)){
						mini = i1 + i2 - e1;
						tempx = baloonx[j] + c2;
						tempy = baloony[j] + r2;
					}
					else {
						if (((i1 + i2 - e1) < mini)){
							mini = i1 + i2 - e1;
							tempx = baloonx[j] + c2;
							tempy = baloony[j] + r2;
						}
					}
				}
			baloonx[j] = tempx;
			baloony[j] = tempy;
		}
		/* Draw during iterations */

		PaintImage();
		for (i = 0; i < 120; i++)
		{
			SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
		}
		for (i = 0; i < 120; i++)
		{
			for (r2 = -3; r2 <= 3; r2++)
				for (c2 = -3; c2 <= 3; c2++)
					SetPixel(hDC, baloonx[i] + c2, baloony[i] + r2, RGB(0, 0, 255));
		}
		for (i = 0; i < cbuff; i++)
		{
			if (ccx && ccy) SetPixel(hDC, ccx[i], ccy[i], RGB(255, 255, 0));
		}
		for (i = 0; i < buff_count / 5; i++)
		{
			SetPixel(hDC, rbx[i], rby[i], RGB(255, 0, 0));
		}
		ct++;
		alr++;
		if (ct == 80) ThreadRunning1 = 0;

		/* Sleep thread 10ms for viewing */
		Sleep(10);
	}
}

void custom_contour1(HWND AnimationWindowHandle)

{
	HDC		hDC;
	float		max1 = 18.0, min1 = 0.0;
	float		max2, min2, min4, max4;
	float		max3 = 65025.0, min3 = 0.0;
	float		i1, i2, i3, e1;
	float		sum1, dist, mini, cdist;
	int			ct, j, tempx, tempy, cx, cy;
	int			w;
	float		h;

	ct = 0;
	w = (int)(0.15*120);
	if (w < 1 && w > 0) w = 1;
	h = sqrt(abs(cox1 - ccx1[x])*abs(cox1 - ccx1[x]) + abs(coy1 - ccy1[x])*abs(coy1 - ccy1[x]));
	h += h*1.3;
	h /= w;
	cdist = cx = cy = 0;

	for (j = 0; j < 120; j++)
	{
		if (j != x)
		{
			cx += ccx1[j];
			cy += ccy1[j];
		}
	}
	cx /= 119;
	cy /= 119;

	while (ThreadRunning3 == 1 && state != 2)
	{
		hDC = GetDC(MainWnd);

		/* calculate energy terms for contours in 7x7 window */

		for (j = (x - w); j <= (x + w); j++)
		{
			if (j != x)
			{
				for (r2 = -3; r2 <= 3; r2++)
					for (c2 = -3; c2 <= 3; c2++)
					{
						if (j < x)
						{
							dist = sqrt(abs(ccx1[j%120] + c2 - ccx1[(j + 1) % 120])*abs(ccx1[j%120] + c2 - ccx1[(j + 1) % 120]) + abs(ccy1[j%120] + r2 - ccy1[(j + 1) % 120])*abs(ccy1[j%120] + r2 - ccy1[(j + 1) % 120]));
							if (r2 == -3 && c2 == -3) min2 = max2 = dist;
							else
							{
								if (dist < min2) min2 = dist;
								if (dist > max2) max2 = dist;
							}
						}
						if (j > x)
						{
							dist = sqrt(abs(ccx1[j % 120] + c2 - ccx1[(j + 1) % 120])*abs(ccx1[j % 120] + c2 - ccx1[(j + 1) % 120]) + abs(ccy1[j % 120] + r2 - ccy1[(j + 1) % 120])*abs(ccy1[j % 120] + r2 - ccy1[(j + 1) % 120]));
							if (r2 == -3 && c2 == -3) min2 = max2 = dist;
							else
							{
								if (dist < min2) min2 = dist;
								if (dist > max2) max2 = dist;
							}
						}
					}

				for (r2 = -3; r2 <= 3; r2++)
					for (c2 = -3; c2 <= 3; c2++)
					{
						if (j < x) dist = sqrt(abs(ccx1[j%120] + c2 - ccx1[(j + 1) % 120])*abs(ccx1[j%120] + c2 - ccx1[(j + 1) % 120]) + abs(ccy1[j%120] + r2 - ccy1[(j + 1) % 120])*abs(ccy1[j%120] + r2 - ccy1[(j + 1) % 120]));
						if (j > x) dist = sqrt(abs(ccx1[j%120] + c2 - ccx1[(j - 1) % 120])*abs(ccx1[j%120] + c2 - ccx1[(j - 1) % 120]) + abs(ccy1[j%120] + r2 - ccy1[(j - 1) % 120])*abs(ccy1[j%120] + r2 - ccy1[(j - 1) % 120]));
						if ((r2 == 0) && (c2 == 0)) i1 = 0;
						else
							i1 = (abs(ccx1[j%120] - ccx1[j%120] + c2)*abs(ccx1[j%120] - ccx1[j%120] + c2) + abs(ccy1[j%120] - ccy1[j%120] + r2)*abs(ccy1[j%120] - ccy1[j%120] + r2) - min1) / (max1 - min1);
						i2 = (dist - min2) / (max2 - min2);
						if ((sqrt(abs(ccx1[j%120] + c2 - ccx1[x])*abs(ccx1[j%120] + c2 - ccx1[x]) + abs(ccy1[j%120] + r2 - ccy1[x])*abs(ccy1[j%120] + r2 - ccy1[x]))) < (abs(x - j)*h*1.5)) i2 = 1.0;
						if (dist < cav1*360 && (j != x-1 || j != x+1) ) i2 = 1.0;
						e1 = (gradient[(ccy1[j%120] + r2)*COLS + (ccx1[j%120] + c2)] * gradient[(ccy1[j%120] + r2)*COLS + (ccx1[j%120] + c2)] - min3) / (max3 - min3);
						i1 *= 0.8;
						i2 *= 1.8;
						e1 *= 0.25;
						if ((r2 == -3) && (c2 == -3)){
							mini = i1 + i2 - e1;
							tempx = ccx1[j%120] + c2;
							tempy = ccy1[j%120] + r2;
						}
						else {
							if ((i1 + i2 - e1) < mini){
								mini = i1 + i2 - e1;
								tempx = ccx1[j%120] + c2;
								tempy = ccy1[j%120] + r2;
							}
						}
					}
				ccx1[j%120] = tempx;
				ccy1[j%120] = tempy;
			}
		}


		/* Draw during iterations */

		PaintImage();
		for (i = 0; i < 120; i++)
		{
			for (r2 = -3; r2 <= 3; r2++)
				for (c2 = -3; c2 <= 3; c2++)
					SetPixel(hDC, ccx1[i]+c2, ccy1[i]+r2, RGB(151, 255, 255));
		}
		for (i = 0; i < cbuff; i++)
		{
			if (ccx && ccy) SetPixel(hDC, ccx[i] + c2, ccy[i] + r2, RGB(255, 255, 0));
		}
		ct++;
		fgt++;
		if (ct == 100)
		{ 

			ThreadRunning3 = 0;
		}

		/* Sleep thread 10ms for viewing */
		Sleep(10);
	}
}

void custom_contour(HWND AnimationWindowHandle)

{
	HDC		hDC;
	float		max1 = 18.0, min1 = 0.0;
	float		max2, min2, min4, max4;
	float		max3 = 65025.0, min3 = 0.0;
	float		i1, i2, i3, e1;
	float		sum1, dist, mini, cdist;
	int			ct, j, tempx, tempy, cx, cy;
	int			w;
	float		h;

	ct = 0;
	w = (int)(0.30*cbuff);
	if (w < 1 && w > 0) w = 1;
	h = sqrt(abs(cox - ccx[x])*abs(cox - ccx[x]) + abs(coy - ccy[x])*abs(coy - ccy[x]));
	h += h*0.15;
	h /= w;
	cdist = cx = cy = 0;

	for (j = 0; j < cbuff; j++)
	{
		if (j != x)
		{
			cx += ccx[j];
			cy += ccy[j];
		}
	}
	cx /= cbuff - 1;
	cy /= cbuff - 1;

	while (ThreadRunning2 == 1 && state != 1)
	{
		hDC = GetDC(MainWnd);

		/* calculate energy terms for contours in 7x7 window */

		for (j = (x - w); j <= (x + w); j++)
		{
			if (j != x)
			{
				for (r2 = -3; r2 <= 3; r2++)
					for (c2 = -3; c2 <= 3; c2++)
					{
						if (j < x)
						{
							dist = sqrt(abs(ccx[j%cbuff] + c2 - ccx[(j + 1) % cbuff])*abs(ccx[j%cbuff] + c2 - ccx[(j + 1) % cbuff]) + abs(ccy[j%cbuff] + r2 - ccy[(j + 1) % cbuff])*abs(ccy[j%cbuff] + r2 - ccy[(j + 1) % cbuff]));
							if (r2 == -3 && c2 == -3) min2 = max2 = dist;
							else
							{
								if (dist < min2) min2 = dist;
								if (dist > max2) max2 = dist;
							}
						}
						if (j > x)
						{
							dist = sqrt(abs(ccx[j%cbuff] + c2 - ccx[(j - 1) % cbuff])*abs(ccx[j%cbuff] + c2 - ccx[(j - 1) % cbuff]) + abs(ccy[j%cbuff] + r2 - ccy[(j - 1) % cbuff])*abs(ccy[j%cbuff] + r2 - ccy[(j - 1) % cbuff]));
							if (r2 == -3 && c2 == -3) min2 = max2 = dist;
							else
							{
								if (dist < min2) min2 = dist;
								if (dist > max2) max2 = dist;
							}
						}
					}

				for (r2 = -3; r2 <= 3; r2++)
					for (c2 = -3; c2 <= 3; c2++)
					{
						if (j < x) dist = sqrt(abs(ccx[j%cbuff] + c2 - ccx[(j + 1) % cbuff])*abs(ccx[j%cbuff] + c2 - ccx[(j + 1) % cbuff]) + abs(ccy[j%cbuff] + r2 - ccy[(j + 1) % cbuff])*abs(ccy[j%cbuff] + r2 - ccy[(j + 1) % cbuff]));
						if (j > x) dist = sqrt(abs(ccx[j%cbuff] + c2 - ccx[(j - 1) % cbuff])*abs(ccx[j%cbuff] + c2 - ccx[(j - 1) % cbuff]) + abs(ccy[j%cbuff] + r2 - ccy[(j - 1) % cbuff])*abs(ccy[j%cbuff] + r2 - ccy[(j - 1) % cbuff]));
						if ((r2 == 0) && (c2 == 0)) i1 = 0;
						else
							i1 = (abs(ccx[j%cbuff] - ccx[j%cbuff] + c2)*abs(ccx[j%cbuff] - ccx[j%cbuff] + c2) + abs(ccy[j%cbuff] - ccy[j%cbuff] + r2)*abs(ccy[j%cbuff] - ccy[j%cbuff] + r2) - min1) / (max1 - min1);
						i2 = (dist - min2) / (max2 - min2);
						if ((sqrt(abs(ccx[j%cbuff] + c2 - ccx[x])*abs(ccx[j%cbuff] + c2 - ccx[x]) + abs(ccy[j%cbuff] + r2 - ccy[x])*abs(ccy[j%cbuff] + r2 - ccy[x]))) < (abs(x - j)*h)) i2 = 1.0;
						if (dist < cav * 2 && (j != x - 1 || j != x + 1)) i2 = 1.0;
						e1 = (gradient[(ccy[j%cbuff] + r2)*COLS + (ccx[j%cbuff] + c2)] * gradient[(ccy[j%cbuff] + r2)*COLS + (ccx[j%cbuff] + c2)] - min3) / (max3 - min3);
						i1 *= 0.8;
						i2 *= 1.8;
						e1 *= 0.25;
						if ((r2 == -3) && (c2 == -3)){
							mini = i1 + i2 - e1;
							tempx = ccx[j%cbuff] + c2;
							tempy = ccy[j%cbuff] + r2;
						}
						else {
							if ((i1 + i2 - e1) < mini){
								mini = i1 + i2 - e1;
								tempx = ccx[j%cbuff] + c2;
								tempy = ccy[j%cbuff] + r2;
							}
						}
					}
				ccx[j%cbuff] = tempx;
				ccy[j%cbuff] = tempy;
			}
		}


		/* Draw during iterations */

		PaintImage();
		for (i = 0; i < 120; i++)
		{
			SetPixel(hDC, ccx1[i], ccy1[i], RGB(151, 255, 255));
		}
		for (i = 0; i < 120; i++)
		{
			SetPixel(hDC, baloonx[i], baloony[i], RGB(0, 0, 255));
		}
		for (i = 0; i < cbuff; i++)
		{
			for (r2 = -3; r2 <= 3; r2++)
				for (c2 = -3; c2 <= 3; c2++)
			if (ccx && ccy) SetPixel(hDC, ccx[i] + c2, ccy[i] + r2, RGB(255, 255, 0));
		}
		ct++;
		fgt++;
		if (ct == 100)
		{

			ThreadRunning2 = 0;
		}

		/* Sleep thread 10ms for viewing */
		Sleep(10);
	}
}