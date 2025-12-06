///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=11.0592MHz, SysCLK=11.0592MHz
//	Description:
//			CH0,CH2 Capture buffer mode
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
#define MCU_SYSCLK		11059200
/*************************************************/
///*************************************************
//set CpuClk (MAX.36MHz)
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


WordTypeDef C0Capture,C2Capture;
WordTypeDef C0CaptureA,C2CaptureA;
WordTypeDef C0CaptureB,C2CaptureB;


DWordTypeDef Freq;
bit bBusy;

///***********************************************************************************
//Function:   void INT_PCA(void)
//Description:PCA Interrupt handler
//		 
//Input:   
//Output:     
//*************************************************************************************/
void INT_PCA(void) interrupt INT_VECTOR_PCA
{
	_push_(SFRPI);
	SFRPI=0;
	if(CF)
	{
		CF=0;

	}
	if(CCF0)
	{
		CCF0=0;
		C0CaptureA.B.BLow=CCAP1L;
		C0CaptureA.B.BHigh=CCAP1H;
		C0CaptureB.B.BLow=CCAP0L;
		C0CaptureB.B.BHigh=CCAP0H;
		if(!bBusy)
		{
			if(C0CaptureB.W>=C0CaptureA.W)
			{
				C0Capture.W=C0CaptureB.W-C0CaptureA.W;
			}
			else
			{
				C0Capture.W=(0xFFFF-C0CaptureA.W)+C0CaptureB.W;
			}
		}
		LED_R=!LED_R;
	}
	if(CCF2)
	{
		CCF2=0;
		C2CaptureA.B.BLow=CCAP3L;
		C2CaptureA.B.BHigh=CCAP3H;
		C2CaptureB.B.BLow=CCAP2L;
		C2CaptureB.B.BHigh=CCAP2H;
		if(!bBusy)
		{
			if(C2CaptureB.W>=C2CaptureA.W)
			{
				C2Capture.W=C2CaptureB.W-C2CaptureA.W;
			}
			else
			{
				C2Capture.W=(0xFFFF-C2CaptureA.W)+C2CaptureB.W;
			}
		}
		LED_G_1=!LED_G_1;
	}
	
	_pop_(SFRPI);
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
	PORT_SetP2OpenDrainPu(BIT2|BIT4);					// set P22,P24 as open-drain with pull-high for digital input
}

///***********************************************************************************
//Function:   	void InitPCA_Capture_Buffer(void)
//Description:   	Initialize PCA for capture buffer
//Input:   
//Output:     
//*************************************************************************************/
void InitPCA_Capture_Buffer(void)
{
	PCA_SetCLOCK_SYSCLK();				// PCA clock: SysClk = 12MHz
	
	PCA_CH0_SetMode_16BitCaptureFalling();			// set CH0 for 16bit capture falling edge
	PCA_CH2_SetMode_16BitCaptureFalling();			// set CH2 for 16bit capture falling edge

	PCA_CH01_SetPWM_EnBufferMode();					// enable Buffer mode
	PCA_CH23_SetPWM_EnBufferMode();					// enable Buffer mode

	PCA_SetCounter(0);
	PCA_SetCounterReload(0);

	PCA_SetCEX0CEX2CEX4_P22P24P17();	// CEX0/CEX2/CEX4 pin: P22/P24/P17
	
	PCA_CH0_EnInterrupt();				// Enable CH0 interrupt
	PCA_CH2_EnInterrupt();				// Enable CH2 interrupt

	PCA_EnPCACounter();					// Enable PCA counter 
		
}

///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnPCA();						// Enable PCA interrupt
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
	UART0_SetS0BRGValue(S0BRG_BRGRL_115200_2X_11059200_1T);

	UART0_EnS0BRG();					// Enable S0BRG
}

///***********************************************************************************
//Function:   	void InitClock()
//Description:	Initialize clock
//Input:   
//Output:   		
//*************************************************************************************/
void InitClock(void)
{
#if (MCU_SYSCLK==11059200)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// SysClk=11.0592MHz CpuClk=11.0592MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1);
	
#else
	// SysClk=11.0592MHz CpuClk=5.5296MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1);
#endif
#endif

#if (MCU_SYSCLK==12000000)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// SysClk=12MHz CpuClk=12MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1);
	
#else
	// SysClk=12MHz CpuClk=6MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1);
#endif
#endif

#if (MCU_SYSCLK==22118400)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// SysClk=22.1184MHz CpuClk=22.1184MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx4, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4|OSCIn_IHRCO);
#else
	// SysClk=22.1184MHz CpuClk=11.0592MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx4, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4|OSCIn_IHRCO);
#endif
#endif

#if (MCU_SYSCLK==24000000)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// SysClk=24MHz CpuClk=24MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx4, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4|OSCIn_IHRCO);
#else
	// SysClk=24MHz CpuClk=12MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx4, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4|OSCIn_IHRCO);
#endif
#endif

#if (MCU_SYSCLK==29491200)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// Cpuclk high speed
	CLK_SetCpuCLK_HighSpeed();
	// SysClk=29.491200MHz CpuClk=29.491200MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx5.33, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X533|OSCIn_IHRCO);
#else
	// SysClk=29.491200MHz CpuClk=14.7456MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx5.33, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X533|OSCIn_IHRCO);
#endif
#endif

#if (MCU_SYSCLK==32000000)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// Cpuclk high speed
	CLK_SetCpuCLK_HighSpeed();
	// SysClk=32MHz CpuClk=32MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx5.33, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X533|OSCIn_IHRCO);
#else
	// SysClk=32MHz CpuClk=16MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx5.33, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X533|OSCIn_IHRCO);
#endif
#endif

#if (MCU_SYSCLK==36000000)
#if (MCU_CPUCLK==MCU_SYSCLK)
	// Cpuclk high speed
	CLK_SetCpuCLK_HighSpeed();
	// CKMIx6,x8,x12
	CLK_SetCKM_x6x8x12();	
	// SysClk=36MHz CpuClk=18MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx6, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4_X6|OSCIn_IHRCO);
#else
	// CKMIx6,x8,x12
	CLK_SetCKM_x6x8x12();	
	// SysClk=36MHz CpuClk=18MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx6, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X4_X6|OSCIn_IHRCO);
#endif
#endif


#if (MCU_SYSCLK==44236800)
	// SysClk=44.2368MHz CpuClk=22.1184MHz
	CLK_SetCKCON0(IHRCO_110592MHz|CPUCLK_SYSCLK_DIV_1|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx8, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X8|OSCIn_IHRCO);
#endif

#if (MCU_SYSCLK==48000000)
	// SysClk=48MHz CpuClk=24MHz
	CLK_SetCKCON0(IHRCO_12MHz|CPUCLK_SYSCLK_DIV_2|SYSCLK_MCKDO_DIV_1|ENABLE_CKM|CKM_OSCIN_DIV_2);
	DelayXus(100);
	// IHRCO, MCK=CKMIx8, OSCin=IHRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_CKMI_X8|OSCIn_IHRCO);
#endif

	// P60 Output MCK/4
	//CLK_P60OC_MCKDiv4();
}





///***********************************************************************************
//Function:       void InitSystem(void)
//Description:    Initialize MCU
//Input:   
//Output:     
//*************************************************************************************/
void InitSystem(void)
{
	InitClock();
	InitPort();
	InitPCA_Capture_Buffer();
	InitUart0_S0BRG();
	InitInterrupt();

	INT_EnAll();
}



void main()
{

    InitSystem();
	
	LED_G_0=0;LED_R=0;LED_G_1=0;
    DelayXms(1000);
	LED_G_0=1;LED_R=1;LED_G_1=1;
	
	printf("\nHello!!");

	C0Capture.W=0;
	C2Capture.W=0;

	while(1)
    {
    	DelayXms(1000);
		LED_G_0=!LED_G_0;
		bBusy=1;
		if((C0Capture.W!=0)&&(C2Capture.W!=0))
		{
			Freq.DW = (u32)(MCU_SYSCLK)/C0Capture.W;
			printf("\nC0 Freq:%ldHz",Freq.DW);
			Freq.DW = (u32)(MCU_SYSCLK)/C2Capture.W;
			printf("\nC2 Freq:%ldHz",Freq.DW);
		}
		bBusy=0;

    }
    
}

