/*
 * wheels.c
 *
 *  Created on: Sep 26, 2023
 *      Author: tylerwilliams
 */


#include  "msp430.h"
#include  <string.h>
#include  <stdlib.h>
#include  "Compiled/functions.h"
#include  "Compiled/LCD.h"
#include  "Compiled/ports.h"
#include  "Compiled/macros.h"
#include  "Compiled/timers.h"

  // Global Variables
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
unsigned int moving;         //Variable Tyler created
char start_moving;           //Variable Tyler created
unsigned int Last_Time_Sequence;   //Variable to identify Time_Sequence has changed
unsigned int cycle_time;            // time base used to control making shapes
unsigned int time_change;           // an identifier that a change has occurred
unsigned int delay_start;
unsigned int left_motor_count;
unsigned int right_motor_count;
unsigned int segment_count;

unsigned char ADC_Channel;
unsigned int ADC_Left_Detect;
unsigned int ADC_Right_Detect;
unsigned int ADC_Thumb_Detect;
unsigned char adc_char[4];

unsigned int lookflag;
char wheel_state;
extern unsigned int wheel_flag;
extern char blackline;
unsigned int configure_wheels_flag;
unsigned int waitflag;
unsigned int going_forward;
unsigned int spinflag;
unsigned int traceflag;
unsigned int turn_middle;
unsigned int two_rotations;
unsigned int turn_time;
unsigned int enter_flag;
unsigned int enter_time;
unsigned int already_set_left;
unsigned int already_set_right;
char prev_state;
extern unsigned int leave_time;
unsigned char leave_state;
unsigned int leave_flag;
extern unsigned int turn_time;
extern unsigned int circle_stop_count;
extern unsigned int count_to_stop;
extern unsigned int wait_to_leave;

extern unsigned int white_val;
extern unsigned int black_val;


// Function Prototype
void Wheels_Process(void);
void Leave_Process(void);
void stop(void);



// Function to find by driving in an arc, then follow a blackline
void Wheels_Process(void) {
    switch(wheel_state) {
    case IDLE:              // initial idle state
        if(wheel_flag == HIGH) {        // if wheel flag is high, start the wheels process
            wheel_flag = LOW;
            wheel_state = FMOVE;
        }
        break;
    case FMOVE:         // case used to trace the line, and arc to find the line
        LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle

        going_forward = TRUE;           // set going forward flag true
        if(traceflag == TRUE) {         // Trace black line
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            traceflag = FALSE;
            strcpy(display_line[0], "BL TRAVEL ");
            display_changed = TRUE;
            update_display = TRUE;
            count_to_stop = HIGH;           // start timer to wait after stopping inside circle
            wheel_state = TADJUST;
        }
        else {
            strcpy(display_line[0], " BL START ");          // arc to find black line by using different PWM values
            RIGHT_FORWARD_SPEED = MIDSLOW; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = MID; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            display_changed = TRUE;
            update_display = TRUE;
            wheel_state = FINDWHITE;            // next state
        }
        break;
    case FINDWHITE:         // case to find white first to know you're on the board
        if(ADC_Left_Detect < 56) {       // find white
            wheel_state = FADJUST;
        }
        if(ADC_Right_Detect < 56) {     // find white
            wheel_state = FADJUST;
        }
        break;
    case FADJUST:       // case that looks for black to intercept the line
        if(ADC_Left_Detect > 250) {         // find black
            strcpy(display_line[0], "INTERCEPT ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == TRUE) {         // if arcing, spin to align with the line
                spinflag = TRUE;
                going_forward = FALSE;
            }
            else {
                traceflag = TRUE;       // if not arcing, but aligning rather, trace and follow the line
            }
            wheel_state = INITSTOP;         // next state
        }
        if(ADC_Right_Detect > 250) {             // find black
            strcpy(display_line[0], "INTERCEPT ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == TRUE) {     // if arcing, spin to align with the line
                spinflag = TRUE;
                going_forward = FALSE;
            }
            else {
                traceflag = TRUE;       // if not arcing, but aligning rather, trace and follow the line
            }
            wheel_state = INITSTOP;     // next state
        }
        break;
    case FADJUST2:      // different case used to continue spinning until it detects black again and finds the line
        if(ADC_Left_Detect > 250) {         // find balck
            strcpy(display_line[0], " BL TURN  ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == TRUE) {
                spinflag = TRUE;
                going_forward = FALSE;
            }
            else {
                traceflag = TRUE;
            }
            wheel_state = INITSTOP;
        }
        if(ADC_Right_Detect > 250) {            // find black
            strcpy(display_line[0], " BL TURN  ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == TRUE) {
                spinflag = TRUE;
                going_forward = FALSE;
            }
            else {
                traceflag = TRUE;
            }
            wheel_state = INITSTOP;
        }
        break;
    case ALIGN:         // case to start spinning to find line
        RIGHT_FORWARD_SPEED = JGOSLOW; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = MIDDLE; // P6.4 Left Reverse PWM duty cycle
        strcpy(display_line[0], "  BL TURN ");
        display_changed = TRUE;
        update_display = TRUE;
        wheel_state = FADJUST2;         // next state
        break;
    case TADJUST:           // case made to adjust PWM values while tracing the line
        switch(blackline) {
        case MEASURE:           // case to measure the ADC values
            if(count_to_stop == LOW) {      // if timer is low, it is time to stop and display BL Circle for the TAs
                RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
                RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
                LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
                LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
                strcpy(display_line[0], "BL CIRCLE ");
                circle_stop_count = HIGH;
                display_changed = TRUE;
                update_display = TRUE;
                wheel_state = CIRCLESTOP;       // case to stop
            }
            else if((ADC_Left_Detect > 200) && (ADC_Right_Detect > 200)) {  //if both values above 200, both wheels go forward
                blackline = BOTHON;
            }
            else if((ADC_Left_Detect < 200) && (ADC_Right_Detect > 200)) {  //if left less than 200 and right greater than 200: left wheel is off, so turn off right wheel to realign
                blackline = LEFTOFF;
            }
            else if((ADC_Left_Detect > 200) && (ADC_Right_Detect < 200)) { //if left greater than 200 and right less than 200: right wheel is off, so turn off left wheel to realign
                blackline = RIGHTOFF;
                prev_state = 'R';
            }
//            else {
//                line = BOTHOFF;
//            }
            break;
        case BOTHON:
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            blackline = MEASURE;
            break;
        case LEFTOFF:
            RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            blackline = MEASURE;
            break;
        case RIGHTOFF:
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
            blackline = MEASURE;
            break;
        default: break;
        }
        break;
        case TADJUST2:          // second trace adjust state to be used after BL Circle has been displayed
            switch(blackline) {
            case MEASURE:
                if((ADC_Left_Detect > 200) && (ADC_Right_Detect > 200)) {   //if both values above 200, both wheels go forward
                    blackline = BOTHON;
                }
                else if((ADC_Left_Detect < 200) && (ADC_Right_Detect > 200)) {  //if right greater than 200 and left less than 200: left wheel is off, so turn off right wheel to realign
                    blackline = LEFTOFF;
                    prev_state = 'L';
                }
                else if((ADC_Left_Detect > 200) && (ADC_Right_Detect < 200)) {  //if left greater than 200 and right less than 200: right wheel is off, so turn off left wheel to realign
                    blackline = RIGHTOFF;
                    prev_state = 'R';
                }
                break;
            case BOTHON:
                RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
                LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
                blackline = MEASURE;
                break;
            case LEFTOFF:
                RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
                LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
                blackline = MEASURE;
                break;
            case RIGHTOFF:
                RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
                LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
                blackline = MEASURE;
                break;
            default: break;
            }
            break;
    case INITSTOP:      // case to turn off all wheels
        RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        waitflag = HIGH;
        wheel_state = STOP;     // next state
        break;
    case CIRCLESTOP:            // case to wait after displaying BL Circle then restart tracing the circle
        if(circle_stop_count == LOW){
            wheel_state = TADJUST2;
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
        }
        break;
    case STOP:          // case to send to new case after stopping depending on what flag is set high
        if(waitflag == LOW) {       // if waitflag is low, the wait is up anf time to send to new case
            if(spinflag == HIGH) {      // if spinflag high --> time to align
                wheel_state = ALIGN;
                spinflag = LOW;
            }
            else if(traceflag == HIGH) {
                wheel_state = FMOVE;        // if traceflag is high --> time to follow the line
            }
            else {
                wheel_state = IDLE;         // else send back to idle state
            }
        }
        break;
    default: break;
    }
}


void Leave_Process(void) {
    switch(leave_state) {
    case IDLE:          // initial idle case
        break;
    case LEAVING:       // case to stop wheels before dismounting circle to show TAs BL Exit
        RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        strcpy(display_line[0], "  BL EXIT ");
        display_changed = TRUE;
        update_display = TRUE;
        wait_to_leave = HIGH;           // wait to let TAs see BL Exit
        leave_state = BEGINLEAVING;     // next state
        break;
    case BEGINLEAVING:      // case to drive out of the circle
        if(wait_to_leave == LOW) {      // after wait to leave timer is finished, leave the circle by driving forward
            RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = SLOW; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            leave_time = HIGH;          // start timer to measure how far the car is driving out of the circle
            leave_state = LADJUST;      // next state
        }
        break;
    case LADJUST:           // case to stop the car once it is 2 feet away from the circle
        if(leave_time == LOW) {     // once the car is out of the circle
            strcpy(display_line[0], " BL STOP  ");
            display_changed = TRUE;
            update_display = TRUE;
            RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            leave_state = IDLE;         // set back to idle state
        }
        break;
    default: break;
    }
}

// function to stop all wheels
void stop(void) {
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
}




