///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//			Description:
//				UART0, LIN Slave
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
#define LED_R		P34
#define LED_G_1		P35

#define LIN_TIME_MAX				10

#define LIN_SYNC_BYTE				0x55

#define LIN_PID_MASTER_TX_MSG		0x30		
#define LIN_PID_MASTER_RX_MSG		0x31

#define LIN_PID_MASTER_TX_MSG_P		0xF0		
#define LIN_PID_MASTER_RX_MSG_P		0xB1


// define LIN status
typedef enum
{
  LIN_IDLE_BREAK=0,
  LIN_SYNC,
  LIN_PID,
  LIN_SLAVE_INIT_DATA,
  LIN_MASTER_RX_DATA,
  LIN_MASTER_RX_CHECKSUM,
  LIN_MASTER_RX_DONE,
  LIN_MASTER_TX_DATA,
  LIN_MASTER_TX_CHECKSUM,
  LIN_MASTER_TX_DONE,
  LIN_ERR
}LIN_State;

// define LIN mode
typedef enum
{
  	LIN_MODE_MASTER_TX=0,
	LIN_MODE_MASTER_RX,
	LIN_MODE_SLAVE_TX,
	LIN_MODE_SLAVE_RX
}LIN_Mode;

// define frame struct
typedef struct
{
	LIN_Mode Mode;
  	LIN_State State;
  	u8 PID ;
  	u8 Data[8];
  	u8 DataLength;
  	u8 DataInx;
  	u8 Check;
} LIN_Frame ;

#define BIT(A,B)      ((A>>B)&0x01)  

idata LIN_Frame LINFrame;

u8 LedTime;
u8 LINOverTime;

void LIN_Slave_StartFrame(void);
u8 LIN_CalcParity(u8 id);
u8 LIN_CalcChecksum(u8 id,u8 *lindata,u8 length);

///***********************************************************************************
//Function:   	void INT_UART0(void)
//Description:	UART0 Interrupt handler
//		 
//Input:   
//Output:     
//*************************************************************************************/
void INT_UART0(void) interrupt INT_VECTOR_UART0
{
	u8 x;
	_push_(SFRPI);
	SFRPI=0;
	if(RI0==1)
	{
		// LIN status
		if(LINFrame.State==LIN_IDLE_BREAK)
		{
			if((S0CFG1&SBF0)!=0)
			{ // Rx Break done
				UART0_LIN_ClrSBF0();
				LINFrame.State=LIN_PID;				// if Auto Baud Rate , SYNC(0x55) for ABR, so the next data is PID
				//LINFrame.State=LIN_SYNC;			// if no ABR, the next data is SYNC(0x55)
				LINOverTime=LIN_TIME_MAX;
			}
		}
		/**/
		else if(LINFrame.State==LIN_SYNC)
		{ // Rx SYNC done
			x=S0BUF;
			if(x == LIN_SYNC_BYTE)
			{
				LINFrame.State++;
				LINOverTime=LIN_TIME_MAX;
			}
			else
			{
				LINFrame.State=LIN_ERR;
			}
		}
		/**/
		else if(LINFrame.State==LIN_PID)
		{ // Rx PID done
			LINOverTime=LIN_TIME_MAX;
			LINFrame.PID = S0BUF;
			if(LINFrame.PID == LIN_PID_MASTER_RX_MSG_P)
			{ // PID for Master get msg 
				LINFrame.State=LIN_SLAVE_INIT_DATA;
			}
			else if(LINFrame.PID == LIN_PID_MASTER_TX_MSG_P)
			{// PID for Master send msg 
				LINFrame.State=LIN_MASTER_TX_DATA;
				LINFrame.DataInx=0;
			}
			else 
			{// PID error
				LINFrame.State=LIN_ERR;
			}

		}
		else if(LINFrame.State==LIN_MASTER_TX_DATA)
		{
			LINOverTime=LIN_TIME_MAX;
			LINFrame.Data[LINFrame.DataInx] = S0BUF;
			LINFrame.DataInx++;
			if(LINFrame.DataInx>=LINFrame.DataLength)
			{
				LINFrame.State++;
			}

		}
		else if(LINFrame.State==LIN_MASTER_TX_CHECKSUM)
		{
			LINOverTime=LIN_TIME_MAX;
			LINFrame.Check=S0BUF;
			LINFrame.State++;
		}
		else if(LINFrame.State==LIN_MASTER_TX_DONE)
		{
			
		}
		else
		{
			LINFrame.State=LIN_ERR;
		}
		RI1 = 0;
	}
	else
	{
		TI1 = 0;
		if(LINFrame.State==LIN_MASTER_RX_DATA)
		{
			LINOverTime=LIN_TIME_MAX;
			S0BUF=LINFrame.Data[LINFrame.DataInx];
			LINFrame.DataInx++;
			if(LINFrame.DataInx>=LINFrame.DataLength)
			{
				LINFrame.State++;
			}
		}
		else if(LINFrame.State==LIN_MASTER_RX_CHECKSUM)
		{
			LINOverTime=LIN_TIME_MAX;
			S0BUF=LINFrame.Check;
			LINFrame.State++;
			
		}
		else if(LINFrame.State==LIN_MASTER_RX_DONE)
		{
			
		}
		else
		{
			LINFrame.State=LIN_ERR;
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
	if(LINOverTime!=0) LINOverTime--;
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
//Function:   	void InitUart0_LIN(void)
//Description:   	Initialize Uart0 for LIN mode
//Input:   
//Output:     
//*************************************************************************************/
void InitUart0_LIN(void)
{
	
	UART0_SetAccess_S0CR1();			// Enable access S0CR1 
	UART0_SetModeLIN();					// UART0 Mode:LIN 			
	UART0_EnReception();				// Enable reception
	UART0_SetBRGFromS0BRG();			// B.R. source: S0BRG
	UART0_SetS0BRGBaudRateX2();			// S0BRG x2
	UART0_SetS0BRGSelSYSCLK();			// S0BRG clock source: SYSCLK

	// Sets B.R. value
	UART0_SetS0BRGValue(S0BRG_BRGRL_19200_2X_12000000_1T);
	
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
	TM_SetT0Mode_1_16BIT_TIMER();			// TIMER0 Mode: 16-bit
	TM_SetT0Clock_SYSCLKDiv12();			// TIMER0 Clock source: SYSCLK/12
	TM_SetT0Gate_Disable();					// TIMER0 disable gate
	TM_SetT0LowByte(TIMER_12T_1ms_TL);						// Set TL0 value
	TM_SetT0HighByte(TIMER_12T_1ms_TH);					// Set TH0 value
	TM_EnableT0();							// Enable TIMER0
}


///***********************************************************************************
//Function:   	void InitInterrupt()
//Description:	Initialize Interrupt
//Input:   
//Output:   		
//*************************************************************************************/
void InitInterrupt(void)
{
	INT_EnUART0();						// Enable UART0 interrupt
	INT_EnTIMER0();						// Enable Timer0 interrupt
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
	InitUart0_LIN();
	InitInterrupt();
}


/*
*************************************************************************************
*  Function
*
*************************************************************************************
*/
///***********************************************************************************
//Function:   	void LIN_Slave_StartFrame(void)
//Description:   	LIN start receive
//Input:   
//Output:     
//*************************************************************************************/
void LIN_Slave_StartFrame(void)
{
	UART0_LIN_SetRX();
	UART0_LIN_WaitSYNC();
	// set S0BRG Value
	UART0_SetS0BRGValue(S0BRG_BRGRL_19200_2X_12000000_1T);
	// auto baud rate adjustment£¬
	UART0_LIN_AutoBR();			
	LINFrame.State=LIN_IDLE_BREAK;
	LINFrame.DataLength=8;
}

///***********************************************************************************
//Function:	uint8 LIN_CalcParity(uint8 id)
//Description: LIN calculate parity
//Input:	
//Output:	  
//*************************************************************************************/
u8 LIN_CalcParity(u8 id)
{
	u8 parity, p0,p1;
	parity=id; 
	p0=(BIT(parity,0)^BIT(parity,1)^BIT(parity,2)^BIT(parity,4))<<6;     //
	p1=(!(BIT(parity,1)^BIT(parity,3)^BIT(parity,4)^BIT(parity,5)))<<7;  //
	parity|=(p0|p1);
	return parity;
}


///***********************************************************************************
//Function:	u8 LIN_CalcChecksum(uint8 id,uint8 *data,uint8 length)
//Description: LIN calculate checksum
//Input:	
//Output:	  
//*************************************************************************************/
u8 LIN_CalcChecksum(u8 id,u8 *lindata,u8 length)
{
	u8 i;
	u16 check_sum = 0;
	//0x3C 0x3D use standard checksum
	if(id != 0x3c && id != 0x7d)  
	{// Enhanced checksum
	 	check_sum  = id ;
	} 
	else 
	{// standard checksum
	 	check_sum = 0 ;
	}
	for (i = 0; i < length; i++) 
	{    
	  	check_sum += *(lindata++);
	  
	  	if (check_sum > 0xFF)     
	  	{
	  		check_sum -= 0xFF;
	  	}
	}
	return (~check_sum);  
}




/*
*************************************************************************************
*/ 

void main()
{
	u8 i;
	
	InitSystem();
	
	LED_G_0 =0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	LED_G_0 =1;LED_G_1=1;LED_R=1;	

	INT_EnAll();
	LIN_Slave_StartFrame();

	while(1)
    {
		if(LedTime==0)
		{
			LedTime=100;
			LED_G_0 = !LED_G_0;	
		}
		if(LINFrame.State==LIN_SLAVE_INIT_DATA)
		{
			for(i = 0 ;i<8;i++)
			{
				LINFrame.Data[i] = 0x58+i ;
			}
			LINFrame.DataLength=8;
			LINFrame.Check = LIN_CalcChecksum(LINFrame.PID, &LINFrame.Data[0], LINFrame.DataLength);
			LINFrame.DataInx=1;
			UART0_LIN_SetTX();
			S0BUF=LINFrame.Data[0];
			LINFrame.State=LIN_MASTER_RX_DATA;
		}
		if(LINFrame.State==LIN_MASTER_RX_DONE)
		{
			LED_G_1=0;
			DelayXms(2);
			LED_G_1=1;
			DelayXms(2);
			LIN_Slave_StartFrame();
			
		}
		if(LINFrame.State==LIN_MASTER_TX_DONE)
		{
			// check 
			if(LINFrame.Check == LIN_CalcChecksum(LINFrame.PID, &LINFrame.Data[0], LINFrame.DataLength))
			{ // check pass
				LED_R=0;
				DelayXms(2);
				LED_R=1;
				DelayXms(2);
			}
			LIN_Slave_StartFrame();
			
		}

		if(LINFrame.State==LIN_ERR)
		{
			LIN_Slave_StartFrame();
			
		}

		if((LINFrame.State != LIN_IDLE_BREAK)&&(LINOverTime ==0))
		{
			LINFrame.State = LIN_ERR;
		}
    }

}


