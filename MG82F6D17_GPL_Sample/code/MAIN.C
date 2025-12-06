///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			GPL Bit Order Reversed
//			GPL CRC calculate(CCIT16 CRC16 0x1021)		
//			GPL Flash automatic CRC
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


#define SFR_Page_(x)		SFRPI = x;

code u8 TestBuf[16]={0xAA,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


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
//Function:   	u32 GetCounter()
//Description:	get value of the counter
//Input:  
//Output:     
//*************************************************************************************/
u32 GetCounter()
{
	DWordTypeDef dwTime;
	TM_GlobalSTOP(TIMER0_GLOBAL_BIT|TIMER1_GLOBAL_BIT);
	dwTime.B.BHigh=TH0;
	dwTime.B.BMHigh=TL0;
	dwTime.B.BMLow=TH1;
	dwTime.B.BLow=TL1;
	return dwTime.DW;
}

///***********************************************************************************
//Function:   	void InitTimer0()
//Description:	Initialize Timer0
//		 T0: 16bit Timer,CLK: Sysclk/192 6us~393216us	
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
//		 T1: 16bit Timer,CLK: Sysclk/48 1.5us~98304us 
//Input:   
//Output:   		
//*************************************************************************************/
void InitTimer1(void)
{
	TM_SetT1Mode_1_16BIT_TIMER();			// TIMER1 Mode: 16-bit timer
	TM_SetT1Clock_SYSCLKDiv12();			// TIMER1 Clock source: SYSCLK/12
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
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);		// set P30,P31,P33,P34,P35 as Quasi-Bidirectional
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
//Function:       void InitSystem(void)
//Description:    Initialize MCU
//Input:   
//Output:     
//*************************************************************************************/
void InitSystem(void)
{
	InitPort();
	InitUart0_S0BRG();
	InitTimer0();
	InitTimer1();
}

///***********************************************************************************
//Function:   u16 CRC16_CCIT(u8 *ptr,u16 len)
//Description: calculation CRC16 (0x1021) by SW
//Input:   
//		 u8 *pBuf: the Buffer to be CRC
//		 u16 len: the length of Buffer
//Output:     
//		 u16: CRC16
//*************************************************************************************/
u16 CRC16_CCIT(u8 *ptr,u16 len)
{
	u16 wCRCin = 0x0000;
	u16 wCPoly = 0x1021;
	u16 wChar=0;
	u16 i;
	u8 x;
	for(i=0;i<len;i++)
	{
		wChar=(ptr[i])&0x00FF;
		wCRCin=wCRCin^(wChar<<8);
		for(x=0;x<8;x++)
		{
			if((wCRCin&0x8000)!=0)
			{
				wCRCin=wCRCin<<1;
				wCRCin=wCRCin^wCPoly;
			}
			else
			{
				wCRCin=wCRCin<<1;
			}
		}
	}
    return wCRCin;
}


///***********************************************************************************
//Function:   	u16 GPL_CRC(u8 *pBuf,u8 len)
//Description:	calculation CRC16 (0x1021) 
//Input:   		
//				u8 *pBuf: the Buffer to be CRC
//				u8 len: the length of Buffer 
//Output:   		
//				u16:CRC16
//*************************************************************************************/
u16 GPL_CRC(u8 *pBuf,u8 len)
{
	WordTypeDef CRCValue;
	u8 i;
	// set CRC Seed
	GPL_CRC_WriteCRCSeedH(0x00);
	GPL_CRC_WriteCRCSeedL(0x00);
	
	for(i=0;i<len;i++)
	{
		GPL_CRC_WriteCRCData(pBuf[i]);
	}
	GPL_CRC_ReadCRCResultH(CRCValue.B.BHigh);
	GPL_CRC_ReadCRCResultL(CRCValue.B.BLow);
	return CRCValue.W;
}

///***********************************************************************************
//Function:   	u16 GPL_AutoFlashCRC(u16 StartAddr,u16 EndAddr)
//Description:	Automatic calculation CRC for flash
//Input:   		
//				u16 StartAddr: Flash start address
//				u16 EndAddr: Flash end address( The last byte of the flash page. 
//							Ex: ....0x33FF,0x35FF,0x37FF,0x39FF,0x3BFF,0x3DFF,0x3FFF.) 
//Output:   		
//				u16:CRC16
//*************************************************************************************/
u16 GPL_AutoFlashCRC(u16 StartAddr,u16 EndAddr)
{
	WordTypeDef CRCValue;
	bit bEA=EA;					// Backup EA
	// Set CRC Seed
	GPL_CRC_WriteCRCSeedH(0x00);
	GPL_CRC_WriteCRCSeedL(0x00);

	GPL_CRC_CRC0DA_DataIn();
	
	// Set End address
	EA = 0; 					// Disable interrupt
	IFADRH = 0; 				// IFADRH must 0
	IFADRL= IAPLB_P;			// IAPLB address
	IFD= (EndAddr>>8)&0xFE;		// IAPLB 
	IFMT = ISP_WRITE_P;			// Write P-Page SFR
	ISPCR = 0x80;				// start ISP/IAP
	SCMD = 0x46;
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear ISPCR

	// Set Start address
	IFADRH = StartAddr/256; 	
	IFADRL= StartAddr%256;		
	IFMT = ISP_AUTO_FLASH_CRC;	// Automatic flash read for CRC
	ISPCR = 0x80;				// start ISP/IAP
	SCMD = 0x46;
	SCMD = 0xB9;
	nop();
	GPL_CRC_ReadCRCResultH(CRCValue.B.BHigh);		// Read CRC result
	GPL_CRC_ReadCRCResultL(CRCValue.B.BLow);
	IFMT=0;
	ISPCR = 0;					// clear ISPCR
	EA = bEA;					// restore EA
	return CRCValue.W;
}


void main()
{
	WordTypeDef CRCValue;
	u8 TestBorev;
	DWordTypeDef dwTime;
	

	unsigned char volatile code *pCode;

	InitSystem();

	SendStr("\nStart GPL Test!");

	LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_1=1;LED_R=1;	

	// Bit Order Reversed
	TestBorev=0x46;
	SendStr("\nBorev old:0x");
	SendHex(TestBorev);
	GPL_BOREV(TestBorev);
	SendStr("\nBorev new:0x");
	SendHex(TestBorev);

	
	// HW Calculate CRC16 of the FLASH 0x0000~0x03FF
	ResetCounter();
	CRCValue.W=GPL_AutoFlashCRC(0x0000, (1024-1));
	dwTime.DW=GetCounter();
	SendStr("\nHW 1KFlash CRC16 Value:0x");
	SendHex(CRCValue.B.BHigh);
	SendHex(CRCValue.B.BLow);
	SendStr("; Time:");
	SendDW2DEC(dwTime.DW);

	// SW Calculate CRC16 of the FLASH 0x0000~0x03FF
	pCode=0x0000;
	ResetCounter();
	CRCValue.W=CRC16_CCIT(pCode, 1024);
	dwTime.DW=GetCounter();
	SendStr("\nSW 1KFlash CRC16 Value:0x");
	SendHex(CRCValue.B.BHigh);
	SendHex(CRCValue.B.BLow);
	SendStr("; Time:");
	SendDW2DEC(dwTime.DW);

	// HW Calculate CRC16 of TestBuf[16]
	ResetCounter();
	CRCValue.W=GPL_CRC(TestBuf,16);
	dwTime.DW=GetCounter();
	SendStr("\nHW TestBuf[16] CRC16 Value:0x");
	SendHex(CRCValue.B.BHigh);
	SendHex(CRCValue.B.BLow);
	SendStr("; Time:");
	SendDW2DEC(dwTime.DW);
	
	// SW Calculate CRC16 of TestBuf[16]
	ResetCounter();
	CRCValue.W=CRC16_CCIT(TestBuf,16);
	dwTime.DW=GetCounter();
	SendStr("\nSoft TestBuf[16] CRC16 Value:0x");
	SendHex(CRCValue.B.BHigh);
	SendHex(CRCValue.B.BLow);
	SendStr("; Time:");
	SendDW2DEC(dwTime.DW);
	while(1)
	{
		DelayXms(100);
		LED_G_0 = !LED_G_0;
	}

}


