// TableTrafficLight.c solution to EE319K Lab 5, spring 2021
// Runs on TM4C123
// Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 17, 2021

/* 

 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3-1 (built-in white LED)
// "don't walk" light connected to PF1 (built-in red LED)
#include <stdint.h>
#include "SysTick.h"
#include "TExaS.h"
#include "../inc/tm4c123gh6pm.h"



void DisableInterrupts(void);
void EnableInterrupts(void);

#define PB543210                (*((volatile uint32_t *)0x400050FC)) // bits 5-0
#define PE210                   (*((volatile uint32_t *)0x4002401C)) // bits 2-0
#define PF321                   (*((volatile uint32_t *)0x40025038)) // bits 3-1

struct State{
	uint32_t outC; // output for cars 
	uint32_t outP; // output for peds 
	uint32_t wait; 
	uint32_t next[8];
};

typedef const struct State State_t;

#define Start 0   
#define West_G 1 		
#define West_Y 2
#define West_R 3 
#define South_G 4
#define South_Y 5
#define Walk_W 6
#define Walk_R1 7
#define Walk_O1 8
#define Walk_R2 9
#define Walk_O2 10
#define Walk_R3 11
#define Walk_O3 12
#define Walk_RF 13


State_t FSM[14] = {
// outC  outP delay   000     001     010     011      100     101    110     111 
	{0x24, 0x02, 100, {Start, West_G, South_G, West_G, Walk_W, Walk_W, Walk_W, Walk_W}}, // Start 
	{0x0C, 0x02, 100, {West_Y, West_G, West_Y, West_Y, West_Y, West_Y, West_Y, West_Y}}, // West_G 
	{0x14, 0x02, 100, {West_R, West_R, West_R, West_R, West_R, West_R, West_R, West_R}}, // West_Y
	{0x24, 0x02, 100, {Start, West_G, South_G, South_G, Start, Start, South_G, South_G}}, // West_R
	{0x21, 0x02, 100, {South_Y, South_Y, South_G, South_Y, South_Y, South_Y, South_Y, South_Y}}, // South_G
	{0x22, 0x02, 100, {Start, Start, Start, Start, Start, Start, Start, Start}}, // South_Y 
	{0x24, 0x0E, 100, {Walk_R1, Walk_R1, Walk_R1, Walk_R1, Walk_W, Walk_R1, Walk_R1, Walk_R1}}, // Walk_W
	{0x24, 0x02, 100, {Walk_O1, Walk_O1, Walk_O1, Walk_O1, Walk_O1, Walk_O1, Walk_O1, Walk_O1}}, // Walk_R1
	{0x24, 0x00, 100, {Walk_R2, Walk_R2, Walk_R2, Walk_R2, Walk_R2, Walk_R2, Walk_R2, Walk_R2}}, // Walk_O1
	{0x24, 0x02, 100, {Walk_O2, Walk_O2, Walk_O2, Walk_O2, Walk_O2, Walk_O2, Walk_O2, Walk_O2}}, // Walk_R2
	{0x24, 0x00, 100, {Walk_R3, Walk_R3, Walk_R3, Walk_R3, Walk_R3, Walk_R3, Walk_R3, Walk_R3}}, // Walk_O2
	{0x24, 0x02, 100, {Walk_O3, Walk_O3, Walk_O3, Walk_O3, Walk_O3, Walk_O3, Walk_O3, Walk_O3}}, // Walk_R3
	{0x24, 0x00, 100, {Walk_RF, Walk_RF, Walk_RF, Walk_RF, Walk_RF, Walk_RF, Walk_RF, Walk_RF}}, // Walk_O3
	{0x24, 0x02, 100, {Start, West_G, South_G, West_G, Walk_W, West_G, South_G, West_G}}	// Walk_RF
};
	


void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|GPIO_PORTB_DATA_R;
}

// run this version in the simulator
int main(void){volatile uint32_t delay;
  DisableInterrupts();
  //TExaS_Init(&LogicAnalyzerTask);
  PLL_Init(); // PLL on at 80 MHz
  SYSCTL_RCGC2_R |= 0x32; // Ports B,E,F
  delay = SYSCTL_RCGC2_R;
	
	uint32_t S;
	uint32_t Input;
	GPIO_PORTB_DIR_R |= 0x003F; //output
	GPIO_PORTE_DIR_R &= 0xF8; //input
	GPIO_PORTF_DIR_R |= 0x0E; //output
	GPIO_PORTB_DEN_R |= 0x003F;
	GPIO_PORTE_DEN_R |= 0x07;
	GPIO_PORTF_DEN_R |= 0x0E;

// run this version on the board

	//int main(void){volatile uint32_t delay;
  //DisableInterrupts();
  //TExaS_Init(&LogicAnalyzerTask);
  //PLL_Init(); // PLL on at 80 MHz
  //SYSCTL_RCGC2_R |= 0x32; // Ports B,E,F
  //delay = SYSCTL_RCGC2_R;

// **************************************************	
//**old version	
	//int main(void){ volatile uint32_t delay;
  //DisableInterrupts();
  //TExaS_Init(&LogicAnalyzerTask);
  // PLL_Init();     // PLL on at 80 MHz
  //SysTick_Init();   // Initialize SysTick for software waits
// **************************************************
// weird old bug in the traffic simulator
// run next two lines on real board to turn on F E B clocks
//  SYSCTL_RCGCGPIO_R |= 0x32;  // real clock register 
//  while((SYSCTL_PRGPIO_R&0x32)!=0x32){};
// run next two lines on simulator to turn on F E B clocks
 // SYSCTL_RCGC2_R |= 0x32;  // LM3S legacy clock register
 // delay = SYSCTL_RCGC2_R;
// **************************************************
 
  EnableInterrupts();
	
	
  S =  Start;
  while(1){
		GPIO_PORTB_DATA_R = FSM[S].outC; // output
		GPIO_PORTF_DATA_R = FSM[S].outP;
		SysTick_Wait10ms(FSM[S].wait); // wait
		Input = GPIO_PORTE_DATA_R; // input
		S = FSM[S].next[Input]; // next	
  }
}



