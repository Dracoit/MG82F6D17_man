///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			Through the IVREF 1.4v, calculate the P10 input voltage
//	Note:
//
//    Creat time::
//    Modify::
//    
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
//Function:		void IAP_WritePPage(u8 PsfrAddr,u8 PsfrData)
//Description:	write P page sfr
//Input:   
//		 		u8 PsfrAddr: sfr Address,u8 PsfrData: sfr data
//Output:     
//*************************************************************************************/
void IAP_WritePPage(u8 PsfrAddr,u8 PsfrData)
{
	bit bEA=EA;
	EA = 0; 					//
	IFADRH = 0; 				// IFADRH must be 0
	IFADRL= PsfrAddr;			// sfr Address
	IFD= PsfrData;				// sfr data
	IFMT = ISP_WRITE_P;			// write P page sfr
	ISPCR = 0x80;				// Enable ISP/IAP
	SCMD = 0x46;
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA;					// restore Interrupt
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
//Function:   	void InitADC(void)
//Description:   	Initialize ADC
//Input:   
//Output:     
//*************************************************************************************/
void InitADC(void)
{
	ADC_Enable();						// Enable ADC
	ADC_SetClock_SYSCLK();				// ADC Clock = SYSCLK       sps= 12M/30=600K
	ADC_SetMode_SetADCS();				// ADC Trigger mode: set ADCS
	ADC_SetADCData_12Bit();				// ADC Data resolution: 12bit			
	ADC_SetRightJustified();			// ADC Right-Justified

	IAP_WritePPage(PCON3_P,IVREN_P);	// Enable IVR 1.4V
}

///***********************************************************************************
//Function:   	u16 GetAdcData()
//Description:	read ADC result of the current channel  
//Input:   
//Output:   		u16 :ADC Value
//*************************************************************************************/
u16 GetAdcValue()
{
	WordTypeDef wAdcValue;
	ADCON0 = ADCON0|ADCS;							// set ADCS,Start ADC
    while((ADCON0&ADCI)==0);           				// wait ADC complete
    wAdcValue.B.BHigh=ADCDH;
    wAdcValue.B.BLow=ADCDL;
 	ADCON0 = ADCON0&(~ADCI);		           		// clear ADCI flag
 	return wAdcValue.W&0x0FFF;
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
	InitUart0_S0BRG();
	InitADC();
	
}


void main()
{
	u16 wADCValue;
	u32 dwTemp0,dwTemp1;
	
    InitSystem();
	
	LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_1=1;LED_R=1;

    while(1)
    {
		DelayXms(500);
		LED_R = !LED_R;

		
		ADC_SetChannel_IVR14();		// Set the ADC channel to IVR 1.4v
		dwTemp0=GetAdcValue();		// read ADC value
		
		ADC_SetChannel_AIN0();		// Set the ADC channel to AIN0
		dwTemp1=GetAdcValue();		// read ADC value
		// calculate the P10(AIN0) input voltage
		wADCValue=(dwTemp1*1400L)/dwTemp0;
		// Output the result by UART
		printf("\nP10 V:%d",wADCValue);
		 
		

    }
}


