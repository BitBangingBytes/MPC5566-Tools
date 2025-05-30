# handlers.s - Decrementer (IVOR10) interrupt example
# Rev 1.0: Sept 2, 2004, S Mihalik,
# Rev 1.1: May 15, 2006 S.M.- Use GPIO[195] instead of GPIO[205]
# Rev 1.2: Jun 13, 2007 S.M. Added .section .ivor_handlers for linking
#                       and changed GPIO address to global variable
# Rev 1.3  Nov  1 2008 SM - Used r4 instead of r0 to clear DIS flag in spr TSR
# Copyright Freescale Semiconductor, Inc. 2007. All rights reserved
 
# STACK FRAME DESIGN: Depth: 4 words (0x10, or 16 bytes)
#            ************* ______________
#   0x0C     *    r4     *    |	GPR Save Area
#   0x08     *    r3     * ___|__________
#   0x04     * resvd- LR * Reserved for calling function 
#   0x00     *  SP(GPR1) * Backchain (same as gpr1 in GPRs)
#            ************* 

	.globl IVOR10Handler
  .extern DECctr            # Counter of Decrementer interrupts taken              
  .extern GPDI_83_ADDR     # GPDI[83] register address   
	.extern GPDO_83_ADDR     # GPDO[83] register address     

  .section .ivor_handlers  

#                  # Align IVOR handlers on a 16 byte boundary for MPC555x
# .align 4		  	 # GHS, Cygnus, Diab(default) use .align 4
  .align 16			   # Metrowerks requires .align 16
                	
 
IVOR10Handler:

prolog:									# PROLOGUE 
	stwu	r1, -0x10 (r1)				# Create stack frame and store back chain
	stw		r3,  0x08 (r1)
	stw	 	r4,  0x0C (r1)	
	 
DECisr:
	lis	 	r3, DECctr@ha	   			# Read DECctr
	lwz	  	r4, DECctr@l (r3)
	addi	r4, r4, 0x1					# Increment DECctr
	stw		r4, DECctr@l (r3)			# Write back new DECctr

    lis		r3, GPDI_83_ADDR@ha		# Get pointer to memory containing GPDI[114] address	
	lwz		r3, GPDI_83_ADDR@l(r3) 
	lbz		r4, 0 (r3)                  # Read pin input state from register GPDI[114] 
	addi	r4, r4, 1					# Add one to state for toggle effect
	lis		r3, GPDO_83_ADDR@ha        # Get pointer to memory containing GPDO[114] address
	lwz 	r3, GPDO_83_ADDR@l(r3)
	stb		r4, 0 (r3)                  # Output toggled GPIO 114 pin state to reg. GPDO[114]

	lis	 	r4, 0x0800					# Write "1" clear Dec Interrupt Status (DIS) flag
	mtspr 	336, r4						# DIS flag is in spr TSR (spr 336)

epilog:							        # EPILOGUE
	lwz	 	r4,  0x0C (r1)				# Restore gprs
	lwz	  	r3,  0x08 (r1)
	addi  	r1, r1, 0x10	  			# Restore space on stack
	rfi						      	    # End of Interrupt

			