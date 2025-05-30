0_Decrementer (WORKED)
  - First program I got to work, the key was initializing all of the memory space with 00's.
  - This was done by noting the memory space used in the linker file then on the glitching machine
  - creating a loader file that full size initialized with all 00's then loading the binary
  - from CodeWarrior into that zero initialized binary. When it's sent to the MPC5566's RAM via BAM
  - bootloader then it will run. If memory isn't initialized with 00's first, ECC checking causes the
  - processor to halt and your program will never start.
  - Also getting the linker file figured out, adding the __start.c file and getting __start.o loaded at
  - the start of memory where the BAM jumps to after loading the program.

1_FlexCAN_Transmit (Message)
  - This program when loaded causes a message to be sent from the CAN bus at 400kHz, no flash memory is
  - accessed so it will run even if you enter the BAM but Flash is locked (which happened with a EMFI glitch
  - that got me BAM access but flash was locked.)
1_FlexCAN_Transmit (Shadow)
  - This program when loaded reads the Shadow memory region where the password is located and sends it via CAN
  - bus at 400kHz as 2 packets. Arbitration ID is 0x22B, size is 4 bytes. If the flash memory is locked this
  - program will crash, it does not have any protection or provide any output if reading memory was unsuccessful.
2_FlexCAN_Transmit (Shadow) - Size Optimized
  - This program is the same as the above but optimized so the binary to load is 0x1700 instead of 0xD000 which 
  - is what the two above are. At 19,200 baud serial this program will load in 9.5 seconds.
3_FlexCAN_Transmit (All Flash)	- Transmit BAM, Shadow and Main Flash sections each with a unique Arbitration ID 
				- (0x100, 0x200, 0x300). At the end of each section the Arbitration ID is incremented
				- by 1 (0x101, 0x201, 0x301) as a way for the receiving application to know it has 
				- reached the end. This final packet should be discarded.
				- This program will crash if Flash can't be accessed or if there are ECC errors.
				- Future improvements would be to handle these cases.