//------------------------------------------------------------------------------
//
//  Description: This file contains the Switch Control
//
//
//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

// Function Prototypes
void Init_Switches(void);
void enable_switch_SW1(void);
void enable_switch_SW2(void);
void Switches_Process(void);
void lcd_180(void);


//------------------------------------------------------------------------------
// Switch Configurations
extern char display_line[4][11];
extern char *display[4];
char pwm_state;
extern char change;
char run_wheels;
extern volatile unsigned char display_changed;
extern unsigned int moving;         //Variable Tyler created
extern char start_moving;           //Variable Tyler created


void Switches_Process(void){
//------------------------------------------------------------------------------
  if (!(P4IN & SW1)){
    strcpy(display_line[0], "Fall  2023");
    strcpy(display_line[1], "   NCSU   ");
    strcpy(display_line[2], " WOLFPACK ");
    strcpy(display_line[3], "  ECE306  ");
    display_changed = TRUE;
    start_moving = 1;
  }

  if (!(P2IN & SW2)) {
    strcpy(display_line[0], " Embedded ");
    strcpy(display_line[1], "  System  ");
    strcpy(display_line[2], "   Rocks  ");
    strcpy(display_line[3], "  GP I/O  ");
    display_changed = TRUE;

  }
//------------------------------------------------------------------------------
}

void forward(void) {
    if (!(L_FORWARD)) {
        P6OUT |= L_FORWARD;
    }
    if (!(R_FORWARD)) {
        P6OUT |= R_FORWARD;
    }
}

void stop(void) {
    P6OUT &= ~L_FORWARD;
    P6OUT &= ~R_FORWARD;
}

