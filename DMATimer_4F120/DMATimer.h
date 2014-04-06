// DMATimer.h
// Runs on LM4F120
// Periodic timer triggered DMA transfer
// Uses Timer5A to trigger the DMA, read from an 8-bit PORT, and then write to a memory Buffer
// There is a Timer5A interrupt after the buffer is full
// Jonathan Valvano
// January 1, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2012
   Section 6.4.5

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

#define MS 1000

// ************DMA_Init*****************
// Initialize the Port to memory transfer, triggered by timer 5A
// This needs to be called once before requesting a transfer
// The source address is fixed, destination address incremented each byte
// Inputs:  period in usec
// Outputs: none
void DMA_Init(unsigned short period);

// ************DMA_Transfer*****************
// Called to transfer bytes from source to destination
// The source address is fixed, destination address incremented each byte
// Inputs:  source is a pointer to the 32-bit I/O, least significant byte to read
//          destination is a pointer to a place to put the copy
//          count is the number of bytes to transfer (max is 1024 words)
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(volatile unsigned long *source, unsigned char *destination, unsigned long count);

// ************DMA_Status*****************
// Can be used to check the status of a previous request
// Inputs:  none
// Outputs: true if still active, false if complete
unsigned long DMA_Status(void); 
