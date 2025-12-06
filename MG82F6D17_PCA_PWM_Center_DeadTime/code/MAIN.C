///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=32MHz, SysCLK=32MHz
//	Description:
//			PCA clock: 96MHz
//			CH0(P22)/CH1(P33),CH2(P24)/CH3(P34),CH4(P17)/CH5(P35)
//			6 Channel, 3 pairs of COMP,16bit,Central Aligned,Dead Time
//			PWM Freq: 96MHz/(2048*2)=23.437KHz
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



#define PWM_MIN			(0*2)
#define PWM_MAX			(1024*2)
#define PWM_3_4			(768*2)
#define PWM_2_4			(512*2)
#define PWM_1_4			(256*2)
#define PWM_LOW			(62*2)
#define PWM_HINT		(64*2)

#define PCA_RELOAD		(PWM_MAX)

#define PCA_C           (65536)       	

#define PCA_CL(x)		(u8)((~(u16)x)+1) 
#define PCA_CH(x)     	(u8)(((~(u16)x)+1)>>8)     


idata WordTypeDef wDuty[3];
u8 uDutyTimecnt;


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
}


///***********************************************************************************
//Function:   	void InitPCA_PWM(void)
//Description:	Initialize PCA for PWM
//		PCA clock: 96MHz
//		CH0(P22)/CH1(P33),CH2(P24)/CH3(P34),CH4(P17)/CH5(P35)
//		6 Channel, 3 pairs of COMP,16bit,Central Aligned,Dead Time
//		PWM Freq: 96MHz/(2048*2)=23.437KHz
//Input:   
//Output:     
//*************************************************************************************/
void InitPCA_PWM(void)
{
	PCA_SetCLOCK_CKMIX16();				// PCA clock: 96MHz
	
	PCA_CH0_SetMode_PWM();
	PCA_CH1_SetMode_PWM();
	PCA_CH2_SetMode_PWM();
	PCA_CH3_SetMode_PWM();
	PCA_CH4_SetMode_PWM();
	PCA_CH5_SetMode_PWM();

	PCA_CH0_SetPWM_16Bit();
	PCA_CH1_SetPWM_16Bit();
	PCA_CH2_SetPWM_16Bit();
	PCA_CH3_SetPWM_16Bit();
	PCA_CH4_SetPWM_16Bit();
	PCA_CH5_SetPWM_16Bit();

	PCA_CH01_SetPWM_EnBufferMode();		// CH0,CH1 Enable Buffer mode
	PCA_CH23_SetPWM_EnBufferMode();		// CH2,CH3 Enable Buffer mode
	PCA_CH45_SetPWM_EnBufferMode();		// CH4,CH5 Enable Buffer mode
	PCA_CH01_SetPWM_EnDeadTime();		// CH0,CH1 Enable Dead-Time
	PCA_CH23_SetPWM_EnDeadTime();		// CH2,CH3 Enable Dead-Time
	PCA_CH45_SetPWM_EnDeadTime();		// CH2,CH3 Enable Dead-Time

	PCA_SetPWM_CenterAligned();			// Central Aligned
	PCA_SetCenterAligned_EnVar();		// Variable Resolution
	PCA_SetCenterAligned_CFonBottom();	// CF on bottom of PWM cycle

	PCA_SetDeadTimePreScaler_SysclkDiv8();	// pre-scaler SysClk/8
	PCA_SetDeadTimePeriod(4);				// period 4*(1/PreScaler)=4*(1/(Sysclk/8))=32/Sysclk

	// Set PWM duty
	PCA_CH0_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));
	PCA_CH1_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));
	PCA_CH2_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));
	PCA_CH3_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));
	PCA_CH4_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));
	PCA_CH5_SetValue(PCA_CH(PWM_MIN),PCA_CL(PWM_MIN));

    // Set PWM Freq
	PCA_SetCounter(PCA_C-PCA_RELOAD);
	PCA_SetCounterReload(PCA_C-PCA_RELOAD);

	// Enable PWM output
	PCA_SetPWM0_EnOutput();					
	PCA_SetPWM1_EnOutput();
	PCA_SetPWM2_EnOutput();
	PCA_SetPWM3_EnOutput();
	PCA_SetPWM4_EnOutput();
	PCA_SetPWM5_EnOutput();

	PCA_SetCEX0CEX2CEX4_P22P24P17();	// Set CEX0:P22,CEX2:P24,CEX4:P17

	PCA_CF_DisInterrupt();				// Disable PCA CF interrupt

	
	
	PCA_EnPCACounter();					// Enable PCA counter
		
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
	WordTypeDef duty;
	
    InitSystem();

	wDuty[0].W=PWM_MIN;
	wDuty[1].W=PWM_2_4;
	wDuty[2].W=PWM_MAX;
	while(1)
    {
    	DelayXms(1);
		uDutyTimecnt++;
		if(uDutyTimecnt>=10)
		{
			uDutyTimecnt=0;
			if(wDuty[0].W>=PWM_MAX)
			{
				wDuty[0].W=PWM_MIN;
			}
			else
			{
				wDuty[0].W=wDuty[0].W+16;
			}
			if(wDuty[1].W>=PWM_MAX)
			{
				wDuty[1].W=PWM_MIN;
			}
			else
			{
				wDuty[1].W=wDuty[1].W+16;
			}
			if(wDuty[2].W>=PWM_MAX)
			{
				wDuty[2].W=PWM_MIN;
			}
			else
			{
				wDuty[2].W=wDuty[2].W+16;
			}
			duty.W=PCA_C-wDuty[0].W;
			// Buffer mode,CH0/CH1 reload by CH1
			PCA_CH1_SetValue(duty.B.BHigh,duty.B.BLow);
			duty.W=PCA_C-wDuty[1].W;
			// Buffer mode,CH2/CH3 reload by CH3
			PCA_CH3_SetValue(duty.B.BHigh,duty.B.BLow);
			duty.W=PCA_C-wDuty[2].W;
			// Buffer mode,CH4/CH5 reload by CH5
			PCA_CH5_SetValue(duty.B.BHigh,duty.B.BLow);
		}
    }
}

