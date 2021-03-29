// Name : Muhammad Akmal Bin Mohd Adzuddin
//ID : 25350
// Green FGB LED : 1 Sec Interval
// Red RGB LED : 4 Sec Interval
// At second = 4, both LED is ON, it will show almost yellowish color

#include <stdio.h>
#include <string.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "ADC.h"
#include "LCD.h"

#define  ONESHOT  0   // counting and interrupt when reach TCMPR number, then stop
#define  PERIODIC 1   // counting and interrupt when reach TCMPR number, then counting from 0 again
#define  TOGGLE   2   // keep counting and interrupt when reach TCMPR number, tout toggled (between 0 and 1)
#define  CONTINUOUS 3 // keep counting and interrupt when reach TCMPR number

static uint16_t Timer0Counter=0;
static uint16_t Timer1Counter=0;
volatile uint32_t ledState = 0;
volatile uint8_t gu8AdcIntFlag;

void AdcIntCallback(uint32_t u32UserData)
{
    gu8AdcIntFlag = 1;	
}

void Init_LED() // Initialize GPIO pins
{
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); // GPC12 pin set to output mode
	DrvGPIO_SetBit(E_GPC, 12);            // Goutput Hi to turn off LED
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_SetBit(E_GPC, 13);            // Goutput Hi to turn off LED
	DrvGPIO_Open(E_GPC, 14, E_IO_OUTPUT); // GPC14 pin set to output mode
	DrvGPIO_SetBit(E_GPC, 14);            // Goutput Hi to turn off LED
	DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT); // GPC15 pin set to output mode
	DrvGPIO_SetBit(E_GPC, 15);            // Goutput Hi to turn off LED
}

//---------------------------------------------------------------------------------TIMER
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN = 1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=PERIODIC;		//Select once mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 46875;		// Set TCMPR [0~16777215]
	//Timeout period = (1 / 12MHz) * 255 + 1 * 46875 = 1 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
	char TEXT1[20]="Time Cnt0 :         ";
	Timer0Counter++;
	sprintf(TEXT1+11,"%d",Timer0Counter);
	//print_Line(1, TEXT1);
	
	ledState = ~ ledState;  // changing ON/OFF state
	if(ledState) DrvGPIO_ClrBit(E_GPA,13); // set GPA13 output for GREEN RGB LED
	else	DrvGPIO_SetBit(E_GPA,13);
 	TIMER0->TISR.TIF =1; 	   
}

//---------------------------------------------------------------------------------TIMER
void InitTIMER1(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR1_S = 0;	//Select 12Mhz for Timer1 clock source 
    SYSCLK->APBCLK.TMR1_EN =1;	//Enable Timer1 clock source

	/* Step 2. Select Operation mode */	
	TIMER1->TCSR.MODE=PERIODIC;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER1->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER1->TCMPR = 46875*4;		// Set TCMPR [0~16777215]									
  //Timeout period = (1 / 12MHz) * (255 + 1) * 46875 * 4 = 4 sec

	/* Step 4. Enable interrupt */
	TIMER1->TCSR.IE = 1;
	TIMER1->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR1_IRQn);	//Enable Timer1 Interrupt

	/* Step 5. Enable Timer module */
	TIMER1->TCSR.CRST = 1;		//Reset up counter
	TIMER1->TCSR.CEN = 1;		//Enable Timer1

//  	TIMER1->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR1_IRQHandler(void) // Timer1 interrupt subroutine 
{
	char TEXT2[20]="Time Cnt1 :         ";
	Timer1Counter+=1;
	sprintf(TEXT2+11,"%d",Timer1Counter);
	//print_Line(2, TEXT2);
	
	ledState = ~ ledState;  							 // changing ON/OFF state
	if(ledState) DrvGPIO_ClrBit(E_GPA,14); // set GPA14 output for RED RGB LED
	else   DrvGPIO_SetBit(E_GPA,14);
 	TIMER1->TISR.TIF =1; 	   
}

int32_t main (void)
{
	int i, j, bar_length;
	char TEXT[4][16];
	//uint16_t adc_value[8];
	float adc_value[8];
	
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;//Enable 12MHz Crystal
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	init_LCD(); 
	clear_LCD();
	
	//print_Line(0,"MINI LAB TEST");
	
	DrvGPIO_Open(E_GPA,13, E_IO_OUTPUT); // set GPA13 output for GREEN LED
	InitTIMER0();                        // Set Timer0 Ticking
	
	DrvGPIO_Open(E_GPA,14, E_IO_OUTPUT); // set GPA14 output for RED LED
	InitTIMER1();                        // Set Timer1 Ticking 	
	
  DrvADC_Open(ADC_SINGLE_END, ADC_CONTINUOUS_OP, 0xFF, INTERNAL_HCLK, 1);
  DrvADC_EnableADCInt(AdcIntCallback, 0);

	i = 7;                 // ADC channel = 7 
	j = 6;                 // ADC channel = 6 
	gu8AdcIntFlag =0;      // reset AdcIntFlag
 	DrvADC_StartConvert(); // start ADC sampling
	
	while(1) {
		
		while(gu8AdcIntFlag==0); // wait till AdcIntFlag is set by AdcIntCallback
		gu8AdcIntFlag=0;		     // reset AdcIntFlag
    adc_value[i] = DrvADC_GetConversionData(i);   // read ADC7 value from ADC registers
		adc_value[j] = DrvADC_GetConversionData(j);   // read ADC6 value from ADC registers
		
		bar_length = adc_value[j]/32;
		RectangleFill(0,20,bar_length,29,FG_COLOR,BG_COLOR);
			DrvSYS_Delay(300000);
		clear_LCD();
		
		sprintf(TEXT[3],"Bar_Length:%4d",bar_length); // convert ADC value into text
		print_Line(3, TEXT[3]);		              // output TEXT to LCD display
		
	DrvSYS_Delay(100000);
		if (adc_value[i] > 3072 && adc_value[i] < 4097)
		{
	DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 15); // output Low to turn on LED
		}
	else if (adc_value[i] > 2048 && adc_value[i] < 3073)
		{
	DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
	
	DrvGPIO_SetBit(E_GPC, 15); // output Hi to turn off LED		
		}
	else if (adc_value[i] > 1024 && adc_value[i] < 2049)
		{
	DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	
	DrvGPIO_SetBit(E_GPC, 14); // output Hi to turn off LED			
	DrvGPIO_SetBit(E_GPC, 15); // output Hi to turn off LED		
		}
	else if (adc_value[i] > 0 && adc_value[i] < 1025)
		{
	DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
			
	DrvGPIO_SetBit(E_GPC, 13); // output Hi to turn off LED	
	DrvGPIO_SetBit(E_GPC, 14); // output Hi to turn off LED			
	DrvGPIO_SetBit(E_GPC, 15); // output Hi to turn off LED		
		}
		

		
	}
}