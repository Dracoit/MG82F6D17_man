///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//			Description:
//				UART1£¬Mode4 (SPI Master)
//				SPICLK <-->TXD	P11
//				MOSI	<-->RXD  P10
//				MISO	<-->S0MI P61
//
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
#define SPI_NSS		P33

sfr UARTSPICLK		= 0xBB;

// Clock polarity(CPOL)
#define SPICLK_LOW	0	// LOW
#define SPICLK_HIGH	1	// High
#define SPI_CPOL  SPICLK_LOW	

// Data order
#define LSB	0	// LSB
#define MSB	1	// MSB
#define SPI_DORD	MSB

// Clock source
#define SPI_CLK_SYSCLK_4	0	// SYSCLK/4
#define SPI_CLK_SYSCLK_12	1	// SYSCLK/12
#define SPI_CLOCK	SPI_CLK_SYSCLK_4

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
	PORT_SetP1PushPull(BIT0|BIT1);						// Set P10£¬P11 as Push-Pull for MOSI,SPICLK
	PORT_SetP6OpenDrainPu(BIT1);						// set P60 as open-drain with pull-high for MISO
}

///***********************************************************************************
//Function:   	BYTE Uart1SPITransceiver(BYTE ToSend)
//Description:   	UART1 SPI Master transmit and receive data
//Input:   		BYTE ToSend
//Output:     
//*************************************************************************************/
BYTE Uart1SPITransceiver(BYTE ToSend)
{
	u8 x;
	SFR_Page_(1);
	S1BUF=ToSend;
	while(TI1==0);
	TI1=0;
	x=S1BUF;
	SFR_Page_(0);
	return x;
}

///***********************************************************************************
//Function:   	void InitUart1_SPI_Master(void)
//Description:   	Initialize Uart1, SPI Master
//Input:   
//Output:     
//*************************************************************************************/
void InitUart1_SPI_Master(void)
{
	P11=1;
	UART1_SetModeSPIMaster();			// UART1 Mode: SPI-Master
	UART1_SetMISOP61();					// MISO Pin:P61
	UART1_SetRxTxP10P11();				// RX£¨MOSI£©->P10,TX(SPI_CLK)->P11
#if (SPI_CLOCK == SPI_CLK_SYSCLK_4)
	UART1_SetSPIClock_SYSCLKDiv4();		// Clock source: SYSCLK/4
#else
	UART1_SetSPIClock_SYSCLKDiv12();	/// Clock source: SYSCLK/12
#endif
#if (SPI_DORD==MSB)
	UART1_SetDataMSB();
#else
	UART1_SetDataLSB();
#endif

#if (SPI_CPOL==SPICLK_LOW)
	UARTSPICLK = 0x23;					// SPICLK  = Low

#endif

	
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
	InitUart1_SPI_Master();
}

void main()
{
	InitSystem();
	LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_1=1;LED_R=1;	
	while(1)
	{
		DelayXms(100);
		LED_G_0 = !LED_G_0;	
		Uart1SPITransceiver(0x05);	
	}

}


