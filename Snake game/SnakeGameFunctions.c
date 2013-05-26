#include <stdio.h>
#include "stdlib.h"
#include "Output.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "systick.h"
#include "rit128x96x4.h"
#include "gpio.h"
#include "inc/lm3s1968.h"
#include "registersDimi.h"
#include "SnakeGameFunctions.h"

#define again 
// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
	//Code Composer Studio Code
	void Delay(unsigned long ulCount){
	__asm (	"    subs    r0, #1\n"
			"    bne     Delay\n"
			"    bx      lr\n");
}

#else		
	//Keil uVision Code
	__asm void
	Delay(unsigned long ulCount)
	{
    subs    r0, #1
    bne     Delay
    bx      lr
	}

#endif


//volatile unsigned long FallingEdges = 0; 

int Xfr = 0;
int Yfr = 0;

int score = 0;

int k = 0;

int SnakeX[200];
int SnakeY[200];

int FruitX[1];
int FruitY[1];

int length = 20;

int type;
//int direction;

extern int printf_lcd( int clear, int xpos, int ypos, const char * format, ...);





void PORTG_Init(void) {	
	volatile unsigned long delay;
	
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOG; //activate port G clock 
	delay = SYSCTL_RCGC2_R; //dummy read - allows few cycles for clock to activate

	GPIO_PORTG_DIR_R &= ~UP; // set PG3 as input
	GPIO_PORTG_DEN_R |= UP; // enable digital I/O on PG3
	GPIO_PORTG_PUR_R |= UP; //Internal pull up resistors enabled
	
	GPIO_PORTG_DIR_R &= ~DOWN; // set PG4 as input
	GPIO_PORTG_DEN_R |= DOWN; // enable digital I/O on PG4
	GPIO_PORTG_PUR_R |= DOWN; //Internal pull up resistors enabled
	
	GPIO_PORTG_DIR_R &= ~LEFT; // set PG5 as input
	GPIO_PORTG_DEN_R |= LEFT; // enable digital I/O on PG5
	GPIO_PORTG_PUR_R |= LEFT; //Internal pull up resistors enabled
	
	GPIO_PORTG_DIR_R &= ~RIGHT; // set PG6 as input
	GPIO_PORTG_DEN_R |= RIGHT; // enable digital I/O on PG6
	GPIO_PORTG_PUR_R |= RIGHT; //Internal pull up resistors enabled
	
	GPIO_PORTG_DIR_R &= ~SELECT; // set PG7 as input
	GPIO_PORTG_DEN_R |= SELECT; // enable digital I/O on PG7
	GPIO_PORTG_PUR_R |= SELECT; //Internal pull up resistors enabled	
}


LED_Init(void){

	GPIO_PORTG_DIR_R |=0x04; // set PG2 as output 
	GPIO_PORTG_DEN_R |=0x04; // enable digital I/O on PG2

}

void SysTick_Init(void){
	
	SysTickPeriodSet(600000);
	IntMasterEnable();
	SysTickIntEnable();
	SysTickEnable();
	
}


LED_ON(void){

	PG2 = 0x04; //bit 2 HIGH --> LED ON

}

LED_OFF(void){

	PG2 = 0x00; //bit 2 LOW --> LED OFF

}

//head image array
static const unsigned char head[] =
{
	0xE0,0x00,0XE0,
	0x0E,0xEE,0X00,
	0xEE,0xEE,0XE0,
	0xE0,0xE0,0XE0,
	0xEE,0xEE,0XE0,
	0xEE,0xEE,0XE0,
	0x0E,0xEE,0X00
};
//body image array
static const unsigned char body[] =
{
	0xEE,0xEE,0XE0,
	0xEE,0xE0,0XE0,
	0xEE,0x0E,0XE0,
	0xEE,0x00,0XE0,
	0xEE,0xE0,0XE0,
	0xEE,0x0E,0XE0,
	0xEE,0xEE,0XE0
};
//fruit image array
static const unsigned char fruit[] =
{
	0xE0,0x00,0XE0,
	0x0E,0x0E,0X00,
	0x00,0xE0,0X00,
	0x0E,0xEE,0X00,
	0xEE,0xEE,0XE0,
	0xEE,0xEE,0XE0,
	0x0E,0xEE,0X00
};
//empty space, used for instant clearing in specific position
static const unsigned char clear[] =
{
	0x00,0x00,0X00,
	0x00,0x00,0X00,
	0x00,0x00,0X00,
	0x00,0x00,0X00,
	0x00,0x00,0X00,
	0x00,0x00,0X00,
	0x00,0x00,0X00
};


//randRangeNum() function reference to http://c-faq.com/lib/randrange.html
unsigned int randRangeNum(unsigned int min, unsigned int max)
{
       double scaled = (double)rand()/RAND_MAX;

       return (max - min +1)*scaled + min;
}

//fruitGeneration() function creates a fruit in a random position on the screen
//from 20 to 110 pixel coordinates for the X coordinate and from 20 to 80 pixel coordinates for Y coordinate
void fruitGeneration(void){

	Xfr = randRangeNum(20,110);	 //randomly created value of fruit's X coordinate from 20 to 110 pixe position
	Yfr = randRangeNum(20,80);	 //randomly created value of fruit's Y coordinate from 20 to 80 pixe position
	FruitX[1] = Xfr;
	FruitY[1] = Yfr;
	RIT128x96x4ImageDraw(fruit,Xfr,Yfr,6,7); //fruit drawing

}

//prints the score on the up left corner of the screen
void printScore(void){

	Output_Color(15);
	printf("*Score: %d", score);	 //score variable value printing
	printf("%c", LF);	//stay in the current position
	Delay(10000000);

}

//checks if the head of the snake's coordinates collides with the fruit's cordinates +-3 pixel positions
void fruitCollisionDetection(void){

	if((SnakeX[0] == Xfr) && (SnakeY[0] == Yfr)||(SnakeX[0] == Xfr+1) && (SnakeY[0] == Yfr+1)||(SnakeX[0] == Xfr-1) && (SnakeY[0] == Yfr-1)||(SnakeX[0] == Xfr+2) && (SnakeY[0] == Yfr+2)||(SnakeX[0] == Xfr-2) && (SnakeY[0] == Yfr-2)||(SnakeX[0] == Xfr+3) && (SnakeY[0] == Yfr+3)||(SnakeX[0] == Xfr-3) && (SnakeY[0] == Yfr-3)){ 

			length++;	//increments by 1 the length variable that runs in the for loop that increments the snake body 
			score += 1; //getting score every time that a collision is detected
			printScore();  // calling of the printScore function
			fruitGeneration(); //recreates a new fruit randomly on screen
	}	
}



//Game Over functions 
void GameOver(void){

	 RIT128x96x4Clear();  //Clears the screen for the display of the Game Over message
	 // Printing of the game Over related message to the screen
	 RIT128x96x4StringDraw("-~-~-~-~-~-~-~-",20,20,15);
	 RIT128x96x4StringDraw("|             |",20,28,15);
	 RIT128x96x4StringDraw("~ -Game Over- ~",20,36,15);
	 RIT128x96x4StringDraw("|             |",20,44,15);
	 RIT128x96x4StringDraw("~ Press RESET ~",20,52,15);
	 RIT128x96x4StringDraw("|             |",20,60,15);
	 RIT128x96x4StringDraw("-~-~-~-~-~-~-~-",20,68,15);
	 Delay(100000000);
	 RIT128x96x4Clear(); // Clears the screen to reset the new game
	 
}
//-----------------------------

//Body Collision checking functions
void bodyCollisionDetection(void){

	for(k = length; k > 0; k--){

		if ((k > 21) && (SnakeX[0] == SnakeX[k]) && (SnakeY[0] == SnakeY[k]))		
			
			GameOver();	  //Game over printing function calling

	}

}
//--------------------------------------------







