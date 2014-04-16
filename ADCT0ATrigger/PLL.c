// PLL.c
// Runs on LM4F120
// A software function to change the bus frequency using the PLL.
// Daniel Valvano
// October 4, 2012

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2012
   Program 2.10, Figure 2.37

 Copyright 2012 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include "PLL.h"

// The two #define statements SYSDIV and LSB in PLL.h
// initialize the PLL to the desired frequency.

// bus frequency is 400MHz/(2*SYSDIV+1+LSB) = 400MHz/(2*2+1+0) = 80 MHz
// see the table at the end of this file

#define SYSCTL_RIS_R            (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RIS_PLLLRIS      0x00000040  // PLL Lock Raw Interrupt Status
#define SYSCTL_RCC_R            (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC_XTAL_M       0x000003C0  // Crystal Value
#define SYSCTL_RCC_XTAL_6MHZ    0x000002C0  // 6 MHz Crystal
#define SYSCTL_RCC_XTAL_8MHZ    0x00000380  // 8 MHz Crystal
#define SYSCTL_RCC_XTAL_16MHZ   0x00000540  // 16 MHz Crystal
#define SYSCTL_RCC2_R           (*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RCC2_USERCC2     0x80000000  // Use RCC2
#define SYSCTL_RCC2_DIV400      0x40000000  // Divide PLL as 400 MHz vs. 200
                                            // MHz
#define SYSCTL_RCC2_SYSDIV2_M   0x1F800000  // System Clock Divisor 2
#define SYSCTL_RCC2_SYSDIV2LSB  0x00400000  // Additional LSB for SYSDIV2
#define SYSCTL_RCC2_PWRDN2      0x00002000  // Power-Down PLL 2
#define SYSCTL_RCC2_BYPASS2     0x00000800  // PLL Bypass 2
#define SYSCTL_RCC2_OSCSRC2_M   0x00000070  // Oscillator Source 2
#define SYSCTL_RCC2_OSCSRC2_MO  0x00000000  // MOSC

// configure the system to get its clock from the PLL
void PLL_Init(void){
  // 1) configure the system to use RCC2 for advanced features
  //    such as 400 MHz PLL and non-integer System Clock Divisor
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
  // 2) bypass PLL while initializing
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
  // 3) select the crystal value and oscillator source
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;// configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;// clear oscillator source field
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;// configure for main oscillator source
  // 4) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
  // 5) use 400 MHz PLL
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;
  // 6) set the desired system divider and the system divider least significant bit
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2_M;  // clear system clock divider field
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2LSB; // clear bit SYSDIV2LSB

// set SYSDIV2 and SYSDIV2LSB fields
  SYSCTL_RCC2_R += (SYSDIV<<23)|(LSB<<22);  // divide by (2*SYSDIV+1+LSB)

  // 7) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
  // 8) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

/*
SYSDIV    LSB     Divisor    Clock (MHz)
 0        0        1         reserved 
 0        1        2         reserved
 1        0        3         reserved
 1        1        4         reserved
 2        0        5         80.000
 2        1        6         66.667
 3        0        7         reserved
 3        1        8         50.000
 4        0        9         44.444
 4        1        10        40.000
 5        0        11        36.364
 5        1        12        33.333
 6        0        13        30.769
 6        1        14        28.571
 7        0        15        26.667
 7        1        16        25.000
 8        0        17        23.529
 8        1        18        22.222
 9        0        19        21.053
 9        1        20        20.000
10        0        21        19.048
10        1        22        18.182
11        0        23        17.391
11        1        24        16.667
12        0        25        16.000
12        1        26        15.385
13        0        27        14.815
13        1        28        14.286
14        0        29        13.793
14        1        30        13.333
15        0        31        12.903
15        1        32        12.500
16        0        33        12.121
16        1        34        11.765
17        0        35        11.429
17        1        36        11.111
18        0        37        10.811
18        1        38        10.526
19        0        39        10.256
19        1        40        10.000
20        0        41        9.756
20        1        42        9.524
21        0        43        9.302
21        1        44        9.091
22        0        45        8.889
22        1        46        8.696
23        0        47        8.511
23        1        48        8.333
24        0        49        8.163
24        1        50        8.000
25        0        51        7.843
25        1        52        7.692
26        0        53        7.547
26        1        54        7.407
27        0        55        7.273
27        1        56        7.143
28        0        57        7.018
28        1        58        6.897
29        0        59        6.780
29        1        60        6.667
30        0        61        6.557
30        1        62        6.452
31        0        63        6.349
31        1        64        6.250
32        0        65        6.154
32        1        66        6.061
33        0        67        5.970
33        1        68        5.882
34        0        69        5.797
34        1        70        5.714
35        0        71        5.634
35        1        72        5.556
36        0        73        5.479
36        1        74        5.405
37        0        75        5.333
37        1        76        5.263
38        0        77        5.195
38        1        78        5.128
39        0        79        5.063
39        1        80        5.000
40        0        81        4.938
40        1        82        4.878
41        0        83        4.819
41        1        84        4.762
42        0        85        4.706
42        1        86        4.651
43        0        87        4.598
43        1        88        4.545
44        0        89        4.494
44        1        90        4.444
45        0        91        4.396
45        1        92        4.348
46        0        93        4.301
46        1        94        4.255
47        0        95        4.211
47        1        96        4.167
48        0        97        4.124
48        1        98        4.082
49        0        99        4.040
49        1        100       4.000
50        0        101       3.960
50        1        102       3.922
51        0        103       3.883
51        1        104       3.846
52        0        105       3.810
52        1        106       3.774
53        0        107       3.738
53        1        108       3.704
54        0        109       3.670
54        1        110       3.636
55        0        111       3.604
55        1        112       3.571
56        0        113       3.540
56        1        114       3.509
57        0        115       3.478
57        1        116       3.448
58        0        117       3.419
58        1        118       3.390
59        0        119       3.361
59        1        120       3.333
60        0        121       3.306
60        1        122       3.279
61        0        123       3.252
61        1        124       3.226
62        0        125       3.200
62        1        126       3.175
63        0        127       3.150
63        1        128       3.125

*/
