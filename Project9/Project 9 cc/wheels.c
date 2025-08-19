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
char line;
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


// Function Prototype
void Wheels_Process(void);
void stop(void);

void Wheels_Process(void) {
    switch(wheel_state) {
    case IDLE:
        if(waitflag == LOW) {
            if(configure_wheels_flag == HIGH) {
                configure_wheels_flag = LOW;
                wheel_state = FMOVE;
            }
        }
        break;
    case FMOVE:
        LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle

        going_forward = TRUE;
        if(traceflag == TRUE) {
            strcpy(display_line[0], "FOLLOWING ");
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            display_changed = TRUE;
            update_display = TRUE;
            traceflag = FALSE;
            two_rotations = HIGH;
            wheel_state = TADJUST;
        }
        else {
            strcpy(display_line[0], " FORWARD  ");
            RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
            RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
            LEFT_FORWARD_SPEED = SLOW; // P6.3 Left Forward PWM duty cycle
            LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
            display_changed = TRUE;
            update_display = TRUE;
            wheel_state = FADJUST;
        }
        break;
    case FADJUST:
        if(ADC_Left_Detect > 150) {
            strcpy(display_line[0], "INTERCEPT ");
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
        if(ADC_Right_Detect > 150) {
            strcpy(display_line[0], " DETECTED ");

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
    case ALIGN:
        RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = SLOW; // P6.4 Left Reverse PWM duty cycle
        strcpy(display_line[0], " SPINNING ");
        display_changed = TRUE;
        update_display = TRUE;
        wheel_state = FADJUST;
        break;
    case TADJUST:
        switch(line) {
        case MEASURE:
            if(two_rotations == LOW) {
                turn_middle = TRUE;
                wheel_state = INITSTOP;
            }
            else if((ADC_Left_Detect > 200) && (ADC_Right_Detect > 200)) {
                line = BOTHON;
            }
            else if((ADC_Left_Detect < 200) && (ADC_Right_Detect > 200)) {
                line = LEFTOFF;
                prev_state = 'L';
            }
            else if((ADC_Left_Detect > 200) && (ADC_Right_Detect < 200)) {
                line = RIGHTOFF;
                prev_state = 'R';
            }
//            else {
//                line = BOTHOFF;
//            }
            break;
        case BOTHON:
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            line = MEASURE;
            break;
        case LEFTOFF:
            RIGHT_FORWARD_SPEED = SLOWEST; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
            line = MEASURE;
            break;
        case RIGHTOFF:
            RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
            LEFT_FORWARD_SPEED = SLOWEST; // P6.3 Left Forward PWM duty cycle
            line = MEASURE;
            break;
//        case BOTHOFF:
//            if(prev_state == 'R') {
//                prev_state = 'L';
//                RIGHT_FORWARD_SPEED = SLOWEST; // P6.1 Right Forward PWM duty cycle
//                LEFT_FORWARD_SPEED = SLOWER; // P6.3 Left Forward PWM duty cycle
//                line = MEASURE;
//            }
//            if(prev_state == 'L') {
//                prev_state = 'R';
//                RIGHT_FORWARD_SPEED = SLOWER; // P6.1 Right Forward PWM duty cycle
//                LEFT_FORWARD_SPEED = SLOWEST; // P6.3 Left Forward PWM duty cycle
//                line = MEASURE;
//            }
//
//            break;
        default: break;
        }
        break;
    case TURNTOENTER:
        strcpy(display_line[0], " TURNING  ");
        display_changed = TRUE;
        update_display = TRUE;
        RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = SLOW; // P6.4 Left Reverse PWM duty cycle
        turn_time = HIGH;
        turn_middle = LOW;
        wheel_state = TTEADJUST;
        break;
    case TTEADJUST:
        if(turn_time == LOW) {
            enter_flag = HIGH;
            wheel_state = INITSTOP;
        }
        break;
    case ENTER:
        strcpy(display_line[0], " ENTERING ");
        display_changed = TRUE;
        update_display = TRUE;
        RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = SLOW; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        enter_time = HIGH;
        enter_flag = LOW;
        wheel_state = EADJUST;
        break;
    case EADJUST:
        if(enter_time == LOW) {
            wheel_state = INITSTOP;
        }
        break;
    case INITSTOP:
        RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        waitflag = HIGH;
        wheel_state = STOP;
        break;
    case STOP:
        if(waitflag == LOW) {
            if(spinflag == HIGH) {
                wheel_state = ALIGN;
                spinflag = LOW;
            }
            else if(traceflag == HIGH) {
                wheel_state = FMOVE;
            }
            else if(turn_middle == TRUE) {
                wheel_state = TURNTOENTER;
            }
            else if(enter_flag == HIGH) {
                wheel_state = ENTER;
            }
            else {
                wheel_state = IDLE;
            }
        }
        break;
    default: break;
    }
}



void stop(void) {
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
}




