///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//			Description:
//				UART1 TX&RX interrupt
//			Note:
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
#define LED_R			P34
#define LED_G_1		P35


#define SFR_Page_(x)		SFRPI = x;


#define UART1_RX_BUFF_SIZE   32   		
#define UART1_TX_BUFF_SIZE   32   		
xdata u8 RcvBuf[UART1_RX_BUFF_SIZE];
u8 Uart1RxIn =0;
u8 Uart1RxOut =0;
xdata u8 TxBuf[UART1_TX_BUFF_SIZE];
u8 Uart1TxIn =0;
u8 Uart1TxOut =0;
bit bUart1TxFlag;

u8 LedTime;


///***********************************************************************************
//Function:   	void INT_UART1(void)
//Description:	UART1 Interrupt handler
//		 
//Input:   
//Output:     
//*************************************************************************************/
void INT_UART1(void) interrupt INT_VECTOR_UART1
{
	_push_(SFRPI);		   

	SFR_Page_(1);		  
	if(TI1)					
	{
	   TI1 = 0;	   
		if(Uart1TxIn==Uart1TxOut)
		{
			bUart1TxFlag=FALSE;
		}
		else
		{
			S1BUF=TxBuf[Uart1TxOut];
			bUart1TxFlag=TRUE;
			Uart1TxOut++;
			if(Uart1TxOut>=UART1_TX_BUFF_SIZE)
			{
				Uart1TxOut=0;
			}
		}
	}
	if(RI1)					
	{
		RI1 = 0;				
		RcvBuf[Uart1RxIn] = S1BUF;
		Uart1RxIn++;
		if(Uart1RxIn >=UART1_RX_BUFF_SIZE)
		{
			Uart1RxIn =0;
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
}

///***********************************************************************************
//Function:		void Uart1SendByte(u8 tByte)
//Description:	Uart1 send byte
//Input:			u8 tByte: the data to be send
//Output:     
//*************************************************************************************/
void Uart1SendByte(u8 tByte)
{
	u8 i;
	
	if(bUart1TxFlag==FALSE)
	{
		Uart1TxOut=0;
		Uart1TxIn=1;
		TxBuf[0]=tByte;
		SFR_Page_(1);
        TI1=1;
        SFR_Page_(0);
	}
	else
	{
		i=Uart1TxIn;
		TxBuf[i]=tByte;
		i++;
		if(i>=UART1_TX_BUFF_SIZE)
		{
			i=0;
		}
		while(i==Uart1TxOut)
		{
		}
		INT_DisUART1();
		Uart1TxIn=i;
		INT_EnUART1();
	}
}



///***********************************************************************************
//Function:		void Uart1SendStr(BYTE* PStr)
//Description:	Uart1 send string
//Input: 			u8* PStr:the string to be send
//Output:     
//*************************************************************************************/
void Uart1SendStr(u8* PStr)
{
	while(*PStr != 0)
	{
		Uart1SendByte(*PStr);
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
	INT_EnUART1();						// Enable UART1 interrupt
	INT_EnAll();						// Enable global interrupt

}	


///***********************************************************************************
//Function:   	void InitUart1(void)
//Description:   	Initialize Uart1 (B.R. 9600)¡¢RX:P34 TX:P35
//Input:   
//Output:     
//*************************************************************************************/
void InitUart1(void)
{
	UART1_SetMode8bitUARTVar();								// UART1 Mode: 8-bit, Variable B.R

	UART1_EnS1BRG();										// Enable S1BRG
	UART1_SetBaudRateX2();									// S1BRG x2
	UART1_SetRxTxP34P35();									// UART1 Pin£ºRX:P34 TX:P35
	UART1_EnReception();									// Enable reception
	UART1_SetS1BRGSelSYSCLK();								// S1BRG clock source£ºSYSCLK

	// Sets B.R. value
	UART1_SetS1BRGValue(S1BRG_BRGRL_9600_2X_12000000_1T);	
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
	InitUart1();
	InitInterrupt();
}

void main()
{
	InitSystem();

	
	Uart1RxIn=0;
	Uart1RxOut=0;
	Uart1TxIn=0;
	Uart1TxOut=0;
	bUart1TxFlag=0;
	
	LED_G_0=0;
	DelayXms(1000);
	LED_G_0=1;
	DelayXms(500);
	
	Uart1SendStr("Start!\n");


	while(1)
	{
		if(LedTime==0)
		{
			LedTime=100;
		     LED_G_0 = !LED_G_0;
		}
		if(Uart1RxIn != Uart1RxOut)
		{
			Uart1SendByte(RcvBuf[Uart1RxOut]);
			Uart1RxOut++;
			if(Uart1RxOut>=UART1_RX_BUFF_SIZE)
			{
				Uart1RxOut=0;
			}
		}

	}

}


