/* main.c - Decrementer interrupt example */
/* Rev 1.0 April 19, 2004 S.Mihalik, Copyright Freescale, 2007 All Rights Reserved */
/* Rev 1.1 May 15 2006 SM- Changed GPIO205 to GPIO195 for use in 324, 208 packages */ 
/* Rev 1.2 Aug 12 2006 SM- Made variable i volatile */
/* Rev 1.3 Jun 13 2007 SM- Passed spr IVPR value from link file, changed GPIO pin */
/* Notes:  */
/*  1. MMU not initialized; must be done by debug scripts or BAM */
/*  2. SRAM not initialized; must be done by debug scripts or in a crt0 type file */

#include "mpc5566.h" /* Use proper include file like mpc5510.h, mpc5554.h, mpc563m.h */

extern IVOR10Handler();
extern uint32_t __IVPR_VALUE;	 /* Interrupt Vector Prefix value from link file */

  uint32_t DECctr = 0;	   /* Counter for Decrementer interrupts */
  vuint32_t GPDI_83_ADDR = (uint32_t) &SIU.GPDI[83].R; /* GPDI[83] reg. addr. */
  vuint32_t GPDO_83_ADDR = (uint32_t) &SIU.GPDO[83].R; /* GPDO[83] reg. addr. */

asm void initIrqVectors(void) {
  lis       r0, __IVPR_VALUE@h    /* IVPR value is passed from link file */
  ori       r0, r0,__IVPR_VALUE@l /* Note: IVPR lower bits are unused in MPC555x */ 
  mtivpr	r0
/* The following two lines are required for MPC555x, and are unuseed for MPC551x */
  li		r0, IVOR10Handler@l	/* IVOR10(Dec) = lower half of handler address */							
  mtivor10	r0						
}

asm void initDEC(void) {
  lis		r0, 0x00B7		/* Load initial DEC value of 12M (0x00B7 1B00) */
  ori		r0, r0, 0x1B00
  mtdec		r0
  mtdecar	r0				/* Load same initial value to DECAR	 */
  lis		r0, 0x0440		/* Enable DEC interrupt and auto-reload	*/
  mttcr		r0			
  li		r0, 0x4000		/* Enable Time Base and Decrementer (set TBEN) */
  mthid0	r0
}

void main (void) {	
  volatile uint32_t i=0;    /* Dummy idle counter */
  initIrqVectors();			/* Initialize interrupt vectors registers*/
  initDEC();				/* Initialize Decrementer routine */
  SIU.PCR[83].R= 0x0303;	/* Init. pin for GPIO output that can be read as input */
  asm (" wrteei 1");		/* Enable external interrupts (INTC, DEC, FIT) */
  while (1) { i++; }		/* Loop forever */
}
