
 void Delay(unsigned long ulCount);
  __asm void Delay(unsigned long ulCount);


extern int printf_lcd( int clear, int xpos, int ypos, const char * format, ...);

void EdgeCounter_Init (void);
extern void PORTG_Init(void);
extern LED_Init(void);
extern void SysTick_Init(void);

extern LED_ON(void);
extern LED_OFF(void);

extern static const unsigned char head[];
extern static const unsigned char body[];
extern static const unsigned char fruit[];
extern static const unsigned char clear[];

extern unsigned int randRangeNum(unsigned int min, unsigned int max);

extern void fruitGeneration(void);
extern void printScore(void);
extern void fruitCollisionDetection(void);
extern void motion(int direction);
extern void drawSnake(void);
extern void GameOver(void);
extern void bodyCollisionDetection(void);
