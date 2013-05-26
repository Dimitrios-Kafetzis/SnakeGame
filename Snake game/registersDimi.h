/*
* 	 dimiRegisters.h
*	 Dimitrios Kafetzis - el09dk
*/

//-----------------REGISTERS--Interrupts_App-----------------------//
#define SYSCTL_RCGC2_R (*((volatile unsigned long *) 0x400FE108))
#define GPIO_PORTG_DIR_R (*((volatile unsigned long *) 0x40026400))
#define	GPIO_PORTG_DEN_R (*((volatile unsigned long *) 0x4002651C))
#define GPIO_PORTG_DATA_R (*((volatile unsigned long *) 0x400263FC))
#define GPIO_PORTG_PUR_R (*((volatile unsigned long *) 0x40026510))

#define SYSCTL_RCGC2_GPIOG 0x00000040	
#define PG2 (*((volatile unsigned long *) 0x40026010))	  //LED 
#define PG3 (*((volatile unsigned long *) 0x40026020))	  //UP
#define PG4 (*((volatile unsigned long *) 0x40026040))	  //DOWN
#define PG5 (*((volatile unsigned long *) 0x40026080))	  //LEFT
#define PG6 (*((volatile unsigned long *) 0x40026100))	  //RIGHT
#define PG7 (*((volatile unsigned long *) 0x40026200))	  //SELECT 



#define UP    GPIO_PIN_3
#define DOWN  GPIO_PIN_4
#define LEFT  GPIO_PIN_5
#define RIGHT GPIO_PIN_6
#define SELECT GPIO_PIN_7 

//PLL1 related registers
#define SYSCTL_RCC_R (*((volatile unsigned long *)0x400FE060))
#define	SYSCTL_RCC_BYPASS 0x00000800 //PLL Bypass
#define SYSCTL_RCC_USESYSDIV 0x00400000	//Enable System Clock Divider

#define SYSCTL_RCC_XTAL_M 0x000003C0 //Crystal Value
#define SYSCTL_RCC_XTAL_8MHZ    0x00000380  // 8 MHz



#define SYSCTL_RCC_OSCSRC_M 0x00000030 // oscillator source
#define SYSCTL_RCC_OSCSRC_MAIN 0x00000000 //MOSC

#define SYSCTL_RCC_PWRDN 0x00002000 //PLL Power Down

#define SYSCTL_RCC_SYSDIV_M 0x07800000 //System Clock Divisor

#define SYSCTL_RCC_SYSDIV_15    0x07000000  // System clock /15

#define SYSCTL_RIS_R (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RIS_PLLLRIS 0x00000040 //PLL Lock Raw Interrupt Status



//SYSTick related registers
#define NVIC_ST_CTRL_R (*((volatile unsigned long *)0xE000E010)) 
#define NVIC_ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R (*((volatile unsigned long *)0xE000E018))


