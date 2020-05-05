/* ---- Registers ---- */
#define GPIO_E			0x40021000
#define GPIO_E_IDR		((volatile unsigned short*)	(GPIO_E+0x10))	// The entire IDR
#define GPIO_E_IDR_LOW	((volatile unsigned char*)	(GPIO_E+0x10))	// The LOW values un the IDR
#define GPIO_E_IDR_HIGH	((volatile unsigned char*)	(GPIO_E+0x11))	// The HIGH values un the IDR

#define GPIO_E_ODR		((volatile unsigned short*)	(GPIO_E+0x14))	// The entire ODR
#define GPIO_E_ODR_LOW	((volatile unsigned char*)	(GPIO_E+0x14))	// The LOW values in the ODR, E0-7
#define GPIO_E_ODR_HIGH	((volatile unsigned char*)	(GPIO_E+0x15))	// The HIGH values in the ODR, E8-15

#define GPIO_E_MODER	((volatile unsigned int*)	GPIO_E)			// I/O config
#define GPIO_E_OTYPER	((volatile unsigned char*)	(GPIO_E+5))		// Push-pull/open-drain config
#define GPIO_E_SPEEDR	((volatile unsigned int*)	(GPIO_E+0x08))	// Output speed
#define GPIO_E_PUPDR	((volatile unsigned short*)	(GPIO_E+0x0E))	// Pull-up/pull-down config

/* ---- Display functions ---- */
void app_init(void){
}

__attribute__((naked))  
void graphic_initalize(void) 
{ 
	__asm volatile (" .HWORD  0xDFF0\n");
	__asm volatile (" BX LR\n"); 
}

__attribute__((naked))  
void graphic_clear_screen(void) 
{ 
	__asm volatile (" .HWORD  0xDFF1\n");
	__asm volatile (" BX LR\n"); 
}

__attribute__((naked)) 
void graphic_pixel_set( int x, int y) 
{
	__asm volatile (" .HWORD  0xDFF2\n"); 
	__asm volatile (" BX LR\n"); 
}

__attribute__((naked)) 
void graphic_pixel_clear( int x, int y) 
{ 
	__asm volatile (" .HWORD  0xDFF3\n");
	__asm volatile (" BX LR\n"); 
}

/* ---- Graphic functions ---- */


