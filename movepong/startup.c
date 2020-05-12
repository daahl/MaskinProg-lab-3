/*
 *  startup.c
 *
 */
 #include <stdio.h>
 #include <stdbool.h>
 #include "delay.h"
 #include "draw.h"
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");      /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");                 /* call main */
__asm__ volatile(".L1: B .L1\n");               /* never return */
}

/* ---- GPIO registers ---- */
#define GPIO_D          0x40020C00
#define GPIO_D_IDR      ((volatile unsigned short*) (GPIO_D+0x10))  // The entire IDR
#define GPIO_D_IDR_HIGH ((volatile unsigned char*)  (GPIO_D+0x11))  // The HIGH values un the IDR
#define GPIO_D_ODR      ((volatile unsigned short*) (GPIO_D+0x14))  // The entire ODR
#define GPIO_D_ODR_LOW  ((volatile unsigned char*)  (GPIO_D+0x14))  // The LOW values in the ODR, D0-7
#define GPIO_D_ODR_HIGH ((volatile unsigned char*)  (GPIO_D+0x15))  // The HIGH values in the ODR, D8-15
#define GPIO_D_MODER    ((volatile unsigned int*)   GPIO_D)         // I/O config
#define GPIO_D_OTYPER   ((volatile unsigned char*)  (GPIO_D+5))     // Push-pull/open-drain config
#define GPIO_D_PUPDR    ((volatile unsigned short*) (GPIO_D+0x0E))  // Pull-up/pull-down config
 
/* ---- Global variables ---- */
unsigned char KEYLIST[16] = {1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xE, 0, 0xF, 0xD};                      // Keypad key values
unsigned char HEXLIST[16] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x77,0x7C,0x39,0x5E,0x79,0x71}; // Segment values for 0-F (hex)
 
/* ---- Init ---- */
void appInit(void)
{
    *GPIO_D_MODER       = 0x55005555;   // Pin D15-12 outports, D11-0 inports
    *GPIO_D_OTYPER      = 0x00;         // Push-pull on outpins
    *GPIO_D_PUPDR       = 0x00AA;       // Pull-down on inpins
    *GPIO_D_ODR_HIGH    = 0;            // Reset outpins to 0 at init
}

// Max geometry size
#define MAX_POINTS 25

/* ---- Structs ---- */
typedef struct POINT{
    unsigned char x, y;
}POINT, *PPOINT;

typedef struct{
  int numpoints;
  int sizex;
  int sizey;
  POINT px[ MAX_POINTS ];
} GEOMETRY, *PGEOMETRY;
 
typedef struct tObj{
    PGEOMETRY geo;
    int dirx,diry;
    int posx,posy;
    int monsterType;
    void (* draw ) (struct tObj *);
    void (* clear ) (struct tObj *);
    void (* move ) (struct tObj *);
    void (* set_speed ) (struct tObj *, int, int);
} OBJECT, *POBJECT;
 
/* ---- Helper functions - KeyPad ---- */ 
/*
 *  Activates the given row, r, by editing the ODR.
 */
void kbActivateRow(unsigned char row){
    switch(row){
        case 1: *GPIO_D_ODR_HIGH = 0x10; break;
        case 2: *GPIO_D_ODR_HIGH = 0x20; break;
        case 3: *GPIO_D_ODR_HIGH = 0x40; break;
        case 4: *GPIO_D_ODR_HIGH = 0x80; break;
        default: *GPIO_D_ODR_HIGH = 0;
    }
}
 
/*
 * Returns which column is activated.
 */
unsigned char kbGetCol(void){
    unsigned char col = *GPIO_D_IDR_HIGH;
    if(col & 8) return 4;
    if(col & 4) return 3;
    if(col & 2) return 2;
    if(col & 1) return 1;
    return 0;
}
 
/*
 * Activate each row, one at a time,
 *  read each column,
 *  return the key value.
 */
unsigned char keyB(void){
    unsigned char keyValue = 0xFF; // Default value if no key is pressed
   
        for(unsigned char row = 1; row <= 4; row++){
            kbActivateRow(row);
            unsigned char col = kbGetCol();
           
            if(col){
                kbActivateRow(0); // Turn off all rows once a key has been pressed
                keyValue = KEYLIST[(4 * (row - 1)) + (col - 1)];
            }
        }
       
    return keyValue;
}
 
/* ---- Helper functions - Objects ---- */
// DRAW each pixel. Take the loop size from each objects' geometry numpoints
void draw_ballobject(POBJECT o){
    char numpoints = o->geo->numpoints;
    for (int i = 0; i < numpoints; i++){
        graphic_pixel_set( o->posx+o->geo->px[i].y , o->posy+o->geo->px[i].x);
    }
}

// CLEAR each pixel where the object is. Take the loop size from each objects' geometry numpoints
void clear_ballobject(POBJECT o){
    int numpoints = o->geo->numpoints;
    for (int i = 0; i < numpoints ; i++){
        graphic_pixel_clear( o->posx+o->geo->px[i].y , o->posy+o->geo->px[i].x);
    }
}

void move_ballobject(POBJECT o){
    clear_ballobject( o );
	// Create new local variables assigned with objects' xy positions for later usage
    short new_x = o->posx + o->dirx;
    short new_y = o->posy + o->diry;
    short new_dir_x = o->dirx;
    short new_dir_y = o->diry;
	
	if ( new_x < 1 ){                       // Left boundary
		new_dir_x = ((o->dirx) * -1);
		new_x = 1;
		
	}
	if ( (new_x + o->geo->sizex) > 128 ){   // Right boundary
		new_dir_x = ((o->dirx) * -1);
		new_x = 124;
		
	}
	if( new_y < 1 ){                       // Upper boundary
		new_dir_y = ((o->diry) * -1);
		new_y = 1;
	
	}
	if( (new_y + o->geo->sizey) > 64){     // Lower boundary
		new_dir_y = ((o->diry) * -1);
		new_y = 60;
	}
	
	// Assign the new object positions
    o->posx = new_x;
    o->posy = new_y;
    o->dirx = new_dir_x;
    o->diry = new_dir_y;
	
    draw_ballobject(o);  
}
 
void set_object_speed(POBJECT o, int speedx, int speedy){
    o->dirx = speedx;
    o->diry = speedy;
}
 
int objects_overlap( POBJECT o1, POBJECT o2 ){
	// o1 = victim, o2 = spider
    int collide = 0;
	
	// Loop over each of the spiders' pixels
	for(int i = 0; i <= o2->geo->numpoints; i++){
		// Then loop over the balls' pixels
		for(int n = 0; n<=o1->geo->numpoints; n++){
				if((o1->geo->px[n].y + o1->posy == o2->geo->px[i].y + o2->posy) && (o1->geo->px[n].x + o1->posx == o2->geo->px[i].x + o2->posx))
					collide = 1;
		}		
	}
	
		
	return collide;
}

// See if the spider is too close to the edge
int spider_overlap(POBJECT o){
    if((o->posx) <= 2){
        return 1;
    }
    if((o->posx + o->geo->sizex) >= 125){
        return 1;
    }
    if((o->posy) <= 2){
        return 1;
    }
    if((o->posy + o->geo->sizey) >= 61){
        return 1;
    }
    return 0;
}

GEOMETRY ball_geometry ={
    12,    /* numpoints */
    4,4,   /* sizex,sizey */
    {
    /* px[0,1,2 ...] */
    {0,1},{0,2},{1,0},{1,1},{1,2},{1,3},{2,0},{2,1},{2,2},{2,3},{3,1}, {3,2}
    }
};
 
GEOMETRY spider_geometry = {
    22, /*   numpoints   */
    6,8, /* sizex,sizey */
    {/* px[0,1,2 ...] */
    {0,2},{0,3},
    {1,1},{1,4},
    {2,0},{2,1},{2,2},{2,3},{2,4},{2,5},
    {3,0},{3,2},{3,3},{3,5},
    {4,1},{4,4},
    {5,2},{5,3},
    {6,1},{6,4},
    {7,0},{7,5},
    }
};
 

static OBJECT spider = {
    &spider_geometry,   // geometry
    0,0,                // dirx, diry
    25,25,              // posx, posy
    1,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_object_speed
};

static OBJECT ballobject = {
    &ball_geometry, // geometry
    0,0,                // dirx, diry
    1,1,                // posx, posy
    0,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_object_speed
};

int main(void)
{
	// Init
    char c;
    POBJECT victim = &ballobject;
    POBJECT creature = &spider;
    appInit();
    graphic_initalize();
    graphic_clear_screen();
    victim->set_speed(victim,4,1);
	
	// Main loop
    while(1)
    {
        victim->move( victim );
        creature->move( creature );
        c = keyB();
		
		// Check which key is pressed and change the spiders direction
        switch( c )
        {
            case 6: creature->set_speed( creature, 2, 0); break;
            case 4: creature->set_speed( creature, -2, 0); break;
            case 5: creature->set_speed( creature, 0, 0); break;
            case 2: creature->set_speed( creature, 0, -2); break;
            case 8: creature->set_speed( creature, 0, 2); break;
            default:
            creature->set_speed( creature, 0, 0); break;
        }
		
		
        if(objects_overlap( victim, creature)){
            // Game won
            break;
        }
        if(spider_overlap(creature)){
            // Game over
            break;
        }
		
		// Game speed
        delay_micro(100);
    }
}