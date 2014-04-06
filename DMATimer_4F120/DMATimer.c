// DMATimer.c
// Runs on LM4F120
// Periodic timer triggered DMA transfer
// Uses Timer5A to trigger the DMA, read from an 8-bit PORT, and then write to a memory Buffer
// There is a Timer5A interrupt after the buffer is full
// Jonathan Valvano
// January 25, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2012
   Section 6.4.5, Program 6.1

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "lm4f120h5qr.h"

// The control table used by the uDMA controller.  This table must be aligned to a 1024 byte boundary.
unsigned long ucControlTable[256] __attribute__ ((aligned(1024)));
// Timer5A uses uDMA channel 8 encoding 3
#define CH8 (8*4)
#define BIT8 0x00000100

// ***************** Timer5A_Init ****************
// Activate Timer5A trigger DMA periodically
// Inputs:  period in usec
// Outputs: none
void Timer5A_Init(unsigned short period){ volatile unsigned long Delay;
  SYSCTL_RCGCTIMER_R |= 0x20;      // 0) activate timer5
  Delay = 0;                       // wait for completion
  TIMER5_CTL_R &= ~0x00000001;     // 1) disable timer5A during setup
  TIMER5_CFG_R = 0x00000004;       // 2) configure for 16-bit timer mode
  TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
  TIMER5_TAILR_R = period-1;       // 4) reload value
  TIMER5_TAPR_R = 49;              // 5) 1us timer5A
  TIMER5_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
  TIMER5_IMR_R |= 0x00000001;      // 7) arm timeout interrupt
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000040; // 8) priority 2
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 19 in NVIC
  // vector number 108, interrupt number 92
  TIMER5_CTL_R |= 0x00000001;      // 10) enable timer5A
// interrupts enabled in the main program after all devices initialized
}
unsigned long Count=0;
void Timer5A_Handler(void){ // interrupts after each block is transferred
  TIMER5_ICR_R = TIMER_ICR_TATOCINT; // acknowledge timer5A timeout
  Count++;
}
// ************DMA_Init*****************
// Initialize the PortF to memory transfer, triggered by timer 5A
// This needs to be called once before requesting a transfer
// The source address is fixed, destination address incremented each byte
// Inputs:  period in usec
// Outputs: none
void DMA_Init(unsigned short period){  int i;
  volatile unsigned long delay; 
  for(i=0; i<256; i++){
    ucControlTable[i] = 0;
  }
  SYSCTL_RCGCDMA_R = 0x01;    // µDMA Module Run Mode Clock Gating Control
  delay = SYSCTL_RCGCDMA_R;   // allow time to finish 
  UDMA_CFG_R = 0x01;          // MASTEN Controller Master Enable
  UDMA_CTLBASE_R = (unsigned long)ucControlTable;
  UDMA_CHMAP1_R = (UDMA_CHMAP1_R&0xFFFFFFF0)|0x00000003;  // timer5A
  UDMA_PRIOCLR_R = BIT8;     // default, not high priority
  UDMA_ALTCLR_R = BIT8;      // use primary control
  UDMA_USEBURSTCLR_R = BIT8; // responds to both burst and single requests
  UDMA_REQMASKCLR_R = BIT8;  // allow the µDMA controller to recognize requests for this channel
  Timer5A_Init(period);
}
// ************DMA_Transfer*****************
// Called to transfer bytes from source to destination
// The source address is fixed, destination address incremented each byte
// Inputs:  source is a pointer to the 32-bit I/O, least significant byte to read
//          destination is a pointer to a place to put the copy
//          count is the number of bytes to transfer (max is 1024 words)
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(volatile unsigned long *source, unsigned char *destination, unsigned long count){ 
  ucControlTable[CH8]   = (unsigned long)source;                 // first and last address
  ucControlTable[CH8+1] = (unsigned long)destination+count-1;    // last address
  ucControlTable[CH8+2] = 0x0C000001+((count-1)<<4);             // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   0     8-bit destination address increment
   DSTSIZE           29:28   0     8-bit destination data size
   SRCINC            27:26   3     no source address increment
   SRCSIZE           25:24   0     8-bit source data size
   reserved          23:18   0     Reserved  
   ARBSIZE           17:14   0     Arbitrates after 1 transfer
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0      N/A for this transfer type
   XFERMODE          2:0     1      Use basic transfer mode
  */
  UDMA_ENASET_R = BIT8;  // µDMA Channel 8 is enabled.
  // bit 8 in UDMA_ENASET_R become clear when done
  // bits 2:0 ucControlTable[CH8+2] become clear when done
}

// ************DMA_Status*****************
// Can be used to check the status of a previous request
// Inputs:  none
// Outputs: true if still active, false if complete
unsigned long DMA_Status(void){ 
  return (UDMA_ENASET_R&BIT8);  // µDMA Channel 8 enable bit is high if active
}

