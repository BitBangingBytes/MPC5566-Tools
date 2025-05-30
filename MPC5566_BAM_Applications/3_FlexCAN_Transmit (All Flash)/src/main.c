/* main.c: FlexCAN program */
/* Description: Transmit one message from FlexCAN A buf. 0 to FlexCAN C buf. 4 */
/* Rev 0.1 Jan 16, 2006 S.Mihalik, Copyright Freescale, 2006. All Rights Reserved */
/* Rev 0.2 Jun 6 2006 SM - changed Flexcan A to C & enabled 64 msg buffers */
/* Rev 0.3 Jun 15 2006 SM - 1. Made globals uninitialized */
/*         2. RecieveMsg function:  read CANx_TIMER, removed setting buffer's CODE*/
/*         3. added idle loop code for smoother Nexus trace */
/*         4. modified for newer Freescale header files (r 16) */
/* Rev 0.4 Aug 11 2006 SM - Removed redundant CAN_A.MCR init */
/* Rev 0.5 Jan 31 2007 SM - Removed other redundant CAN_C.MCR init */
/* Rev 0.6 Mar 08 2007 SM - Correced init of MBs - cleared 64 MBs, instead of 63 */
/* Rev 0.7 Jul 20 2007 SM - Changes for MPC5510 */
/* Rev 0.8 May 15 2008 SM - Changes for new MPC5510 header file symbols */
/* Rev 1.0 Jul 10 2009 SM - Cleared CAN Msg Buf flag by writing to reg. not bit, */
/*   increased Tx pads slew rate, changed RxCODE, RxLENGTH, dummy data types and */
/*   init receiving CAN first to allow CAN bus sync time before receiving first msg*/
/* Notes:  */
/*  1. MMU not initialized; must be done by debug scripts or BAM */
/*  2. SRAM not initialized; must be done by debug scripts or in a crt0 type file */

#include "mpc5566.h" /* Use proper include file such as mpc5510.h or mpc5554.h */


void initCAN_A (void) {
  uint8_t   i;

  CAN_A.MCR.R = 0x5000003F;       /* Put in Freeze Mode & enable all 64 message buffers */
  //CAN_A.CR.R = 0x04DB0006;        /* Configure for 8MHz OSC, 100KHz bit time */
  CAN_A.CR.R = 0x00000000;        /* Clear register */
  CAN_A.CR.B.PRESDIV = 4;		
  CAN_A.CR.B.RJW = 1;
  CAN_A.CR.B.PROPSEG = 2;
  CAN_A.CR.B.PSEG1 = 1;
  CAN_A.CR.B.PSEG2 = 1;
  
  for (i=0; i<64; i++) {
    CAN_A.BUF[i].CS.B.CODE = 0;   /* Inactivate all message buffers */
  } 
  CAN_A.BUF[0].CS.B.CODE = 8;     /* Message Buffer 0 set to TX INACTIVE */
  SIU.PCR[83].R = 0x062C;         /* MPC555x: Configure pad as CNTXA, open drain */
  SIU.PCR[83].B.ODE = 0;		  // Disable Open Drain - Push/Pull
  SIU.PCR[84].R = 0x0500;         /* MPC555x: Configure pad as CNRXA */
  CAN_A.MCR.R = 0x0000003F;       /* Negate FlexCAN A halt state for 64 MB */
}

void TransmitMsg (void) {
  uint8_t	i;
                                   /* Assumption:  Message buffer CODE is INACTIVE */
  const uint8_t TxData[] = {"Hello"};  /* Transmit string*/
  CAN_A.BUF[0].CS.B.IDE = 0;           /* Use standard ID length */
  CAN_A.BUF[0].ID.B.STD_ID = 555;      /* Transmit ID is 555 */
  CAN_A.BUF[0].CS.B.RTR = 0;           /* Data frame, not remote Tx request frame */
  CAN_A.BUF[0].CS.B.LENGTH = sizeof(TxData) -1 ; /* # bytes to transmit w/o null */
  for (i=0; i<sizeof(TxData); i++) {
    CAN_A.BUF[0].DATA.B[i] = TxData[i];      /* Data to be transmitted */
  }
  CAN_A.BUF[0].CS.B.SRR = 1;           /* Tx frame (not req'd for standard frame)*/
  CAN_A.BUF[0].CS.B.CODE =0xC;         /* Activate msg. buf. to transmit data frame */ 
}

void Transmit8Bytes (uint16_t messageID, uint8_t* TxData) {
  uint8_t	i;
                                   /* Assumption:  Message buffer CODE is INACTIVE */
  CAN_A.BUF[0].CS.B.IDE = 0;           /* Use standard ID length */
  CAN_A.BUF[0].ID.B.STD_ID = messageID;      /* Transmit ID */
  CAN_A.BUF[0].CS.B.RTR = 0;           /* Data frame, not remote Tx request frame */
  CAN_A.BUF[0].CS.B.LENGTH = 8;
  for (i=0; i<8; i++) {
    CAN_A.BUF[0].DATA.B[i] = TxData[i];      /* Data to be transmitted */
  }
  CAN_A.BUF[0].CS.B.SRR = 1;           /* Tx frame (not req'd for standard frame)*/
  CAN_A.BUF[0].CS.B.CODE =0xC;         /* Activate msg. buf. to transmit data frame */ 
}


void sendFlash(uint8_t* flash_address, uint32_t length, uint8_t* TxData, uint16_t messageID) {

	uint32_t i = 0;
	uint32_t IdleCounter = 0;
	for(i = 0; i < length; i++) {
		if(flash_address + i) 
		{
			TxData[i%8] = *(flash_address + i);			
		}
		else 
		{
			TxData[i%8] = 0xFF;			
		}

		if( (i%8) == 7) {
			Transmit8Bytes(messageID, TxData);
			IdleCounter = 0;
			while(IdleCounter < 0x400) 
			{
				IdleCounter++;
				clr_wd_next();
			}
		}
	}
	Transmit8Bytes((messageID + 1), TxData);
	IdleCounter = 0;
	while(IdleCounter < 0x400) 
	{
		IdleCounter++;
		clr_wd_next();
	}
}

void main(void) {
  volatile uint32_t IdleCtr = 0;
  // Define start address and length for each memory array to be dumped
  uint8_t* FlashMain = (uint8_t *)0x00000000;
  uint8_t* FlashShadow = (uint8_t *)0x00FFFC00;  
  uint8_t* FlashBAM = (uint8_t *)0xFFFFC000;
  uint32_t  FlashMainSize = 0x300000;
  uint32_t  FlashShadowSize = 0x400;
  uint32_t  FlashBAMSize = 0x4000;
  uint8_t   bytes[8];
  uint32_t i = 0;
  
  initCAN_A();             /* Initialize FlexCAN A & one of its buffers for transmit*/
  
  sendFlash(FlashBAM, FlashBAMSize, bytes, 0x100);
  sendFlash(FlashShadow, FlashShadowSize, bytes, 0x200);
  sendFlash(FlashMain, FlashMainSize, bytes, 0x300);
  
  while (1) {              /* Idle loop: increment counter */
    IdleCtr++;
    clr_wd_next();

    
    if ((IdleCtr % 10000000) == 0) 
    {
    	TransmitMsg();
    }
  }
}
