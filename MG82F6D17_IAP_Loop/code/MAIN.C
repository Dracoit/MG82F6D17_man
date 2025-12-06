///*********************************************************************
//    Project:MG82F6D17-DEMO
//    Author:LZD
//			MG82F6D17 SSOP20_V10 EV Board (TH194A)
//			CpuCLK=12MHz, SysCLK=12MHz
//	Description:
//			IAP operation, 
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


#define ISP_START_ADDRESS    0x3C00	 //ISP start Addr
#define IAP_START_ADDRESS    0x3800	 //IAP start Addr  
#define IAP_END_ADDRESS		 ISP_START_ADDRESS

#define IAP_ERR_RETRY_CNT_MAX	5	// retry max

idata u8 TrapFlag[3];


#define IAP_SYS_PARA_FLAG	0xA5
#define	SYS_PARA_ADDR		(IAP_START_ADDRESS+0)

// Sys para
typedef struct{
	u16 Para1;			// 
	u32 Para2;			// 
	u8 Para3;
	u8 Flag;			// 0xA5 
}SysParaDef; 


typedef union 
{ 
	u8 BUF[sizeof(SysParaDef)];
	SysParaDef B;
}SysParaTypeDef;
xdata SysParaTypeDef SysPara;	// Systerm para	


void ReadSysParaFromIAP(void);
u8 SaveSysParaToIAP(void);
void RestoreSysPara(void);

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
	PORT_SetP2PushPull(BIT2|BIT4|BIT6);					// Set P22,P24,P26 as push-pull, for LED
	PORT_SetP1OpenDrainPu(BIT0|BIT1);						// Set P10,P11 as open-drain with pull-high for digital input
	P10=1;
	P11=1;
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
	u8 i;

	TrapFlag[0]='W';
	
    InitSystem();
    
	LED_G_0=0;LED_G_1=0;LED_R=0;
	DelayXms(1000);
	printf("\nStart IAP DEMO!");
	LED_G_0=1;LED_G_1=1;LED_R=1;

	TrapFlag[1]='i';
	TrapFlag[2]='n';
	//read IAP low boundary
	i=IAP_ReadPPage(IAPLB_P);
    printf("\nRead IAPLB:%02bX",i);

	// set IAP low boundary to IAP_END_ADDRESS,IAP size none
	IAP_WritePPage(IAPLB_P,IAP_END_ADDRESS/256);
	i=IAP_ReadPPage(IAPLB_P);
    printf("\nNOW IAPLB:%02bX  ",i);
	TrapFlag[1]=0xFF;
	TrapFlag[2]=0xFF;

	
    ReadSysParaFromIAP();
	printf("\nRead SysPara: ");
	for(i=0;i<(sizeof(SysParaDef));i++)
	{
		printf("%02bX ",SysPara.BUF[i]);
	}

	printf("\nWrite cnt:%ld",SysPara.B.Para2);

    while(1)
    {
    	SysPara.B.Para1++;
		LED_G_0=!LED_G_0;
	   	DelayXms(100);
		if(P10==0)
		{
			// Check key press,Debouce
			i=0;
			do{
				DelayXus(100);
				if(P10==0)
				{
					i++;
				}
				else
				{
					break;
				}
			}while (i<200);

			if(i>=200)
			{
				RestoreSysPara();
				TrapFlag[1]='i';
				printf("\nSave SysPara: ");
				for(i=0;i<(sizeof(SysParaDef));i++)
				{
					printf("%02bX ",SysPara.BUF[i]);
				}
				LED_G_1=1;LED_R=1;
				if(SaveSysParaToIAP()==0)
				{ // success
					printf(" ---OK!");
					LED_G_1=0;
				}
				else
				{ // fail
					printf(" ---Fail!");
					LED_R=0;
					while(1)
					{
						CLRWDT();
					}
				}
				TrapFlag[1]=0xFF;

				// check key release
				i=0;
				do{
					DelayXus(100);
					if(P10==1)
					{
						i++;
					}
					else
					{
						i=0;
					}
				}while (i<200);
			}
		}



		if(P11==0)
		{
			i=0;
			do{
				DelayXus(100);
				if(P11==0)
				{
					i++;
				}
				else
				{
					break;
				}
			}while (i<200);
		
			if(i>=200)
			{
				SysPara.B.Para2++;
				TrapFlag[1]='i';
				printf("\nSave SysPara: ");
				for(i=0;i<(sizeof(SysParaDef));i++)
				{
					printf("%02bX ",SysPara.BUF[i]);
				}
				LED_G_1=1;LED_R=1;
				if(SaveSysParaToIAP()==0)
				{ // Success
					printf(" ---OK!");
					LED_G_1=0;
				}
				else
				{ // Fail
					printf(" ---Fail!");
					LED_R=0;
					while(1)
					{
						CLRWDT();
					}
				}
				TrapFlag[1]=0xFF;
		
			}
		}

    }

}

///***********************************************************************************
//Function:       void ReadSysParaFromIAP(void)
//Description:    Read system para from IAP
//Input:   
//Output:     
//*************************************************************************************/
void ReadSysParaFromIAP(void)
{
	u8 i;
	u16 wAddr;
	wAddr=0;
	for(i=0;i<(512/(sizeof(SysParaDef)));i++)
	{//Find valid data tags
		if(CBYTE[(SYS_PARA_ADDR)+(sizeof(SysParaDef)-1)+wAddr]==IAP_SYS_PARA_FLAG)
		{// A valid system parameter marker was found
			break;
		}
		wAddr=wAddr+(sizeof(SysParaDef));
	}
	if(i<(512/(sizeof(SysParaDef))))
	{ // Finded valid data tags
		for(i=0;i<(sizeof(SysParaDef));i++)
		{ // Read system parameters
			SysPara.BUF[i]= CBYTE[(SYS_PARA_ADDR)+wAddr+i];
		}
	}
	else
	{ // If no valid system parameters exist, reset
		RestoreSysPara();
	}
}


///***********************************************************************************
//Function:       void SaveSysParaToIAP(void)
//Description:    Save system para to IAP
//Input:   
//Output: 0->success,other->fail    
//*************************************************************************************/
u8 SaveSysParaToIAP(void)
{
	u8 i,l,ErrCnt;
	u16 wAddr;
	bit bWrite;
	bit bErr;

	SysPara.B.Flag=IAP_SYS_PARA_FLAG;
	ErrCnt=0;
IAP_ERR_RETRY:
	if(ErrCnt>=IAP_ERR_RETRY_CNT_MAX) return ErrCnt;
	ErrCnt++;
	bErr=FALSE;
	bWrite=FALSE;
	TrapFlag[2]='n';
	wAddr=0;
	for(i=0;i<(512/(sizeof(SysParaDef)));i++)
	{ // Find valid data tags
		if(CBYTE[(SYS_PARA_ADDR)+(sizeof(SysParaDef)-1)+wAddr]==IAP_SYS_PARA_FLAG)
		{
			break;
		}
		wAddr=wAddr+(sizeof(SysParaDef));
	}
	if(i<(512/(sizeof(SysParaDef))))
	{ // Finded valid data tags

		for(i=0;i<sizeof(SysParaDef);i++)
		{
			wAddr++;
			if(CBYTE[(SYS_PARA_ADDR)+wAddr]!=SysPara.BUF[i])
			{
				// Different from the previously saved data, the data needs to be written to the FLASH flag
				bWrite=TRUE;
				break;
			}
		}
	}
	else
	{ // No find
		bWrite=TRUE;
	}
	
	if(bWrite==TRUE)
	{
        // Set IAPLB
		IAP_WritePPage(IAPLB_P,IAP_START_ADDRESS>>8);
		
        // Write data to FLASH
		wAddr=0;
		for(i=0;i<(512/(sizeof(SysParaDef)));i++)
		{ 
			// Find data segments are all empty
			for(l=0;l<(sizeof(SysParaDef));l++)
			{
				if(CBYTE[(SYS_PARA_ADDR)+wAddr+l]!=0xFF) 
				{
					break;
				}
				
			}
			if(l<(sizeof(SysParaDef)))
			{
				// Not empty
				IAP_WriteByte((SYS_PARA_ADDR)+(sizeof(SysParaDef)-1)+wAddr, 0x00);
			}
			else
			{  // Empty
								
                // Write data to FLASH
				for(l=0;l<sizeof(SysParaDef);l++)
				{
					IAP_WriteByte((SYS_PARA_ADDR)+wAddr+l, SysPara.BUF[l]);
				}
				break;
			}
			wAddr=wAddr+(sizeof(SysParaDef));
		}
		if(i<(512/(sizeof(SysParaDef))))
		{
			// Space has been found and data has been written to FLASH
		}
		else
		{
			// No space
			wAddr=0x0000;
			// Erase FLASH
			IAP_ErasePage(SYS_PARA_ADDR>>8);
            // Write data to FLASH
			for(l=0;l<sizeof(SysParaDef);l++)
			{
				IAP_WriteByte((SYS_PARA_ADDR)+l, SysPara.BUF[l]);
			}
		}
		// Reset IAPLB
		IAP_WritePPage(IAPLB_P,IAP_END_ADDRESS>>8);

		// Verify	
		for(l=0;l<(sizeof(SysParaDef));l++)
		{
			if(CBYTE[(SYS_PARA_ADDR)+wAddr+l]!= SysPara.BUF[l]) 
			{
				bErr=TRUE;
				break;
			}
			
		}
		
	}
	
	TrapFlag[2]=0xFF;

	if(bErr==TRUE) goto IAP_ERR_RETRY;

	return 0;
	
}

///***********************************************************************************
//Function:       void RestoreSysPara(void)
//Description:    reset System para
//Input:   
//Output:     
//*************************************************************************************/
void RestoreSysPara(void)
{
	SysPara.B.Para1=0x0000;
	SysPara.B.Para2=0x00000000;
	SysPara.B.Para3=0x00;
	SysPara.B.Flag=IAP_SYS_PARA_FLAG;
}



