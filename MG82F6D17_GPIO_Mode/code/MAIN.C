///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			IO Mode 
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
	PORT_SetP1AInputOnly(BIT0|BIT1);				// Set P10,P11 as Analog-Input-Only 
	PORT_SetP1OpenDrainPu(BIT5);						// Set P15 as open-drain with pull-high 
	PORT_SetP1OpenDrain(BIT6);					// Set P16 as open-drain
	PORT_SetP1PushPull(BIT7);						// Set P17 as Push-Pull

	//PORT_SetP2AInputOnly(BIT2|BIT4);				// Set P22,P24 as Analog-Input-Only 
	//PORT_SetP2OpenDrainPu(BIT2|BIT4);				// Set P22,P24 as open-drain with pull-high
	//PORT_SetP2OpenDrain(BIT2|BIT4);				// Set P22,P24 as open-drain
	PORT_SetP2PushPull(BIT2|BIT4);					// Set P22,P24 as Push-Pull

	//PORT_SetP3InputOnly(BIT3);					// Set P33 as Input-Only 
	//PORT_SetP3OpenDrain(BIT3|BIT4|BIT5);			// Set P33,P34,P35 as open-drain
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);	// Set P30,P31,P33,P34,P35 as Quasi-Bidirectional
	//PORT_SetP3PushPull(BIT5);						// Set P35 as Push-Pull

	//PORT_SetP4AInputOnly(BIT4|BIT5);				// Set P44,P45 as Analog-Input-Only 
	//PORT_SetP4OpenDrainPu(BIT4|BIT5);				// Set P44,P45 as open-drain with pull-high
	//PORT_SetP4OpenDrain(BIT4|BIT5);				// Set P44,P45 as open-drain
	//PORT_SetP4PushPull(BIT4|BIT5);				// Set P44,P45 as Push-Pull

	//PORT_SetP6AInputOnly(BIT0|BIT1);				// Set P60,P61 as Analog-Input-Only 
	//PORT_SetP6OpenDrainPu(BIT0|BIT1);				// Set P60,P61 as open-drain with pull-high
	//PORT_SetP6OpenDrain(BIT0|BIT1);				// Set P60,P61 as open-drain
	PORT_SetP6PushPull(BIT0|BIT1);				// Set P60,P61 as Push-Pull
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

}



void main()
{
    InitSystem();
	
	LED_G_0=0;LED_R=0;LED_G_1=0;
    DelayXms(1000);
	LED_G_0=1;LED_R=1;LED_G_1=1;
					
    while(1)
    {
			LED_G_0=0;
		DelayXms(500);
		LED_G_0=1;
		LED_R=0;
		DelayXms(500);
		LED_R=1;
		LED_G_1=0;
		DelayXms(500);
			LED_G_1=1;

		// P6, Only Page 1
		SFR_SetPage(1);
		P60=0;
		P61=0;
			DelayXus(100);
		P60=1;
		P61=1;
			DelayXus(100);
		SFR_SetPage(0);

		}
    }


