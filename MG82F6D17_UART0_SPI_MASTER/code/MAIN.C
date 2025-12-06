///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			UART0£¬Mode4 (SPI Master)
//			SPICLK <-->TXD	P31
//			MOSI	<-->RXD  P30
//			MISO	<-->S0MI P16
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
	PORT_SetP3QuasiBi(BIT3|BIT4|BIT5);		// set P33,P34,P35 as Quasi-Bidirectional
	PORT_SetP3PushPull(BIT0|BIT1);			// Set P30,P31 as Push-Pull for MOSI,SPICLK
	PORT_SetP1OpenDrainPu(BIT6);			// set P16 as open-drain with pull-high for MISO
}

///***********************************************************************************
//Function:   	BYTE Uart0SPITransceiver(BYTE ToSend)
//Description:   	UART0 SPI Master transmit and receive data
//Input:   		BYTE ToSend
//Output:     
//*************************************************************************************/
BYTE Uart0SPITransceiver(BYTE ToSend)
{
	S0BUF=ToSend;
	while(TI0==0);
	TI0=0;
	return S0BUF;
}

///***********************************************************************************
//Function:   	void InitUart0_SPI_Master(void)
//Description:   	Initialize Uart0, SPI Master
//Input:   
//Output:     
//*************************************************************************************/
void InitUart0_SPI_Master(void)
{
	P31=1;
	UART0_SetModeSPIMaster();			// UART0 Mode: SPI-Master
	UART0_SetMISOP16();					// MISO Pin:P16
	UART0_SetRxTxP30P31();				// RX£¨MOSI£©->P30,TX(SPI_CLK)->P31
#if (SPI_CLOCK == SPI_CLK_SYSCLK_4)
	UART0_SetSPIClock_SYSCLKDiv4();		// Clock source: SYSCLK/4
#else
	UART0_SetSPIClock_SYSCLKDiv12();	// Clock source: SYSCLK/12
#endif

#if (SPI_DORD==MSB)
	UART0_SetDataMSB();
#else
	UART0_SetDataLSB();
#endif

#if (SPI_CPOL==SPICLK_LOW)									
	UARTSPICLK = 0x21;					// SPICLK  = Low
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
	InitUart0_SPI_Master();
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
		Uart0SPITransceiver(0x05);	
	}

}


