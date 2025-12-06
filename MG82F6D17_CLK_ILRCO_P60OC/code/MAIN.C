///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=32KHz, SysCLK=32KHz
//	Description:
//			Set Clock to ILRCO 32KHz, P60 output MCK/4
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


#define LED_G_0		P33
#define LED_R		P34
#define LED_G_1		P35



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
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();
		nop();

		xMs--;
		
	}
}


///***********************************************************************************
//Function:   	void InitClock_ILRCO()
//Description:	Initialize clock for ILRCO
//Input:   
//Output:   		
//*************************************************************************************/
void InitClock_ILRCO(void)
{
	// P60 Output MCK/4
	CLK_P60OC_MCKDiv4();

	// IHRCO Enable, MCK=OSCin, OSCin=ILRCO
	CLK_SetCKCON2(ENABLE_IHRCO|MCK_OSCin|OSCIn_ILRCO);
	nop();
	// IHRCO Disable, MCK=OSCin, OSCin=ILRCO
	CLK_SetCKCON2(DISABLE_IHRCO|MCK_OSCin|OSCIn_ILRCO);

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
	PORT_SetP6PushPull(BIT0);							// Set P60 as Push-Pull for Clock output
}




///***********************************************************************************
//Function:       void InitSystem(void)
//Description:    Initialize MCU
//Input:   
//Output:     
//*************************************************************************************/
void InitSystem(void)
{
	
	InitClock_ILRCO();
	InitPort();

}



void main()
{
	
    InitSystem();
	
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_0=1;LED_G_1=1;LED_R=1;
					
    while(1)
    {
		LED_G_0=0;
		DelayXms(200);
		LED_G_0=1;
		LED_G_1=0;
		DelayXms(200);
		LED_G_1=1;
		LED_R=0;
		DelayXms(200);
		LED_R=1;
    }
}

