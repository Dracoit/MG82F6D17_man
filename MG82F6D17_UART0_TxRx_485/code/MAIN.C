///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description£º
//			UART0 485 TX&RX interrupt , P17 control DE/RE
//	Note£º
//
//    Creat time::
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
#define MCU_SYSCLK		12000000
/*************************************************/
///*************************************************
//Set CpuClk (MAX.36MHz)
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

#define IO_485_CTRL		P17

#define Set485Tx()		IO_485_CTRL=1
#define Set485Rx()		IO_485_CTRL=0

#define UART0_RX_BUFF_SIZE   32   		 
#define UART0_TX_BUFF_SIZE   32   		 
xdata u8 RcvBuf[UART0_RX_BUFF_SIZE];
u8 Uart0RxIn =0;


u8 UartOvTime;
bit bUartOvFlag;
u8 LedTime;


///***********************************************************************************
//Function:   	void INT_UART0(void)
//Description:	UART0 Interrupt handler
//Input:   
//Output:     
//*************************************************************************************/
void INT_UART0(void) interrupt INT_VECTOR_UART0
{
	_push_(SFRPI);		   

	SFR_SetPage(0);		   
	if(TI0)					
	{
	   TI0 = 0;	   
	}
	if(RI0)					
	{
		RI0 = 0;				
		if(Uart0RxIn<UART0_RX_BUFF_SIZE)
		{
			RcvBuf[Uart0RxIn] = S0BUF;
			Uart0RxIn++;

			UartOvTime=10;
		}
		
	}
	_pop_(SFRPI);		   
}

///***********************************************************************************
//Function:   	void INT_T0(void)
//Description:	T0 Interrupt handler
//Input:   
//Output:     
//*************************************************************************************/
void INT_T0(void) interrupt INT_VECTOR_T0
{
	TH0=TIMER_12T_1ms_TH;
	TL0=TIMER_12T_1ms_TL;

	if(LedTime!=0) LedTime--;
	if(UartOvTime!=0)
	{
		UartOvTime--;
		if(UartOvTime==0) bUartOvFlag=TRUE;
	}
}

///***********************************************************************************
//Function:		void Uart0SendByte(u8 tByte)
//Description:	Uart0 send byte
//Input:			u8 tByte: the data to be send
//Output:     
//*************************************************************************************/
void Uart0SendByte(u8 tByte)
{
	bit bES;
	bES=ES0;
	S0BUF=tByte;
	while(TI0==0);
	TI0=0;
	ES0=bES;
}



///***********************************************************************************
//Function:		void Uart0SendStr(BYTE* PStr)
//Description:	Uart0 send string
//Input: 			u8* PStr:the string to be send
//Output:     
//*************************************************************************************/
void Uart0SendStr(u8* PStr)
{
	while(*PStr != 0)
	{
		Uart0SendByte(*PStr);
		PStr ++;
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
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);		// set P30,P31,P33,P34,P35 as Quasi-Bidirectional
	PORT_SetP1OpenDrainPu(BIT7);						// set P17 open-drain with pull-high, for 485 control
}


///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnTIMER0();						// Enable Timer0 interrupt
	INT_EnUART0();						// Enable UART0 interrupt
	INT_EnAll();						// Enable global interrupt
	
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
//Function:   	void InitTimer0()
//Description:	Initialize Timer0
//Input:   
//Output:   		
//*************************************************************************************/
void InitTimer0(void)
{
	TM_SetT0Mode_1_16BIT_TIMER();			// TIMER0 Mode: 16-bit
	TM_SetT0Clock_SYSCLKDiv12();			// TIMER0 Clock source: SYSCLK/12
	TM_SetT0Gate_Disable();	
	
	TM_SetT0LowByte(TIMER_12T_1ms_TL);		// Set TL0 value
	TM_SetT0HighByte(TIMER_12T_1ms_TH);		// Set TH0 value

	TM_EnableT0();							// Enable TIMER0
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
	InitTimer0();
	InitUart0_S0BRG();
	InitInterrupt();
}

void main()
{
	u8 i;
	
	InitSystem();

	Uart0RxIn=0;
	UartOvTime=0;
	bUartOvFlag=FALSE;
	
	Set485Rx();
	
	
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_0=1;LED_G_1=1;LED_R=1;	

	Set485Tx();
	DelayXus(10);
	Uart0SendStr("Start!\n");
	DelayXus(10);
	Set485Rx();

	while(1)
	{
		if(LedTime==0)
		{
			LedTime=100;
			LED_G_0 = !LED_G_0;
		}
		if(bUartOvFlag)
		{
			Set485Tx();
			DelayXus(10);
			for(i=0;i<Uart0RxIn;i++)
			{
				Uart0SendByte(RcvBuf[i]);
			}
			Uart0RxIn=0;
			DelayXus(10);
			Set485Rx();
			bUartOvFlag=0;
		}

	}

}


