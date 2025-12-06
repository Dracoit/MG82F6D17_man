///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			BOD0/BOD1 test;
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

bit bBOD1Flag;

///***********************************************************************************
//Function:   void INT_SF(void)
//Description:SF Interrupt handler
//Input:   
//Output:     
//*************************************************************************************/
void INT_SF(void) interrupt INT_VECTOR_SF
{
	if((PCON1 & BOF0)!=0)
	{ // BOD0 interrupt, 1.7V
		PCON1=BOF0;
	}

	if((PCON1 & BOF1)!=0)
	{ // BOD1 ÖÐ¶Ï, 4.2V/3.7V/2.4V/2.0V
		PCON1=BOF1;
		bBOD1Flag = 1;
		LED_R=0;
		// To do......

		INT_DisSF_BOD1();			// Disable BOD1 interrupt
	}

	if((PCON1 & WDTF)!=0)
	{ // WDT ÖÐ¶Ï
		PCON1=WDTF;
	}

	if((PCON1 & RTCF)!=0)
	{ // RTC ÖÐ¶Ï
		PCON1=RTCF;
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
//Function:   	u8 IAP_ReadPPage(u8 PsfrAddr)
//Description:	read P page sfr
//Input:   
//		 		u8 PsfrAddr: sfr Address
//Output:     
//		 		u8: sfr data
//*************************************************************************************/
u8 IAP_ReadPPage(u8 PsfrAddr)
{
	bit bEA=EA;
	EA = 0; 					
	IFADRH = 0; 				// IFADRH must be 0
	IFADRL= PsfrAddr;			// sfr Address
	IFMT = ISP_READ_P;			// read P page sfr
	ISPCR = 0x80;				// Enable ISP/IAP
	SCMD = 0x46;
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA;					
	return IFD;					// return sfr data
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
	EA = bEA;					
}

///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnSF_BOD1();					// Enable BOD1 interrupt
	INT_EnSF();							// Enable SF interrupt
}	

///***********************************************************************************
//Function:   	void InitBOD()
//Description:	Initialize BOD
//Input:   
//Output:   		
//*************************************************************************************/
void InitBOD(void)
{
	u8 x;
	x=BIT0;

	//x=x|POW_BOD1_Set42V;				// BOD1 Detecting 4.2V
	x=x|POW_BOD1_Set37V;				// BOD1 Detecting 3.7V
	//x=x|POW_BOD1_Set24V;				// BOD1 Detecting 2.4V
	//x=x|POW_BOD1_Set20V;				// BOD1 Detecting 2.0V

	//First, set the detection voltage to avoid false operations.
	IAP_WritePPage(PCON2_P, x);	
	PCON1=BOF0|BOF1;

	x=x|POW_BOD1_Enable;				// Enable BOD1
	//x=x|POW_BOD1_Disable;				// Disable BOD1

	
	//x=x|POW_BOD1_Enable_OnPD;			// Enable BOD1 on PD mode
	x=x|POW_BOD1_Disable_OnPD;			// Disable BOD1 on PD mode

	//x=x|POW_BOD1_EnReset;				// Enable BOD1 to trigger a MCU reset
	x=x|POW_BOD1_DisReset;			// Disable BOD1 to trigger a MCU reset

	x=x|POW_BOD0_EnReset;				// Enable BOD0(1.7V) to trigger a MCU reset
	//x=x|POW_BOD0_DisReset;			// Disable BOD0(1.7V) to trigger a MCU reset
	
	IAP_WritePPage(PCON2_P, x);	

	
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
	InitBOD();
	InitInterrupt();

	INT_EnAll();						// Enable global interrupt
}



void main()
{
    InitSystem();

	bBOD1Flag=0;
	
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_0=1;LED_G_1=1;LED_R=1;

    while(1)
    {
		DelayXms(200);
		LED_G_0=!LED_G_0;
		if(bBOD1Flag)
		{
			if((PCON1 & BOF1)!=0)
			{// BOF1 set£¬VDD<3.7V
				PCON1 = PCON1 | BOF1;		// clear flag
			}
			else
			{// VDD>3.7V
				//Power voltage restored. Enable BOD1 interruption
				LED_R=1;
				bBOD1Flag = FALSE;
				INT_EnSF_BOD1();			
			}
		}
    }
}

