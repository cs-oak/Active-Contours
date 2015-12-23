
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;

		// Image data
unsigned char	*rgb_buffer;
unsigned char	*OriginalImage;
int				ROWS,COLS;
int				i, j, k, xt, yt, circe, buff_count, cbuff, x, y, r, r2, c, c2;
int				state;
int template1[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
int template2[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
int				*rbx, *rby;
int				baloonx[120], baloony[120], ccx1[120], ccy1[120];
int				*ccx, *ccy;
int				bufferx[1000], buffery[1000];
unsigned char	*gradient;
int				*gradient1, *gradient2;
float			sum;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning, ThreadRunning1, ThreadRunning2, ThreadRunning3;
int		fgt = 0, alr = 0;
float	cav, cox, coy;
float	cav1, cox1, coy1;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage();
void rb_contour(void *);		/* passes address of window */
void bl_contour(void *);
void custom_contour(void *);
void custom_contour1(void *);