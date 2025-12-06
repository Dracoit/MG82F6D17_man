///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//		Description:
//			T2 mode: 8bit PWM, output to P10 
//			T3 mode: 8bit PWM, output to P33 
//		
//		Note:
//			Duty cannot 100%
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

#define IO_T2_PWM_OUT	P10
#define IO_T3_PWM_OUT	P33

#define T2_PWM_STEP_MAX	256		// 2~256
#define T3_PWM_STEP_MAX	100		// 2~256

#define T2_PWM_DUTY_0	((256-T2_PWM_STEP_MAX)+0)	
#define T2_PWM_DUTY_10	((256-T2_PWM_STEP_MAX)+(10*T2_PWM_STEP_MAX)/100)	
#define T2_PWM_DUTY_30	((256-T2_PWM_STEP_MAX)+(30*T2_PWM_STEP_MAX)/100)	
#define T2_PWM_DUTY_60	((256-T2_PWM_STEP_MAX)+(60*T2_PWM_STEP_MAX)/100)	
#define T2_PWM_DUTY_80	((256-T2_PWM_STEP_MAX)+(80*T2_PWM_STEP_MAX)/100)	

#define T3_PWM_DUTY_0	((256-T3_PWM_STEP_MAX)+0)	
#define T3_PWM_DUTY_10	((256-T3_PWM_STEP_MAX)+(10*T3_PWM_STEP_MAX)/100)	
#define T3_PWM_DUTY_30	((256-T3_PWM_STEP_MAX)+(30*T3_PWM_STEP_MAX)/100)	
#define T3_PWM_DUTY_60	((256-T3_PWM_STEP_MAX)+(60*T3_PWM_STEP_MAX)/100)	
#define T3_PWM_DUTY_80	((256-T3_PWM_STEP_MAX)+(80*T3_PWM_STEP_MAX)/100)	


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
//Function:   	void InitTimer2_8bit_PWM(void)
//Description:	Initialize Timer2 for 8bit PWM	
//Input:   
//Output:     
//*************************************************************************************/
void InitTimer2(void)
{
	TM_SetT2Mode_8bitPWM();							// T2 mode: 8bit PWM	
	TM_SetT2Clock_SYSCLKDiv12();					// T2 clock source: SYSCLK/12
	
	TM_SetT2LowByte(T2_PWM_DUTY_10);				// T2 low byte	(duty)  			
	TM_SetT2RLLowByte(T2_PWM_DUTY_10);				// T2 low byte reload  (duty) 
	
	TM_SetT2HighByte(256-T2_PWM_STEP_MAX);			// T2 high byte	(freq)  
	TM_SetT2RLHighByte(256-T2_PWM_STEP_MAX);		// T2 high byte	reload (freq)

	TM_EnableT2();									// Enable T2

	//TM_EnableT2CKO_Invert();
	TM_SetT2CKOT2EX_P10P11();						// Set T2CKO output to P11
	TM_DisT2CKO();									// disable T2CKO output
}

///***********************************************************************************
//Function:   	void InitTimer3_8bit_PWM(void)
//Description: 	Initialize Timer3 for 8bit PWM
//Input:   
//Output:     
//*************************************************************************************/
void InitTimer3(void)
{
	TM_SetT3Mode_8bitPWM();							// T3 mode: 8bit PWM
	TM_SetT3Clock_SYSCLKDiv12();					// T3 clock source: SYSCLK/12
	
	
	TM_SetT3LowByte(T3_PWM_DUTY_10);				// T3 low byte	(duty) 			
	TM_SetT3RLLowByte(T3_PWM_DUTY_10);				// T3 low byte reload  (duty)

	TM_SetT3HighByte(256-T3_PWM_STEP_MAX);			// T3 high byte	(freq)  
	TM_SetT3RLHighByte(256-T3_PWM_STEP_MAX);		// T3 high byte	reload (freq)

	TM_EnableT3();									// Enable T3

	//TM_EnableT3CKO_Invert();
	TM_DisT3CKO();									// disable T3CKO output
}


///***********************************************************************************
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{

	IO_T2_PWM_OUT=1;
	IO_T3_PWM_OUT=1;
	
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT4|BIT5);			// set P30,P31,P34,P35 as Quasi-Bidirectional
	PORT_SetP1PushPull(BIT0);						// Set P10 as Push-Pull for T2 PWM ouptput
	PORT_SetP3PushPull(BIT3);						// Set P33 as Push-Pull for T3 PWM ouptput
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
	
	InitPort();
	InitClock();
	
	InitTimer2();			
	InitTimer3();			


}


void main()
{
	
    InitSystem();
	
 	LED_G_1=0;LED_R=0;
	DelayXms(1000);
 	LED_G_1=1;LED_R=1;

					
    while(1)
    {
    	LED_R=0;
    	LED_G_1=0;
    	TM_SetT2LowByte(T2_PWM_DUTY_0);
    	TM_SetT3LowByte(T3_PWM_DUTY_0);
    	TM_SetT2RLLowByte(T2_PWM_DUTY_0);
    	TM_SetT3RLLowByte(T3_PWM_DUTY_0);
    	DelayXms(1);
    	TM_EnT2CKO();
    	TM_EnT3CKO();
    	LED_G_1=1;
    	DelayXms(2);

    	LED_G_1=0;
    	TM_SetT2RLLowByte(T2_PWM_DUTY_10);
    	TM_SetT3RLLowByte(T3_PWM_DUTY_10);
    	LED_G_1=1;
    	DelayXms(2);

    	LED_G_1=0;
    	TM_SetT2RLLowByte(T2_PWM_DUTY_30);
    	TM_SetT3RLLowByte(T3_PWM_DUTY_30);
    	LED_G_1=1;
    	DelayXms(2);

    	LED_G_1=0;
    	TM_SetT2RLLowByte(T2_PWM_DUTY_60);
    	TM_SetT3RLLowByte(T3_PWM_DUTY_60);
    	LED_G_1=1;
    	DelayXms(2);

    	LED_G_1=0;
    	TM_SetT2RLLowByte(T2_PWM_DUTY_80);
    	TM_SetT3RLLowByte(T3_PWM_DUTY_80);
    	LED_G_1=1;
    	DelayXms(2);

    	LED_G_1=0;
    	TM_SetT2RLLowByte(0xFF);
    	TM_SetT3RLLowByte(0xFF);
    	LED_G_1=1;
    	DelayXms(2);

    	// Duty 100% 
    	LED_G_1=0;			
		TM_DisT2CKO();		
		TM_DisT3CKO();
		IO_T2_PWM_OUT=0;
		IO_T3_PWM_OUT=0;
    	LED_G_1=1;
    	DelayXms(2);

		// Disable Output
    	LED_R=1;
    	LED_G_1=0;
		TM_DisT2CKO();
		TM_DisT3CKO();
		IO_T2_PWM_OUT=1;
		IO_T3_PWM_OUT=1;
    	LED_G_1=1;
		DelayXms(10);
    	
    }
}


