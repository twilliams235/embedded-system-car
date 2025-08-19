/*
 * init.c
 *
 *  Created on: Sep 14, 2023
 *      Author: tylerwilliams
 */


#include  "msp430.h"
#include  <string.h>
#include  "Compiled/functions.h"
#include  "Compiled/LCD.h"
#include  "Compiled/ports.h"
#include  "Compiled/macros.h"
#include  "Compiled/timers.h"



volatile char slow_input_down;
extern char display_line[4][11];
extern char *display[4];
unsigned char display_mode;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;
extern volatile unsigned int Time_Sequence;
extern volatile char one_time;
unsigned int test_value;
char chosen_direction;
char change;

unsigned int wheel_move;
char forward;

unsigned int Last_Time_Sequence;   //Variable to identify Time_Sequence has changed
unsigned int cycle_time;            // time base used to control making shapes
unsigned int time_change;           // an identifier that a change has occurred

unsigned int DAC_data;

void Init_Conditions(void){
//------------------------------------------------------------------------------

  int i;
  for(i=0;i<11;i++){
    display_line[0][i] = RESET_STATE;
    display_line[1][i] = RESET_STATE;
    display_line[2][i] = RESET_STATE;
    display_line[3][i] = RESET_STATE;
  }
  display_line[0][10] = 0;
  display_line[1][10] = 0;
  display_line[2][10] = 0;
  display_line[3][10] = 0;

  display[0] = &display_line[0][0];
  display[1] = &display_line[1][0];
  display[2] = &display_line[2][0];
  display[3] = &display_line[3][0];
  update_display = 0;


// Interrupts are disabled by default, enable them.
  enable_interrupts();
//------------------------------------------------------------------------------
}


void Display_Process(void){
  if(update_display){
    update_display = 0;
    if(display_changed){
      display_changed = 0;
      Display_Update(0,0,0,0);
    }
  }
}


//void Init_DAC(void){
//    DAC_data = 4000; // Value between 0x000 and 0x0FFF
//    SAC3DAT = DAC_data; // Initial DAC data
//    SAC3DAC = DACSREF_1; // Select int Vref as DAC reference
//    SAC3DAC |= DACLSEL_0; // DAC latch loads when DACDAT written
//    // SAC3DAC |= DACIE; // generate an interrupt
//    SAC3DAC |= DACEN; // Enable DAC
//    SAC3OA = NMUXEN; // SAC Negative input MUX controL
//    SAC3OA |= PMUXEN; // SAC Positive input MUX control
//    SAC3OA |= PSEL_1; // 12-bit reference DAC source selected
//    SAC3OA |= NSEL_1; // Select negative pin input
//    SAC3OA |= OAPM; // Select low speed and low power mode
//    SAC3PGA = MSEL_1; // Set OA as buffer mode
//    SAC3OA |= SACEN; // Enable SAC
//    SAC3OA |= OAEN; // Enable OA
//}




