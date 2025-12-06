///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//    		TWSI (SCL/P22, SDA/P24) Slave interrupt
//
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

#define	TWI_OV_TIME_MAX			20

#define I2C_SCL		P22
#define I2C_SDA		P24

#define SLAVE_ADDRESS	0xA0


u8	TWI0OvTime;
bit bTWI0Error;

idata BYTE Tx[5];
idata BYTE Rx[5];
u8 RxInx;
u8 TxInx;

bit bES0;

u8 LedTime;

#define LED_G_0		P33
#define LED_R		P34
#define LED_G_1		P35


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
	if(TWI0OvTime!=0) 
	{
		TWI0OvTime--;
		if(TWI0OvTime==0) bTWI0Error=TRUE;
	}
	if(LedTime!=0) LedTime--;
}


///***********************************************************************************
//Function:   	void INT_TWSI(void)
//Description:	TWI0 Interrupt handler
//		 
//Input:   
//Output:     
//*************************************************************************************/
void INT_TWSI() interrupt INT_VECTOR_TWI0
{
	if(SISTA==0x80)
	{ // 0x80: DATA has been received. ACK has been returned.
		Rx[RxInx]=SIDAT;				// receive data
		RxInx++;
		if(RxInx>=5) RxInx=0;
	}
	else if((SISTA==0x60)||(SISTA==0x68))
	{// 0x60,0x68:Own SLA+W has been received. ACK has been returned
		RxInx=0;	// Clear Rx buffer					
	}
	else if((SISTA==0xA8)||(SISTA==0xB0))
	{// 0xA8,0xB0:Own SLA+R has been received. ACK has been returned
		TxInx=0;	// Clear Tx buffer					
		goto _IIC_SET_SIDAT;
	}
	else if((SISTA==0xB8)||(SISTA==0xC0)||(SISTA==0xC8))
	{
		// 0xB8:  Data byte in SIDAT has been transmitted ACK has been received
		// 0xC0:  Data byte or Last data byte in SIDAT has been transmitted Not ACK has been received
		// 0xC8:  Last Data byte in SIDAT has been transmitted ACK has been received
_IIC_SET_SIDAT:
		SIDAT=Tx[TxInx]; 				// send data
		TxInx++;
		if(TxInx>=5) TxInx=0;
	}
	I2C_SCL=0;
	SICON = ENSI|AA;					// clear TWI0 interrupt flag
	TWI0OvTime = TWI_OV_TIME_MAX;		// restore TWI0 overtime
	I2C_SCL=1;
}

///***********************************************************************************
//Function:		void SendByte(u8 tByte)
//Description:	send byte
//Input:			u8 tByte: the data to be send
//Output:     
//*************************************************************************************/
void SendByte(BYTE tByte)
{
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
void SendStr(BYTE* PStr)
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
void SendHex(BYTE ToSend)
{
	BYTE x;
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
	PORT_SetP2OpenDrainPu(BIT2|BIT4);					// Set P22,P24 as open-drain with pull-high for I2C
}

///***********************************************************************************
//Function:   	void InitTWI0_Slave(void)
//Description: 	Initialize TWI0 for Slave
//Input:   
//Output:   		
//*************************************************************************************/
void InitTWI0_Slave()
{
	
	TWI0_Clear();
	TWI0_SetClock(TWI0_CLK_SYSCLK_32);
	TWI0_SetUseP22P24();
	TWI0_Enable();
	TWI0_SetSlaveAddr(SLAVE_ADDRESS);
	TWI0_SendACK();

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
//Function:   	void InitTimer0()
//Description:	Initialize Timer0
//Input:   
//Output:   		
//*************************************************************************************/
void InitTimer0(void)
{
	TM_SetT0Mode_1_16BIT_TIMER();			// TIMER0 Mode: 16-bit
	TM_SetT0Clock_SYSCLKDiv12();			// TIMER0 Clock source: SYSCLK/12
	TM_SetT0Gate_Disable();					// TIMER0 disable gate

	TM_SetT0LowByte(TIMER_12T_1ms_TL);		// Sets TL0 value
	TM_SetT0HighByte(TIMER_12T_1ms_TH);		// Sets TH0 value

	TM_EnableT0();							// Enable TIMER0
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
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnTIMER0();						// Enable Timer0 interrupt
	INT_EnTWI0();						// Enable TWI0 interrupt
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
	InitClock();
	InitTWI0_Slave();
	InitTimer0();
	InitUart0_S0BRG();
	InitInterrupt();

	INT_EnAll();			//  Enable global interrupt
}



void main()
{
	
    InitSystem();

	Tx[0]=0x55;
	Tx[1]=0xAA;
	Tx[2]=0x5A;
	Tx[3]=0xA5;
	Tx[4]=0x0A;

 	SendStr("\nStart");
 	
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_0=1;LED_G_1=1;LED_R=1;

	
					
    while(1)
    {
    	if(LedTime==0)
    	{
			LED_G_0 = !LED_G_0;
			LedTime=100;
    	}
    	if(bTWI0Error==TRUE)
    	{
    		bTWI0Error=FALSE;
			// error,reset TWI0
			LED_R=0;
			InitTWI0_Slave();
			LED_R=1;
    	}
    }
}


