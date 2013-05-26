/*
*	Snake Game 
*	Dimitrios K
*/

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


volatile unsigned long FallingEdges = 0;  // this variable stores the number of the falling edges

int Xfr = 0;	//X coordinate of the fruit
int Yfr = 0;	//Ycoordinate of the fruit

int score = 0;	//this variable stores the score of the game

int k = 0;	 
int i = 0;

int s;

int SnakeX[200];   //X coordinates array of the snake
int SnakeY[200];   //Y coordinates array of the snake

int length = 20;   //the initial length of the snake

int times = 0;	 //this variable stores the times which are pressed the SELECT button

int type;
int direction;


extern int printf_lcd( int clear, int xpos, int ypos, const char * format, ...);



void EdgeCounter_Init (void){
	
	GPIOIntTypeSet(GPIO_PORTG_BASE, (UP|DOWN|LEFT|RIGHT|SELECT), GPIO_FALLING_EDGE); //setting the type of the interrupt, to trigger when there is a falling edge
	GPIOPinIntClear(GPIO_PORTG_BASE,(UP|DOWN|LEFT|RIGHT|SELECT));  //clearing of the interrupt flags on the pins of the on-board switches
	GPIOPinIntEnable(GPIO_PORTG_BASE, (UP|DOWN|LEFT|RIGHT|SELECT));  //arming of the interrupts on the pins of the switches 
	IntPrioritySet(INT_GPIOG,0x05); //setting of the priority for the GPIO POrt G to be equal to 5 	
	IntEnable(INT_GPIOG); //enabling of the above interrupts	
	IntMasterEnable(); //enabling of the master interrupt, allows the processor to respond to interrupts
	FallingEdges = 0; //initialisation of the variable which stores the number of the falling edges
} 

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
	
	SysTickPeriodSet(600000); //600000 ticks, period of 75ms
	IntMasterEnable(); //allowing the processor to resond to the interrupts
	SysTickIntEnable();	//enables the interrupts
	SysTickEnable(); //enables the SysTick counter
	
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

//the following function creates the motion of the snake, the direction parameter is given by the GPIO Interrupt Handler
void motion(int direction){
	//the following for loop makes the allocation of the X and Y coordinates for the body of the snake
	for(k = length; k > 0; k--){
		SnakeX[k] = SnakeX[(k - 1)];
		SnakeY[k] = SnakeY[(k - 1)];
	}

	switch (direction){
	
		case 1:
			SnakeY[0] -= s; //decrement coordinate Y of the head (UP case)
		break;
		
		case 2:
			SnakeY[0] += s; //increment coordinate Y of the head (DOWN case)
		break;
		
		case 3:
			SnakeX[0] -= s; //decrement coordinate X of the head (LEFT case)
		break;
		
		case 4:
			SnakeX[0] += s; //increment coordinate X of the head (RIGHT case)
		break;

	}

}

//the following function drawSnake() draws the Snake
void drawSnake(void){
	motion(direction);
	for(k = 0; k < length; k++){
	
		if(k == 0){
			
			RIT128x96x4ImageDraw(head,SnakeX[k],SnakeY[k],6,7);	// Snake's Head draw, the coordinates are allocated to the zero position fo the related arrays
			
		}
		else{
			
			RIT128x96x4ImageDraw(body,SnakeX[k],SnakeY[k],6,7);	// Snake's Body draw, the coordinates are allocated to the 1 and over position fo the related arrays
			RIT128x96x4ImageDraw(clear,SnakeX[k+1],SnakeY[k+1],6,7); //erase the previous position of the body																							   
		}
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

// the levelChoice makes the selection of the level of the difficulty of the game in the initial screen
void levelChoice(void){

	times += 1;	 //any time that the select button is pressed this variable is increasing

	Output_Color(15);
	printf("*SLCT: %d", times);	 //times that the SELECT button pressed printing
	printf("%c", LF);	//stay in the current position

	if(times == 1){	   //if the SELECT button is pressed 1 time

		s = 1; //EASY mode, the 's' variable controls the speed of the snake and its initial length		

	 }
	 else if(times == 2){  //if the SELECT button is pressed 2 times

		s = 2; //MEDIUM mode, the 's' variable controls the speed of the snake and its initial length

	 }
	 else if(times == 3){  //if the SELECT button is pressed 3 times

		s = 3; //HARD mode, the 's' variable controls the speed of the snake and its initial length

	 }
	 else if(times > 3){
	 
		times = 1; //if the SELECT button is pressed more than 3 times then it is reset to the beginning			 
	 }

}


//GPIO Interrupt Handler, it is checking frequently the state of the GPIO switch connected inputs 
void GPIOPortG_Handler(void){
	volatile unsigned long rawStatus = GPIOPinIntStatus(GPIO_PORTG_BASE,false);
	GPIOPinIntClear(GPIO_PORTG_BASE,(UP|DOWN|LEFT|RIGHT|SELECT));
	FallingEdges=FallingEdges+1;	

		if(rawStatus & UP){
			direction = 1;	   //UP switch pressed sends direction command equal to 1
			
		}
		else if(rawStatus & DOWN){
			direction = 2;	   //DOWN switch pressed sends direction command equal to 2
					
		}
		else if(rawStatus & LEFT){
			direction = 3;	   //LEFT switch pressed sends direction command equal to 3
			
		}
		else if(rawStatus & RIGHT){
			direction = 4;	   //RIGHT switch pressed sends direction command equal to 4
			
		}
		else if(rawStatus & SELECT){
		
			levelChoice();	 //calling of the levelChoice function	when the SELECT button is pressed

		}
			
}

// This functions controlls the initial screen of the Game.
void snakeGameInit(void){

	 RIT128x96x4Clear();  //Clears the screen for the display of the Game Over message
	 // Printing of the game Over related message to the screen
	 RIT128x96x4StringDraw("~   Level:     ~",10,20,15);
	 RIT128x96x4StringDraw("| EASY-SLCT 1  |",10,28,15);
	 RIT128x96x4StringDraw("~              ~",10,36,15);
	 RIT128x96x4StringDraw("| MEDIUM-SLCT 2|",10,44,15);
	 RIT128x96x4StringDraw("~              ~",10,52,15);
	 RIT128x96x4StringDraw("| HARD-SLCT 3  |",10,60,15);
	 Delay(30000000);
	 RIT128x96x4Clear(); // Clears the screen to reset the new game
	 
}


void SysTick_Handler(void){

		fruitCollisionDetection();	// fruit collision checking 		
		bodyCollisionDetection();	// body collision checking
		drawSnake(); //drawing of the snake 

}	

int main(void) {
	
	PORTG_Init();
	EdgeCounter_Init();
	SysTick_Init();
	RIT128x96x4Init(2000000);
	Output_Init();
	LED_Init();
	
	snakeGameInit();  //initial screen
	RIT128x96x4Clear();	 //screen clearing again, I mentioned that with just the last clearing
	                     //from the previous function calling the screen wasn't totally cleared
	fruitGeneration(); // initial fruit drawing
	

		while(1){

		}		  
}


