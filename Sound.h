// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014

int extern set;
int extern m3;
int extern x3;



void Sound_Init(void);
void Sound_Play(const unsigned char *pt, unsigned long count);
void Play(void);
void Sound_pass(void);
void Sound_over(void);
void Sound_passY(void);
void Sound_fail(void);
void Sound_bird(void);
