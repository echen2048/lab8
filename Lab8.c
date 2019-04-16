// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 11/6/2018 

// Specifications:
// Measure distance using slide pot, sample at 60 Hz
// maximum distance can be any value from 1.5 to 2cm
// minimum distance is 0 cm
// Calculate distance in fixed point, 0.001cm
// Analog Input connected to PD2=ADC5
// displays distance on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats (use them in creative ways)
// 

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "../inc/tm4c123gh6pm.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on ST7735
// main3 adds your convert function, position data is no ST7735

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
float weight;
float bias;
int32_t adcval[7];
int32_t dist[7];
int32_t ymean =1000;
int32_t yvar = 400000;
// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x20;
	delay = 100;
	delay++;
	GPIO_PORTF_DIR_R |= 0x0E;

	GPIO_PORTF_PUR_R |= 0x10;
	GPIO_PORTF_DIR_R &= ~0x10;
	GPIO_PORTF_DEN_R |= 0x10; 
	GPIO_PORTF_DEN_R |= 0x0E;

}

void SysTick_Init(void){
	// write this
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 0x145855;
	NVIC_ST_CURRENT_R = 0;
	
	NVIC_ST_CTRL_R = 0x7;
}

uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
uint32_t ADCmail;
int8_t ADCstatus;
/*
int main(void){       // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 5
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 5
  }
}
int main2(void){
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 5
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  while(1){           // use scope to measure execution time for ADC_In and LCD_OutDec           
    PF2 = 0x04;       // Profile ADC
    Data = ADC_In();  // sample 12-bit channel 5
    PF2 = 0x00;       // end of ADC Profile
    ST7735_SetCursor(0,0);
    PF1 = 0x02;       // Profile LCD
    LCD_OutDec(Data); 
    ST7735_OutString("    ");  // spaces cover up characters from last output
    PF1 = 0;          // end of LCD Profile
  }
}
*/
// your function to convert ADC sample to distance (0.001cm)
uint32_t Convert(uint32_t input){
	return input*weight+bias;
	
}
void SysTick_Handler(void){
	GPIO_PORTF_DATA_R ^=0x02;
	GPIO_PORTF_DATA_R ^=0x02;
	ADCstatus = 1;
	ADCmail = ADC_In();
}

void IO_Touch(void) {
	uint8_t state;
	while(1){
		state = GPIO_PORTF_DATA_R;
		state &= 0x10;
		if(GPIO_PORTF_DATA_R == 0){
			Delay1ms(20);
			state = GPIO_PORTF_DATA_R;
			state &= 0x10;
			if(GPIO_PORTF_DATA_R==0x10){
				return;
			}
		}
	}
 // --UUU-- wait for release; delay for 20ms; and then wait for press
}  
float mean(void){
	int32_t sum=0;
	for(int i=0;i<7;i++){
		sum+=adcval[i];
	}
	return sum/7;
}
float variance(int32_t mean){
	int32_t var=0;
	for(int i=0;i<7;i++){
		var+=(adcval[i]-mean)*(adcval[i]-mean);
	}
	return var;
}
float covar(int32_t mean){
	int32_t covar=0;
	for(int i=0;i<7;i++){
		covar+=(adcval[i]-mean)*(dist[i]-ymean);
	}
	return covar;
}

void AutoCal(void){
	dist[0]=0;
	dist[1]=400;
	dist[2]=800;
	dist[3]=1000;
	dist[4]=1200;
	dist[5]=1600;
	dist[6]=2000;
	
	adcval[0]=5;
	adcval[1]=475;
	adcval[2]=1552;
	adcval[3]=1955;
	adcval[4]=2585;
	adcval[5]=3621;
	adcval[6]=4095;
	
	
	ST7735_OutString("Start AutoCal.\nSet slider to 0\nthen press SW1");
	IO_Touch();
	adcval[0]=ADC_In();
	
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 4mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[1]=ADC_In();
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 8mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[2]=ADC_In();
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 10mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[3]=ADC_In();
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 12mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[4]=ADC_In();
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 16mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[5]=ADC_In();
	
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("Set slider to 20mm.\nPress SW1 to confirm");
	IO_Touch();
	adcval[6]=ADC_In();
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0); 
	
	float mn = mean();
	float var = variance(mn);
	float cv = covar(mn);
	float internal_w = cv/var;
	weight = cv/var;
	bias = ymean-weight*mn;
	ST7735_OutString("Calibration complete\nSW1 to continue");
	
	IO_Touch();
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0); 
	return;
}
/*
int main3(void){ 
  TExaS_Init();         // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  ADC_Init();         // turn on ADC, set channel to 5
  while(1){  
    PF2 ^= 0x04;      // Heartbeat
    Data = ADC_In();  // sample 12-bit channel 5
    PF3 = 0x08;       // Profile Convert
    Position = Convert(Data); 
    PF3 = 0;          // end of Convert Profile
    PF1 = 0x02;       // Profile LCD
    ST7735_SetCursor(0,0);
    LCD_OutDec(Data); ST7735_OutString("    "); 
    ST7735_SetCursor(6,0);
    LCD_OutFix(Position);
    PF1 = 0;          // end of LCD Profile
  }
}   */
int main(void){
  PortF_Init();
	TExaS_Init();
  // your Lab 8
  EnableInterrupts();
	SysTick_Init();
	ADC_Init();
	ST7735_InitR(INITR_REDTAB); 
	AutoCal();
  ADCstatus = -1;
	while(1){
		while(ADCstatus<0){};
		LCD_OutFix(Convert(ADCmail));
		ST7735_SetCursor(0,0);
		ADCstatus = -1;
  }
}


