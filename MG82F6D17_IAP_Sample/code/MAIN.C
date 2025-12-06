///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			IAP operation
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

#define IAP_ADDRESS    0x3800	 //IAP start Addr

idata u8 TrapFlag[3];

///*************************************************
//Function: 		char putchar (char c)   
//Description: 	send for printf
//Input:    		char c
//Output:     
//*************************************************/
char putchar (char c)   
{      
	bit bES;
	bES=ES0;
    ES0=0;        
    S0BUF = c;        
    while(TI0==0);        
    TI0=0;        
    ES0=bES;        
    return 0;
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
//Function:   	void CheckTrapFlag(void)
//Description:   	
//Input:   	
//Output:     
//*************************************************************************************/
void CheckTrapFlag(void)
{ 
	if ((TrapFlag[0]!='W')||(TrapFlag[1]!='i')||(TrapFlag[2]!='n'))
	{
		// Softwave Reset 
		POW_ResetToAP();
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
	CheckTrapFlag();
	SCMD = 0x46;
	CheckTrapFlag();
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
	CheckTrapFlag();
	SCMD = 0x46;
	CheckTrapFlag();
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA;					
}

///***********************************************************************************
//Function:u8 	IAP_ReadByte(u16 ByteAddr)
//Description:	read one byte from IAP
//Input:   
//		 		u16 ByteAddr: IAP Address
//Output:     	u8: the data from IAP 
//*************************************************************************************/
u8 IAP_ReadByte(u16 ByteAddr)
{
	bit bEA=EA;
	IFADRH = ByteAddr>>8;		// IAP Address High byte
	IFADRL= ByteAddr;			// IAP Address Low byte
	EA = 0;						
	IFMT = ISP_READ;			// Read
	ISPCR = 0x80; 				// Enable ISP/IAP
	CheckTrapFlag();
	SCMD = 0x46;
	CheckTrapFlag();
	SCMD = 0xB9;
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA; 					
	return IFD;
}

// Read IAP data by MOVC
#define	IAP_ReadByteByMOVC(x)	CBYTE[x]

///***********************************************************************************
//Function:   	void IAP_ErasePage(u8 ByteAddr)
//Description:	Erase one page 
//		It takes approximately 30 ms to erase a page
//Input:   		u8 ByteAddr: IAP Address High byte
//Output:   		
//*************************************************************************************/
void IAP_ErasePage(u8 ByteAddr)
{
	bit bEA=EA;
	IFADRH = ByteAddr;			// IAP Address High byte
	IFADRL= 0x00;				// must 0x00
	EA = 0;						
	IFMT = ISP_ERASE;			// Erase
	ISPCR = 0x80; 				// Enable ISP/IAP
	CheckTrapFlag();
	SCMD = 0x46;
	CheckTrapFlag();
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA; 					
}

///***********************************************************************************
//Function:   	void IAP_WriteByte(u16 ByteAddr,u8 ByteData)
//Description:	write one byte to IAP  
//		 It takes approximately 80 us to write a byte of data
//Input:   		u16 ByteAddr: IAP Address
//				u8 ByteData: the data to be write
//Output:   		
//*************************************************************************************/
void IAP_WriteByte(u16 ByteAddr,u8 ByteData)
{
	bit bEA=EA;
	IFD = ByteData;				// data to be write	
	IFADRH = ByteAddr>>8;		// IAP address high
	IFADRL= ByteAddr;			// IAP address low
	EA = 0;						// 
	IFMT = ISP_WRITE;			// write 
	ISPCR = 0x80; 				// Enable ISP/IAP
	CheckTrapFlag();
	SCMD = 0x46;
	CheckTrapFlag();
	SCMD = 0xB9;
	nop();
	IFMT=0;
	ISPCR = 0;					// clear
	EA = bEA; 					
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
//Function:   	void InitPort()
//Description:	Initialize IO Port
//Input:   
//Output:   		
//*************************************************************************************/
void InitPort(void)
{
	PORT_SetP3QuasiBi(BIT0|BIT1|BIT3|BIT4|BIT5);		// set P30,P31,P33,P34,P35 as Quasi-Bidirectional
	PORT_SetP1OpenDrainPu(BIT0);						// Set P10 as open-drain with pull-high for digital input
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
	TrapFlag[1]='i';
}




void main()
{
	BYTE IAPData;
	BYTE i;
	BYTE rand;
	bit bOK=TRUE;

	TrapFlag[0]='W';
	
    InitSystem();
    
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	printf("\nStart IAP DEMO!");
	LED_G_0=1;LED_G_1=1;LED_R=1;

	TrapFlag[2]='n';
	// read IAP low boundary
	i=IAP_ReadPPage(IAPLB_P);
    printf("\nRead IAPLB:%02bX",i);

	// set IAP low boundary to 0x4000,IAP size none
	IAP_WritePPage(IAPLB_P,0x40);
	i=IAP_ReadPPage(IAPLB_P);
    printf("\nNOW IAPLB:%02bX",i);
	TrapFlag[2]=0xFF;

	printf("\nOLD IAP:\n");
	i=0;
	do{
		LED_G_1=0;
		IAPData=IAP_ReadByteByMOVC(IAP_ADDRESS+i);	// read data by MOVC
		LED_G_1=1;
		printf("%02bX ",IAPData);
		if((i&0x0F)==0x0F)
		{
			printf("\n");
		}
		i++;
	}while(i!=0);
    

    while(1)
    {
    	rand++;
		LED_G_0=!LED_G_0;
	   	DelayXms(100);
		if(P10==0)
		{
			DelayXms(50);
			if(P10==0)
			{
				TrapFlag[2]='n';
				// set IAP low boundary to IAP_ADDRESS
				IAP_WritePPage(IAPLB_P,IAP_ADDRESS/256);
				LED_R=0;
				// erase page
				IAP_ErasePage(IAP_ADDRESS/256);
				LED_R=1;
				TrapFlag[2]=0xFF;

				// write data
				printf("\nWRITE IAP:\n");
				TrapFlag[2]='n';
				i=0;
				do{
					IAPData=rand+i;
					LED_G_1=0;
					IAP_WriteByte(IAP_ADDRESS+i,IAPData);
					LED_G_1=1;
					printf("%02bX ",IAPData);
					if((i&0x0F)==0x0F)
					{
						printf("\n");
					}
					i++;
				}while(i!=0);
				// set IAP low boundary to 0x4000,IAP size none
				IAP_WritePPage(IAPLB_P,0x40);
				TrapFlag[2]=0xFF;
				printf("\nNOW IAP:\n");
				i=0;
				bOK=TRUE;
				do{
					LED_G_1=0;
					IAPData=IAP_ReadByteByMOVC(IAP_ADDRESS+i);	// read data by MOVC
					LED_G_1=1;
					printf("%02bX ",IAPData);
					if((i&0x0F)==0x0F)
					{
						printf("\n");
					}
					// check
					if(IAPData != (BYTE)(rand+i))
					{
						bOK=FALSE;
					}
					i++;
				}while(i!=0);
				LED_G_1=1;LED_R=1;
				if(bOK)
				{	// PASS
					LED_G_1=0;
				}
				else
				{	// FAIL
					LED_R=0;
				}
				while(P10==0)
				{
					DelayXms(50);
				}
			}
		}
    }
}


