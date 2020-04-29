/*
 * 	startup.c
 *
 */
 #include <stdio.h>
 #include <stdbool.h>
 #include "delay.h"
 #include "draw.h"
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

/* ---- Structioner ---- */
typedef struct POINT{
	unsigned char x, y;
}POINT, *PPOINT;

typedef struct LINE{
	POINT start, end;
}LINE, *PLINE;

typedef struct RECT{
	POINT ulcorner;
	unsigned char width, height;
}RECT, *PRECT;

typedef struct polygonpoint{
	char x, y;
	struct polygonpoint *next;
}POLY, *PPOLY;

#define MAX_POINTS 25

typedef struct tGEOMETRY{
	int numpoints;
	int sizex;
	int sizey;
	POINT px[MAX_POINTS];
}GEOMETRY, *PGEOMETRY;

typedef struct tObj{
	GEOMETRY geo;
    int dirx,diry;
    int posx,posy;
	void (* draw ) (struct tObj *);
    void (* clear ) (struct tObj *);
    void (* move ) (struct tObj *);
    void (* set_speed ) (struct tObj *, int, int);
}OBJECT, *POBJECT;

/* ---- Hjälpfunktioner ---- */
/* vvv RÖR INTE vvv */
void swap(unsigned char* a, unsigned char* b){
	unsigned char tempa = *a;
	unsigned char tempb = *b;
	*a = tempb;
	*b = tempa;
	
} 

int draw_line(PLINE l){
	int deltax, deltay, error, ystep, y; 
	
	LINE l0 = *l;
	unsigned char x0 = l0.start.x;
	unsigned char y0 = l0.start.y;
	unsigned char x1 = l0.end.x;
	unsigned char y1 = l0.end.y;
	
	// Only allow pixels within the display
	if(x0 < 1 || x0 > 128 || x1 < 1 || x1 > 128 || y0 < 1 || y0 > 64 || y1 < 1 || y1 > 64){
		return 0;
	}
	
	// Bresenham's algorithm
	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep){
		swap(&x0,&y0);
		swap(&x1,&y1);
	}
	if (x0 > x1){
		swap(&x0,&x1);
		swap(&y0,&y1);
	} 
	
	deltax = x1-x0;
	deltay = abs(y1 - y0);
	error = 0;
	y = y0;
	
	if (y0 < y1){
		ystep = 1;
	}else{
		ystep = -1;
	}
	
	for(int x = x0; x <= x1; x++){
		if(steep){
			graphic_pixel_set(y, x);
		}else{
			graphic_pixel_set(x, y);
		}
		
		error = error+deltay;
		
		if(2 * error >= deltax){
			y = y + ystep;
			error = error - deltax;
		}
	}
	
	return 1;
}

void draw_rectangle(PRECT r){
	RECT r0 = *r;
	unsigned char xs = r0.ulcorner.x;
	unsigned char ys = r0.ulcorner.y;
	unsigned char width = r0.width;
	unsigned char height = r0.height;
	
	unsigned char x0, y0, x1, y1;
	
	LINE rlines[] = {
		{xs, ys, xs + width, ys}, // Line 1
		{xs, ys, xs, ys + height}, // Line 2
		{xs + width, ys, xs + width, ys + height}, // Line 3
		{xs + width, ys + height, xs, ys + height}, // Line 4
		};
		
	for(int i=0; i < sizeof(rlines) / sizeof(LINE); i++)
	{
		draw_line(&rlines[i]);
	}
}

void draw_polygon(PPOLY p){
	POLY p0 = *p;
	unsigned char x0 = p0.x;
	unsigned char y0 = p0.y;
	POLY pnext = *p0.next;
	
	while(1){
		unsigned char x1 = pnext.x;
		unsigned char y1 = pnext.y;
		LINE l = {x0, y0, x1, y1};
		draw_line(&l);
		x0 = pnext.x;
		y0 = pnext.y;
		
		if(pnext.next == 0){
			break;
		}
		pnext = *pnext.next;
	}
}

void draw_ballobject(POBJECT o){
	OBJECT o0 = *o;
	
	// Draw each pixel in the GEOMETRY px list
	for(int i = 0; i < o0.geo.numpoints; i++)
		graphic_pixel_set(o0.geo.px[i].x, o0.geo.px[i].y);
}

void clear_ballobject(POBJECT o){
	OBJECT o0 = *o;
	
	// Draw each pixel in the GEOMETRY px list
	for(int i = 0; i < o0.geo.numpoints; i++)
		graphic_pixel_clear(o0.geo.px[i].x, o0.geo.px[i].y);
}

void move_ballobject(POBJECT o){
	
}

void set_object_speed(POBJECT o, int speedx, int speedy){
	
}
LINE lines[]={
	{40,10,100,10},
	{40,10,100,20},
	{40,10,100,30},
	{40,10,100,40},
	{40,10,100,50},
	{40,10,100,60},
	{40,10,90,60},
	{40,10,80,60},
	{40,10,70,60},
	{40,10,60,60},
	{40,10,50,60},
	{40,10,40,60}};

RECT rectangles[]={
	{10,10,20,10},
	{25,25,10,20},
	{40,30,70,20},
	{60,35,10,10},
	{70,10,5,5}};

void main(void)
{
    app_init();
	graphic_initalize();
	graphic_clear_screen();
	
	GEOMETRY ball_geometry = {
		12, /*   numpoints   */
		4,4, /* sizex,sizey */
		{/* px[0,1,2 ...] */
		{1,2},{1,3},{2,1},{2,2},{2,3},{2,4},{3,1},{3,2},{3,3},{3,4},{4,2},{4,3}
		}
	};
	
	while(1){
		draw_ballobject(&ball_geometry);
		delay_milli(500);
		clear_ballobject(&ball_geometry);
		delay_milli(500);
	}
	
}

