///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			RTC Clock source: ILRCO 32KHz
//			RTC Clock prescale: 32
//			RTC Reload Value: (64-10)=54
//			RTC Overflow freq: 32K/32/10 ~= 100Hz
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


///***********************************************************************************
//Function:   	void INT_SF(void)
//Description:	SF(system flag)Interrupt handler
//		 		RTC,WDTF,BOD0F,BOD1F
//Input:   
//Output:     
//*************************************************************************************/
void INT_SF(void) interrupt INT_VECTOR_SF
{
	if((PCON1&WDTF)!=0)
	{
		PCON1=WDTF;
	}
	if((PCON1&BOF0)!=0)
	{
		PCON1=BOF0;
	}
	if((PCON1&BOF1)!=0)
	{
		PCON1=BOF1;
	}
	if((PCON1&RTCF)!=0)
	{
		PCON1=RTCF;
		LED_R=!LED_R;
	}
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
//Function:   	void InitRTC(void)
//Description:	Initialize RTC 
//		RTC Clock source: ILRCO 32KHz
//		RTC Clock prescale: 32
//		RTC Reload Value: (64-10)=54
//		RTC Overflow freq: 32K/32/10 ~= 100Hz
//Input:   
//Output:     
//*************************************************************************************/
void InitRTC(void)
{	
	RTC_SetClock_ILRCO();		// RTC Clock source: ILRCO 32KHz
	RTC_SetClock_Div_32();		// RTC Clock prescale: 32
	RTC_SetReload(64-10);		// RTC Reload Value: (64-10)=54
	RTC_SetCounter(64-10);

	RTC_EnRTCO_OutP45();		// Enable RTC output on P45
	
	RTC_ClearRTCF();			// clear RTCF
	RTC_Enable();				// Enable RTC
	
}


///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnSF_RTC();						// Enable RTC interrupt, must enble SF interrupt
	INT_EnSF();							// Enable SF interrupt
}

///***********************************************************************************
//Function:   	void EnP44P45GPIO(void)
//Description:   	set P44/P45 to GPIO, Disable OCD function
//		note: After this setting,P44/P45 will not be used for OCD-ICE simulation.
//Input:   
//Output:     
//*************************************************************************************/
void EnP44P45GPIO(void)
{
	u8 x;
	bit bEA=EA;
	ISPCR=0x80;
	IFADRH=0x00;
	IFADRL=DCON0_P;
	IFMT=ISP_READ_P;
	SCMD=0x46;
	SCMD=0xB9;
	_nop_();
	x=IFD;
	x=x&(~OCDE_P);
	IFD=x;
	IFMT=ISP_WRITE_P;
	SCMD=0x46;
	SCMD=0xB9;
	_nop_();
	ISPCR=0x00;
	IFMT=ISP_STANBY;
	
}

///***********************************************************************************
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{
	EnP44P45GPIO();

	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);		// set P30,P31,P33,P34,P35 as Quasi-Bidirectional
	PORT_SetP4OpenDrainPu(BIT4|BIT5);					// Set P44,P45 as open-drain with pull-high
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
	
	InitRTC();			

	InitInterrupt();		
	
	INT_EnAll();						// Enable global interrupt

}



void main()
{
	
    InitSystem();
	
 	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
 	LED_G_0=1;LED_G_1=1;LED_R=1;

					
    while(1)
    {
    	DelayXms(100);
    	LED_G_0=!LED_G_0;
    }
}

