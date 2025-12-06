///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			ADC window voltage detection and output the ADC value to the uart
//			If the ADC value is within the window threshold, ADC value will be output via the uart.
//			ADC pin: P10, Uart port: P30, P31
//	Note:
//
//    Creat time::
//    Modify::
//*********************************************************************/
#define _MAIN_C

#include <Intrins.h>
#include <Absacc.h>

#include <Stdio.h>  // for printf

#include ".\include\REG_MG82F6D17.H"
#include ".\include\Type.h"
#include ".\include\API_Macro_MG82F6D17.H"
#include ".\include\API_Uart_BRGRL_MG82F6D17.H"


///*************************************************
//Set SysClk (MAX.50MHz) (MAX.50MHz)
//Selection: 
//	11059200,12000000,
//	22118400,24000000,
//	29491200,32000000,
//	44236800,48000000
//*************************************************/
#define MCU_SYSCLK		12000000
/*************************************************/
///*************************************************
//Set  CpuClk (MAX.36MHz)
//	1) CpuCLK=SysCLK
//	2) CpuClk=SysClk/2
//*************************************************/
#define MCU_CPUCLK		(MCU_SYSCLK)
//#define MCU_CPUCLK		(MCU_SYSCLK/2)

#define TIMER_1T_1ms_TH	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(1000000)))) /256) 			
#define TIMER_1T_1ms_TL	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(1000000)))) %256)

#define TIMER_12T_1ms_TH	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(12000000)))) /256) 			
#define TIMER_12T_1ms_TL	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(12000000)))) %256)

#define LED_G_0		P33
#define LED_R		P34
#define LED_G_1		P35


bit bES0;
bit bADCFinish =0;
WordTypeDef wAdcValue;

#define TEST_ADC_BUF_SIZE	16
xdata u16 wAdcBuf[TEST_ADC_BUF_SIZE];
u8 AdcBufCnt;

///***********************************************************************************
//Function:   void INT_ADC(void)
//Description:ADC Interrupt handler
//		 
//Input:   
//Output:     
//*************************************************************************************/
void INT_ADC(void) interrupt INT_VECTOR_ADC
{
	
	if((ADCON0 & ADCWI) != 0)	// ADC Window Comparison Data match has occurred.
	{
		ADCON0 &= ~ADCWI;		// Clear ADC Window interrupt flag
		LED_R = ~LED_R;
		wAdcValue.B.BHigh = ADCDH;
   	 	wAdcValue.B.BLow = ADCDL;
		wAdcValue.W &= 0xFFF;
		wAdcBuf[AdcBufCnt]=wAdcValue.W;
		AdcBufCnt++;
		if(AdcBufCnt>=TEST_ADC_BUF_SIZE)
		{
			bADCFinish=TRUE;
			INT_DisADC();		// Buf overflow,diable ADC interrupt
		}
		
	}
	
}

///*************************************************
//Function: 		char putchar (char c)   
//Description: 	send for printf
//Input:    		char c
//Output:     
//*************************************************/
char putchar (char c)   
{      
	bit bES;
	bES=ES0;
    ES0=0;        
    S0BUF = c;        
    while(TI0==0);        
    TI0=0;        
    ES0=bES;        
    return 0;
}


///*************************************************
//Function:     	void DelayXus(u16 xUs)
//Description:   	dealy, unit:us
//Input:     		u8 Us -> *1us  (1~255)
//Output:     
//*************************************************/
void DelayXus(u8 xUs)
{
	while(xUs!=0)
	{
#if (MCU_CPUCLK>=11059200)
		_nop_();
#endif
#if (MCU_CPUCLK>=14745600)
		_nop_();
		_nop_();
		_nop_();
		_nop_();
#endif
#if (MCU_CPUCLK>=16000000)
		_nop_();
#endif

#if (MCU_CPUCLK>=22118400)
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
#endif
#if (MCU_CPUCLK>=24000000)
		_nop_();
		_nop_();
#endif		
#if (MCU_CPUCLK>=29491200)
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
#endif
#if (MCU_CPUCLK>=32000000)
		_nop_();
		_nop_();
#endif

		xUs--;
	}
}

///*************************************************
//Function:     	void DelayXms(u16 xMs)
//Description:    dealy, unit:ms
//Input:     		u16 xMs -> *1ms  (1~65535)
//Output:     
//*************************************************/
void DelayXms(u16 xMs)
{
	while(xMs!=0)
	{
		CLRWDT();
		DelayXus(200);
		DelayXus(200);
		DelayXus(200);
		DelayXus(200);
		DelayXus(200);
		xMs--;
		
	}
}

///***********************************************************************************
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);		// set P30,P31,P33,P34,P35 as Quasi-Bidirectional
	PORT_SetP1AInputOnly(BIT0);							// Set P10(AIN0) as Analog-Input-Only for ADC input
}


///***********************************************************************************
//Function:   void InitADC(void)
//Description:   Initialize ADC
//Input:   
//Output:     
//*************************************************************************************/
void InitADC(void)
{
	ADC_Enable();						// Enable ADC
	ADC_SetClock_SYSCLK();				// ADC Clock = SYSCLK       sps= 12M/30=600K
	ADC_SetMode_FreeRunning();			// ADC Trigger mode: Freerunning
	ADC_SetADCData_12Bit();				// ADC Data resolution: 12bit			
	ADC_SetRightJustified();			// ADC Right-Justified

	ADC_SetWindow_WithIn();				// ADCWI be set when ADC value is within the window range
	ADC_SetWindow_High(0x700);			// ADC Window High Boundary value
	ADC_SetWindow_Low(0x200);			// ADC Window Low Boundary value

	ADC_DisInterrupt_ADCI();			// Disable ADCI interrupt
	ADC_EnInterrupt_ADCWI();			// Enable ADCWI interrupt
	ADC_DisInterrupt_SMPF();			// Disable SMPF interrupt



	
}

///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnADC();						//  Enable ADC interrupt

	INT_EnAll();						//  Enable global interrupt
	
}	


///***********************************************************************************
//Function:   	void InitUart0_S0BRG()
//Description:	Initialize Uart0, The source of baud rate was S0BRG
//Input:   
//Output:   		
//*************************************************************************************/
void InitUart0_S0BRG(void)
{
	UART0_SetAccess_S0CR1();			// Enable access S0CR1 
	UART0_SetMode8bitUARTVar();			// UART0 Mode: 8-bit, Variable B.R.
	UART0_EnReception();				// Enable reception
	UART0_SetBRGFromS0BRG();			// B.R. source: S0BRG
	UART0_SetS0BRGBaudRateX2();			// S0BRG x2
	UART0_SetS0BRGSelSYSCLK();			// S0BRG clock source: SYSCLK

	// Sets B.R. value
	UART0_SetS0BRGValue(S0BRG_BRGRL_9600_2X_12000000_1T);

	UART0_EnS0BRG();					// Enable S0BRG
}

///***********************************************************************************
//Function:       void InitSystem(void)
//Description:    Initialize MCU
//Input:   
//Output:     
//*************************************************************************************/
void InitSystem(void)
{
	InitPort();
	InitADC();
	InitUart0_S0BRG();
	InitInterrupt();
}

void main()
{
	u8 i;
	
	InitSystem();
	AdcBufCnt=0;
	bADCFinish=FALSE;

	printf("\nStart!");

	LED_G_0=0;LED_R=0;LED_G_1=0;
	DelayXms(1000);
	LED_G_0=1;LED_R=1;LED_G_1=1;

  	while(1)
  	{
		DelayXms(100);
		LED_G_0 = !LED_G_0;

		if(bADCFinish)
		{
			LED_R=1;
			printf("\nADC Win:");
			for(i=0;i<TEST_ADC_BUF_SIZE;i++)
			{
				printf("%04X ",wAdcBuf[i]);
			}
			bADCFinish=FALSE;
			INT_EnADC();			// send data completed,restart ADC interrupt
		}
			
    }
}


