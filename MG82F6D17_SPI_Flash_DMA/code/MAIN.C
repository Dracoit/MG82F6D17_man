///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=24MHz, SysCLK=48MHz
//	Description:
//			SPI(nSS/P33,MOSI/P15,MISO/P16,SPICLK/P17) Read/Write EN25Q32A
//			SPI clock :24MHz
//			Test by with DMA/ without DMA
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
#define MCU_SYSCLK		48000000
/*************************************************/
///*************************************************
//Set  CpuClk (MAX.36MHz)
//	1) CpuCLK=SysCLK
//	2) CpuClk=SysClk/2
//*************************************************/
//#define MCU_CPUCLK		(MCU_SYSCLK)
#define MCU_CPUCLK		(MCU_SYSCLK/2)

#define TIMER_1T_1ms_TH	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(1000000)))) /256) 			
#define TIMER_1T_1ms_TL	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(1000000)))) %256)

#define TIMER_12T_1ms_TH	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(12000000)))) /256) 			
#define TIMER_12T_1ms_TL	((65536-(u16)(float)(1000*((float)(MCU_SYSCLK)/(float)(12000000)))) %256)

#define LED_G_0		P33
#define LED_R		P34
#define LED_G_1		P35


#define SPI_nSS		P33


// SPI
#define Dummy_Data					0xFF
#define FLASH_BLOCK_SIZE			(64L*1024L)					// 64K Byte

#define FLASH_TEST_START_ADDRESS	0x00000000
#define FLASH_TEST_SIZE_MAX			(1024L*1024L)				// 1M Byte

xdata u8 TestBuf[512];
u16 TestBufCnt;

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
//Function:		void SendByte(u8 tByte)
//Description:	send byte
//Input:			u8 tByte: the data to be send
//Output:     
//*************************************************************************************/
void SendByte(u8 tByte)
{
	bit bES0;
	bES0=ES0;
	ES0=0;
	S0BUF=tByte;
	while(TI0==0);
	TI0=0;
	ES0=bES0;
}



///***********************************************************************************
//Function:		void SendStr(BYTE* PStr)
//Description:	send string
//Input: 			u8* PStr:the string to be send
//Output:     
//*************************************************************************************/
void SendStr(u8* PStr)
{
	while(*PStr != 0)
	{
		SendByte(*PStr);
		PStr ++;
	}
}

///***********************************************************************************
//Function:   	void SendHex(u8 ToSend)
//Description:	send data for HEX
//Input: 			u8 ToSend: the data to be send
//Output:     
//*************************************************************************************/
void SendHex(u8 ToSend)
{
	u8 x;
//	SendByte(' ');
//	SendByte('0');
//	SendByte('x');
	x = ToSend >> 4;
	if(x < 10)
	{
		SendByte('0'+x);
	}
	else
	{
		SendByte('A'-10+x);
	}
	x= ToSend&0x0F;
	if(x < 10)
	{
		SendByte('0'+x);
	}
	else
	{
		SendByte('A'-10+x);
	}
}

///***********************************************************************************
//Function:   	void SendDW2DEC(u32 ToSend)
//Description:	send data for DEC
//Input: 			u32 ToSend: the data(4 bytes) to be send
//Output:     
//*************************************************************************************/
void SendDW2DEC(u32 ToSend)
{
	bit bFlag;
	u8 x;
	bFlag=0;
	x=ToSend/1000000000L;
	ToSend=ToSend%1000000000L;
	if(x!=0)
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/100000000L;
	ToSend=ToSend%100000000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/10000000L;
	ToSend=ToSend%10000000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/1000000L;
	ToSend=ToSend%1000000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/100000L;
	ToSend=ToSend%100000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/10000L;
	ToSend=ToSend%10000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/1000L;
	ToSend=ToSend%1000L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/100L;
	ToSend=ToSend%100L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend/10L;
	if((x!=0)||(bFlag==TRUE))
	{	bFlag=TRUE;
		SendByte('0'+x);
	}
	
	x=ToSend%10L;
	SendByte('0'+x);
		
}

///***********************************************************************************
//Function:   	void ResetCounter()
//Description:	reset counter
//Input:  
//Output:     
//*************************************************************************************/
void ResetCounter()
{
	// stop T0,T1
	TM_GlobalSTOP(TIMER0_GLOBAL_BIT|TIMER1_GLOBAL_BIT);
	TM_SetT0HighByte(0);
	TM_SetT0LowByte(0);
	TM_SetT1HighByte(0);
	TM_SetT1LowByte(0);
	// start T0,T1
	TM_GlobalStart(TIMER0_GLOBAL_BIT|TIMER1_GLOBAL_BIT);
}
///***********************************************************************************
//Function:   	void SendCounter()
//Description:	output counter value by UART0
//Input:  
//Output:     
//*************************************************************************************/
void SendCounter()
{
	DWordTypeDef dwTime;
	TM_GlobalSTOP(TIMER0_GLOBAL_BIT|TIMER1_GLOBAL_BIT);
	dwTime.B.BHigh=TH0;
	dwTime.B.BMHigh=TL0;
	dwTime.B.BMLow=TH1;
	dwTime.B.BLow=TL1;
	SendStr("\nTIME: ");
	SendDW2DEC(dwTime.DW);
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
	UART0_SetS0BRGValue(S0BRG_BRGRL_115200_2X_48000000_1T);

	UART0_EnS0BRG();					// Enable S0BRG
}

///***********************************************************************************
//Function:   	void InitTimer0()
//Description:	Initialize Timer0
//		 T0 16bit Timer,CLK: Sysclk/192 6us~393216us	
//Input:   
//Output:   		
//*************************************************************************************/
void InitTimer0(void)
{
	TM_SetT0Mode_1_16BIT_TIMER();			// TIMER0 Mode: 16-bit timer
	TM_SetT0Clock_T1OF();					// TIMER0 Clock source: T1OF
	TM_SetT0Gate_Disable();
	TM_SetT0LowByte(0);						// Sets TL0 value
	TM_SetT0HighByte(0);					// Sets TH0 value
	//TM_EnableT0();						// Enable TIMER0
}

///***********************************************************************************
//Function:   	void InitTimer1()
//Description:	Initialize Timer1
//		 T1 16bit Timer,CLK: Sysclk/48 1.5us~98304us 	
//Input:   
//Output:   		
//*************************************************************************************/
void InitTimer1(void)
{
	TM_SetT1Mode_1_16BIT_TIMER();			// TIMER1 Mode: 16-bit timer
	TM_SetT1Clock_SYSCLKDiv48();			// TIMER1 Clock source: SYSCLK/48
	TM_SetT1Gate_Disable();
	TM_SetT1LowByte(0);						// Sets TL1 value
	TM_SetT1HighByte(0);					// Sets TH1 value
	//TM_EnableT1();						// Enable TIMER1
}

///***********************************************************************************
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{
	PORT_SetP1PushPull(BIT5|BIT7);				// Set P15(MOSI),P17(SPICLK) as push-pull for output
	PORT_SetP1OpenDrainPu(BIT6);				// Set P16(MISO) as open-drain with pull-high for digital input
	PORT_SetP3PushPull(BIT3);					// Set P33(nSS) as push-pull for output
	PORT_P14P15P16P17_HighDrive();  
	PORT_P1_EnFastDrive(BIT4|BIT5|BIT6|BIT7);
	PORT_P3_EnFastDrive(BIT3);
	
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
//Function:   	void InitSPI_Master()
//Description:	Initialize SPI Master
//Input:   
//Output:   		
//*************************************************************************************/
void InitSPI_Master(void)
{
	SPI_Enable();									// Enable SPI
	SPI_SelectMASTERByMSTRbit();					// Set to MASTER
	SPI_SetClock(SPI_CLK_SYSCLK_4);					// Set Clock SYSCLK/4 12M/4=3M
	SPI_SetCPOL_0();								// CPOL=0 
	SPI_SetDataMSB();								// Data MSB
	SPI_SetCPHA_0();								// CPHA=0
	SPI_SetUseP33P15P16P17();						// IO Port: nSS/P33,MOSI/P15,MISO/P16,SPICLK/P17
}

///***********************************************************************************
//Function:   	void InitDMA_XRAM_TO_SPI()
//Description:	Initialize DMA, XRAM->SPI
//Input:   
//Output:   		
//*************************************************************************************/
void InitDMA_XRAM_TO_SPI()
{
	DMA_Disable();									// Disable DMA
	DMA_SetSource_XRAM();							// Source: XRAM
	DMA_SetDestination_SPI0TX();					// Destination: SPI-TX
	DMA_SetXRAMStartaddr((u16)(&TestBuf[0]));		// XRAM Start address
	DMA_SetXRAMStartaddrRL((u16)(&TestBuf[0]));		// XRAM Start address Reload
	DMA_SetCount(65536-256);						// Counter: 512
	DMA_SetCountRL(65536-256);						// Counter Reload: 512
	DMA_SetTrig_Software();							// Trigger source: software(set DMAS0)
	DMA_DisDCF0Int();								// Disable DCF0 trigger DMA interrupt
	DMA_DisLoop();									// Disable Loop mode
	DMA_Enable();									// Enable DMA
}

///***********************************************************************************
//Function:   	void InitDMA_SPI_TO_XRAM()
//Description:	Initialize DMA, SPI->XRAM
//Input:   
//Output:   		
//*************************************************************************************/
void InitDMA_SPI_TO_XRAM()
{
	DMA_Disable();									// Disable DMA
	DMA_SetSource_SPI0RX();							// Source: SPI-RX
	DMA_SetDestination_XRAM();						// Destination: XRAM
	DMA_SetXRAMStartaddr((u16)(&TestBuf[0]));		// XRAM Start address
	DMA_SetXRAMStartaddrRL((u16)(&TestBuf[0]));		// XRAM Start address Reload
	DMA_SetCount(65536-512);						// Counter: 512
	DMA_SetCountRL(65536-512);						// Counter Reload: 512
	DMA_SetTrig_Software();							// Trigger source: software(set DMAS0)
	DMA_DisDCF0Int();								// Disable DCF0 trigger DMA interrupt
	DMA_DisLoop();									// Disable Loop mode
	DMA_Enable();									// Enable DMA
}

///***********************************************************************************
//Function:   	u8 SPITransceiver(u8 SPI_DATA)
//Description:	SPI Master transmit 
//Input:   		u8 SPI_DATA: Data to be send
//Output:   		u8:  Received data
//*************************************************************************************/
u8 SPITransceiver(u8 SPI_DATA)
{
	SPI_SendData(SPI_DATA);							// Send data
	while(SPI_ChkCompleteFlag()==0);				// Wait complete
	SPI_ClearCompleteFlag();						// Clear flag
	return SPI_GetData();							// Return data
}

/*
*************************************************************************************
*  SPI FLASH EN25Q32A
*
*************************************************************************************
*/

///***********************************************************************************
//Function:		u8 EN25Q32A_Flash_1ByteRead (u32 Address)
//Description:	Read 1 byte from EN25Q32A 
//Input:	uint32_t Address: Address 0x00000000 ~ 0xFFFFFFFF
//Output: uint8_t: data 	  
//*************************************************************************************/
u8 EN25Q32A_Flash_1ByteRead (u32 Address)
{ 
	u8 x;
	SPI_nSS = 0;
    
    /* Write command */
    SPITransceiver(0x03);
	
    /* Write address */
    SPITransceiver((u8)(Address>>16));
    SPITransceiver((u8)(Address>>8));
    SPITransceiver((u8)(Address));
    
    /* Read data */
    x=SPITransceiver(Dummy_Data);

	SPI_nSS = 1;

    return  x;
}


///***********************************************************************************
//Function:		void EN25Q32A_Flash_MultiBytesRead (u32 Address, u8 *BufferAddreass, u32 Length)
//Description:	Read n bytes from EN25Q32A 
//
//Input:	  u32 Address: Address 0x00000000 ~ 0xFFFFFFFF
//		  u8 *BufferAddreass: Buffer address
//		  u16 Length: Buffer length
//Output: 
//*************************************************************************************/
void EN25Q32A_Flash_MultiBytesRead (u32 Address, u8 *BufferAddreass, u16 Length)
{
	SPI_nSS = 0;
    /* Write command */
    SPITransceiver(0x03);
	
    /* Write address */
    SPITransceiver((u8)(Address>>16));
    SPITransceiver((u8)(Address>>8));
    SPITransceiver((u8)(Address));
    
    /* Read data */
    while(Length !=0)
	{
		*BufferAddreass=SPITransceiver(Dummy_Data);
        BufferAddreass ++;
        Length--;
	}
	SPI_nSS = 1;
}

///***********************************************************************************
//Function:		u32 EN25Q32A_Flash_Read_ID (void)
//Description:	read EN25Q32A ID 
//
//Input:	
//Output: uint32_t: EN25Q32A ID
//*************************************************************************************/
u32 EN25Q32A_Flash_Read_ID (void)
{
	DWordTypeDef RDAT;
    
    SPI_nSS = 0;
    /* Write command */
    SPITransceiver(0x9F);
    
    /* Read ID */
    RDAT.B.BMHigh=SPITransceiver(Dummy_Data);
    RDAT.B.BMLow=SPITransceiver(Dummy_Data);
    RDAT.B.BLow=SPITransceiver(Dummy_Data);
	SPI_nSS  = 1;
    
    return RDAT.DW;
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Write_Enable (void)
//Description: 	Enable write Flash
//
//Input:	
//Output: 
//*************************************************************************************/
void EN25Q32A_Flash_Write_Enable(void)
{
	u8 RDAT;
	
    
  Re_Flash_Write_Enable:
    /* Write enable */
	SPI_nSS  = 0;
	SPITransceiver(0x06);
	SPI_nSS  = 1;

    /* Read status register */
	SPI_nSS  = 0;
	SPITransceiver(0x05);			//RDSR
   
	RDAT = SPITransceiver(Dummy_Data);                                  // Get received data

	SPI_nSS  = 1;
	
    /* Check WEL == 1 */
	if((RDAT & 0x02) == 0x00)
		goto Re_Flash_Write_Enable;
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Check_Busy(void)
//Description:	check busy 
//
//Input:	
//Output: 
//*************************************************************************************/
void EN25Q32A_Flash_Check_Busy(void)
{
	u8 RDAT;
    
    
    /* Read status register */
	Re_Flash_Check_Busy:
	SPI_nSS  = 0;
	SPITransceiver(0x05);							//RDSR

	RDAT = SPITransceiver(Dummy_Data);                                  // Get received data
	
    /* Check erase or write complete */
	SPI_nSS  = 1;
	if((RDAT & 0x01) == 0x01)			//WIP, write in progress
		goto Re_Flash_Check_Busy;
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Page_Program(u32 Addr, u8 *DataSource, u16 Length)
//Description:	Write n bytes to EN25Q32A
//
//Input:	u32 Addr: Start address. 0x00000000 ~ 0xFFFFFFFF.
//		  u8 *DataSource: Buffer address
//		  u16 Length: Buffer length, MAX.256 bytes
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Page_Program(u32 Addr, u8 *DataSource, u16 Length)
{
    EN25Q32A_Flash_Write_Enable();
    
	SPI_nSS  = 0;
	SPITransceiver( 0x02);                               // Write command 
	
    /* Write address */
    SPITransceiver((u8)(Addr>>16));
    SPITransceiver((u8)(Addr>>8));
    SPITransceiver((u8)(Addr));
	
	while(Length!=0)                                            // Write Data
	{
		SPITransceiver(*DataSource);
        DataSource++;
		Length--;
	}
	
	SPI_nSS  = 1;
   
	EN25Q32A_Flash_Check_Busy();                                       // Wait program complete
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Read_Start(u32 Addr)
//Description:	EN25Q32A start read 
//
//Input:	u32Addr: Start address. 0x00000000 ~ 0xFFFFFFFF.
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Read_Start(u32 Addr)
{

	SPI_nSS  = 0;
   
    /* Write command */
    SPITransceiver(0x03);
	
    /* Write address */
    SPITransceiver((u8)(Addr>>16));
    SPITransceiver((u8)(Addr>>8));
    SPITransceiver((u8)(Addr));
    
	
}
///***********************************************************************************
//Function:		void EN25Q32A_Flash_Read_Stop(void)
//Description:	EN25Q32A stop read
//Input:	
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Read_Stop()
{

	SPI_nSS  = 1;
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Page_Program_Start(u32 Addr)
//Description:	EN25Q32A start page program
//
//Input:	u32Addr: start address. 0x00000000 ~ 0xFFFFFFFF.
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Page_Program_Start(u32 Addr)
{

    EN25Q32A_Flash_Write_Enable();
    
	SPI_nSS  = 0;
	SPITransceiver( 0x02);                               // Write command 
                
    /* Write address */
    SPITransceiver((u8)(Addr>>16));
    SPITransceiver((u8)(Addr>>8));
    SPITransceiver((u8)(Addr));
    
	
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Program_One_Word(u16 wData)
//Description:	EN25Q32A write a WORD(16bit)
//
//Input:	u16 wData: data to be write
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Program_One_Word(u16 wData)

{
    SPITransceiver((u8)(wData>>8));
    SPITransceiver((u8)(wData));
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Page_Program_Stop(void)
//Description:	EN25Q32A stop page program
//
//Input:	
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Page_Program_Stop()
{

	SPI_nSS  = 1;
	EN25Q32A_Flash_Check_Busy();                                       // Wait program complete
}

///***********************************************************************************
//Function:		void EN25Q32A_Flash_Block_Erase(u32 Addr)
//Description:	EN25Q32A block erase
//
//Input:	u32 Addr: start addresss.
//
//Output: 
//*************************************************************************************/ 
void EN25Q32A_Flash_Block_Erase(u32 Addr)
{
    EN25Q32A_Flash_Write_Enable();
    
	SPI_nSS  = 0;
	SPITransceiver(0xD8);                               			// Block Erase command 
                
	
    /* Write address */
    SPITransceiver((u8)(Addr>>16));
    SPITransceiver((u8)(Addr>>8));
    SPITransceiver((u8)(Addr));
	
	SPI_nSS  = 1;
   
	EN25Q32A_Flash_Check_Busy();                                       // Wait Block Erase complete
}


/*
*************************************************************************************
*/ 


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
	InitSPI_Master();
	InitUart0_S0BRG();
	InitTimer0();
	InitTimer1();
}


void main()
{
	u32 i;
	
	DWordTypeDef dwTemp;
	
    InitSystem();
	
	SendStr("\nHello!");
	LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_1=1;LED_R=1;


	
	dwTemp.DW=EN25Q32A_Flash_Read_ID();
	SendStr("\nEN25Q32A ID: ");
	SendHex(dwTemp.B.BMHigh);
	SendHex(dwTemp.B.BMLow);
	SendHex(dwTemp.B.BLow);

	// Test without DMA (1M bytes)
	SendStr("\nEN25Q32A Erase...");
	i=FLASH_TEST_SIZE_MAX/FLASH_BLOCK_SIZE;
	// erase Flash
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	ResetCounter();
	while(i!=0)
	{
		LED_R = !LED_R;
		EN25Q32A_Flash_Block_Erase(dwTemp.DW);
		dwTemp.DW=dwTemp.DW+FLASH_BLOCK_SIZE;
		i--;
	}
	SendCounter();
	for(i=0;i<256;i++)
	{
		TestBuf[i]=i;
	}
	// write Flash
	SendStr("\nEN25Q32A Write 1M");
	i=FLASH_TEST_SIZE_MAX/256;		
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	ResetCounter();
	while(i!=0)
	{
		LED_R = !LED_R;
		EN25Q32A_Flash_Page_Program(dwTemp.DW,&TestBuf[0],256);
		dwTemp.DW=dwTemp.DW+256;
		i--;
	}
	SendCounter();
	// read Flash
	SendStr("\nEN25Q32A Read 1M");
	i=FLASH_TEST_SIZE_MAX/512;		
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	ResetCounter();
	while(i!=0)
	{
		LED_R = !LED_R;
		EN25Q32A_Flash_MultiBytesRead(dwTemp.DW,&TestBuf[0],512);
		dwTemp.DW=dwTemp.DW+512;
		i--;
	}
	SendCounter();


	// Test with DMA (1M bytes)
	SendStr("\nEN25Q32A Erase...");
	i=FLASH_TEST_SIZE_MAX/FLASH_BLOCK_SIZE;
	// erase Flash
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	ResetCounter();
	while(i!=0)
	{
		LED_R = !LED_R;
		EN25Q32A_Flash_Block_Erase(dwTemp.DW);
		dwTemp.DW=dwTemp.DW+FLASH_BLOCK_SIZE;
		i--;
	}
	SendCounter();
	for(i=0;i<256;i++)
	{
		TestBuf[i]=i;
	}
	// write Flash
	SendStr("\nDMA EN25Q32A Write 1M");
	i=FLASH_TEST_SIZE_MAX/256;		
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	InitDMA_XRAM_TO_SPI();					// config DMA, XRAM->SPI
	ResetCounter();
	while(i!=0)
	{
		LED_R = !LED_R;
		EN25Q32A_Flash_Page_Program_Start(dwTemp.DW);
		DMA_ClearCompleteFlag();			// clear DMA completed flag
		DMA_Start();						// Start DMA, start transmit 256 bytes to EN25Q32A by SPI
		while(DMA_ChkCompleteFlag()==0)		// wait DMA completed
		{

		}
		while(SPI_ChkCompleteFlag()==0);	// wait the lastest completed
		SPI_ClearCompleteFlag();			// clear SPI flag
		EN25Q32A_Flash_Page_Program_Stop();
		dwTemp.DW=dwTemp.DW+256;
		i--;
	}
	SendCounter();
	// read Flash
	SendStr("\nDMA EN25Q32A Read 1M");
	i=FLASH_TEST_SIZE_MAX/512;		
	dwTemp.DW=FLASH_TEST_START_ADDRESS;
	InitDMA_SPI_TO_XRAM();					// config DMA, SPI->XRAM
	ResetCounter();
	EN25Q32A_Flash_Read_Start(dwTemp.DW);
	while(i!=0)
	{
		LED_R = !LED_R;
		DMA_ClearCompleteFlag();			// clear DMA completed flag
		DMA_Start();						// Start DMA, start transmit 512 bytes to XRAM by SPI
		while(DMA_ChkCompleteFlag()==0)		// wait DMA completed
		{

		}
		i--;
	}
	EN25Q32A_Flash_Read_Stop();				
	SendCounter();
	
	
    while(1)
    {
    	DelayXms(100);
		LED_G_1 = !LED_G_1;
    }
}


