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

//Hjälpfunktioner
typedef struct POINT{
	unsigned char x, y;
}POINT, *PPOINT;

typedef struct LINE{
	POINT start, end;
}LINE, *PLINE;

int draw_line(PLINE l){
	int deltax, deltay, error, ystep, y; 
	
	LINE l0 = *l;
	unsigned char x0 = l0.start.x;
	unsigned char y0 = l0.start.y;
	unsigned char x1 = l0.end.x;
	unsigned char y1 = l0.end.y;
	
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
}

/* RÖR INTE */
void swap(unsigned char* a, unsigned char* b){
	unsigned char tempa = *a;
	unsigned char tempb = *b;
	*a = tempb;
	*b = tempa;
	
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


void main(void)
{
    app_init();
	graphic_initalize();
	graphic_clear_screen();
	
	while(1)
		{
			for(int i=0; i < sizeof(lines) / sizeof(LINE); i++)
			{
					draw_line(&lines[i]);
					delay_milli(500);
			}
				graphic_clear_screen();
	}
}

