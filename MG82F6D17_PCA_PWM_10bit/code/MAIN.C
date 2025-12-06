///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=32MHz, SysCLK=32MHz
//	Description£º
//			PCACLK: 96MHz
//			CH0(P22)/CH1(P33),CH2(P24)/CH3(P34),CH4(P17)/CH5(P35),CH6(P60)/CH7(P61)
//			Buffer mode,Edge,10bit
//			PWM Freq: 96MHz/(1024)=93.750KHz
//	Note£º
//
//    Creat time:
//    Modify:
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
#define MCU_SYSCLK		32000000
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


idata WordTypeDef wDuty[4];

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
	PORT_SetP1PushPull(BIT7);						// set P17(CEX4) as push-pull for PWM output
	PORT_SetP2PushPull(BIT2|BIT4);					// set P22(CEX0),P24(CEX2) as push-pull for PWM output
	PORT_SetP3PushPull(BIT3|BIT4|BIT5);				// set P33(CEX1),P34(CEX3),P35(CEX5) as push-pull for PWM output
	PORT_SetP6PushPull(BIT0|BIT1);					// set P60(PWM6),P61(PWM7) as push-pull for PWM output
}


///***********************************************************************************
//Function:   	void InitPCA_PWM(void)
//Description:	Initialize PCA for PWM
//Input:   
//Output:   		
//*************************************************************************************/
void InitPCA_PWM(void)
{
	PCA_SetCLOCK_CKMIX16();			// 96MHz
	
	PCA_CH0_SetMode_PWM();
	PCA_CH1_SetMode_PWM();
	PCA_CH2_SetMode_PWM();
	PCA_CH3_SetMode_PWM();
	PCA_CH4_SetMode_PWM();
	PCA_CH5_SetMode_PWM();
	PCA_CH6_SetMode_PWM();
	PCA_CH7_SetMode_PWM();

	PCA_CH0_SetPWM_10Bit();
	PCA_CH1_SetPWM_10Bit();
	PCA_CH2_SetPWM_10Bit();
	PCA_CH3_SetPWM_10Bit();
	PCA_CH4_SetPWM_10Bit();
	PCA_CH5_SetPWM_10Bit();
	PCA_CH6_SetPWM_10Bit();
	PCA_CH7_SetPWM_10Bit();

	PCA_CH01_SetPWM_EnBufferMode();		// CH0,CH1 Enable Buffer mode
	PCA_CH23_SetPWM_EnBufferMode();		// CH2,CH3 Enable Buffer mode
	PCA_CH45_SetPWM_EnBufferMode();		// CH4,CH5 Enable Buffer mode
	PCA_CH67_SetPWM_EnBufferMode();		// CH6,CH7 Enable Buffer mode

	PCA_SetPWM_EdgeAligned();			// Edge

	// Set PWM duty
	PCA_CH0_SetValue_10Bit(0,0);
	PCA_CH1_SetValue_10Bit(0,0);
	PCA_CH2_SetValue_10Bit(0,0);
	PCA_CH3_SetValue_10Bit(0,0);
	PCA_CH4_SetValue_10Bit(0,0);
	PCA_CH5_SetValue_10Bit(0,0);
	PCA_CH6_SetValue_10Bit(0,0);
	PCA_CH7_SetValue_10Bit(0,0);

	// Set Counter
	PCA_SetCounter(0);
	PCA_SetCounterReload(0);

	// Disable PWM output
	PCA_SetPWM0_DisOutput();					
	PCA_SetPWM1_DisOutput();
	PCA_SetPWM2_DisOutput();
	PCA_SetPWM3_DisOutput();
	PCA_SetPWM4_DisOutput();
	PCA_SetPWM5_DisOutput();
	PCA_SetPWM6_DisOutput();
	PCA_SetPWM7_DisOutput();

	PCA_SetCEX0CEX2CEX4_P22P24P17();	// Set CEX0:P22,CEX2:P24,CEX4:P17
	PCA_SetPWM6PWM7_P60P61();			// Set PWM6:P60,PWM7:P61

	PCA_CF_DisInterrupt();				// Disable CF Interrupt

	PCA_EnPCACounter();					// Eanble PCA counter
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
	InitPCA_PWM();
}


void main()
{
	u8 inx;
	
    InitSystem();

	LED_G_0=0;LED_R=0;LED_G_1=0;
	DelayXms(1000);
	LED_G_0=1;LED_R=1;LED_G_1=1;

	// Enable PWM OutPut
	PCA_SetPWM0_EnOutput();					
	PCA_SetPWM1_EnOutput();
	PCA_SetPWM2_EnOutput();					
	PCA_SetPWM3_EnOutput();
	PCA_SetPWM4_EnOutput();					
	PCA_SetPWM5_EnOutput();
	PCA_SetPWM6_EnOutput();
	PCA_SetPWM7_EnOutput();
	inx=0;
	while(1)
    {
    	DelayXms(200);
    	inx++;
    	if(inx>=8)
    	{
			inx=0;
    	}
    	switch(inx)
    	{
			case 0:
				// Duty 100% All High	
				wDuty[0].W=0x0000;
				wDuty[1].W=0x0000;
				wDuty[2].W=0x0000;
				wDuty[3].W=0x0000;
				break;

			case 1:
				// Duty 75% 	
				wDuty[0].W=0x0100;
				wDuty[1].W=0x0100;
				wDuty[2].W=0x0100;
				wDuty[3].W=0x0100;
				break;

			case 2:
				// Duty 50% 	
				wDuty[0].W=0x0200;
				wDuty[1].W=0x0200;
				wDuty[2].W=0x0200;
				wDuty[3].W=0x0200;
				break;

			case 3:
				// Duty 25% 	
				wDuty[0].W=0x0300;
				wDuty[1].W=0x0300;
				wDuty[2].W=0x0300;
				wDuty[3].W=0x0300;
				break;
				
			case 4:
				// Duty 0% 	 All low
				wDuty[0].W=0x0400;
				wDuty[1].W=0x0400;
				wDuty[2].W=0x0400;
				wDuty[3].W=0x0400;
				break;

			case 5:
				// Duty 25% 	
				wDuty[0].W=0x0300;
				wDuty[1].W=0x0300;
				wDuty[2].W=0x0300;
				wDuty[3].W=0x0300;
				break;

			case 6:
				// Duty 50% 	
				wDuty[0].W=0x0200;
				wDuty[1].W=0x0200;
				wDuty[2].W=0x0200;
				wDuty[3].W=0x0200;
				break;

			case 7:
				// Duty 75% 	
				wDuty[0].W=0x0100;
				wDuty[1].W=0x0100;
				wDuty[2].W=0x0100;
				wDuty[3].W=0x0100;
				break;
				
    	}
		// Buffer mode,CH0/CH1 reload by CH1
		PCA_CH1_SetValue_10Bit(wDuty[0].B.BHigh,wDuty[0].B.BLow);
		// Buffer mode,CH2/CH3 reload by CH3
		PCA_CH3_SetValue_10Bit(wDuty[1].B.BHigh,wDuty[1].B.BLow);
		// Buffer mode,CH4/CH5 reload by CH5
		PCA_CH5_SetValue_10Bit(wDuty[2].B.BHigh,wDuty[2].B.BLow);
		// Buffer mode,CH6/CH7 reload by CH7
		PCA_CH7_SetValue_10Bit(wDuty[3].B.BHigh,wDuty[3].B.BLow);
    }
}
