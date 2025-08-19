/*
 * wheels.c
 *
 *  Created on: Sep 26, 2023
 *      Author: tylerwilliams
 */


#include  "msp430.h"
#include  <string.h>
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
unsigned int configure_wheels_flag;
unsigned int waitflag;
unsigned int going_forward;
unsigned int spinflag;


// Function Prototype
void Wheels_Process(void);
void Go_Forward(void);
void stop(void);
void Go_Reverse(void);
void Go_CCW(void);
void Go_CW(void);

void Wheels_Process(void) {
    switch(wheel_state) {
    case IDLE:
        if(waitflag == 0) {
            if(configure_wheels_flag == 1) {
                configure_wheels_flag = 0;
                wheel_state = FMOVE;
            }
        }
        break;
    case FMOVE:
        LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle
        RIGHT_FORWARD_SPEED = SLOW; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = SLOW; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        going_forward = 1;
        strcpy(display_line[0], " FORWARD  ");
        strcpy(display_line[1], "  LOOKING ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          ");
        display_changed = TRUE;
        update_display = TRUE;
        wheel_state = FADJUST;
        break;
    case FADJUST:
        if(ADC_Left_Detect > 150) {
            strcpy(display_line[0], "          ");
            strcpy(display_line[1], " DETECTED ");
            strcpy(display_line[2], "          ");
            strcpy(display_line[3], "          ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == 1) {
                spinflag = 1;
                going_forward = 0;
            }
            wheel_state = INITSTOP;
        }
        if(ADC_Right_Detect > 150) {
            strcpy(display_line[0], "          ");
            strcpy(display_line[1], " DETECTED ");
            strcpy(display_line[2], "          ");
            strcpy(display_line[3], "          ");
            display_changed = TRUE;
            update_display = TRUE;
            if(going_forward == 1) {
                spinflag = 1;
                going_forward = 0;
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
        strcpy(display_line[1], "  LOOKING ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          ");
        display_changed = TRUE;
        update_display = TRUE;
        wheel_state = FADJUST;
        break;
    case INITSTOP:
        RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
        RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
        LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
        LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
        waitflag = 1;
        wheel_state = STOP;
        break;
    case STOP:
        if(waitflag == 0) {
            if(spinflag == 1) {
                wheel_state = ALIGN;
                spinflag = 0;
            }
            else {
                wheel_state = IDLE;
            }
        }
        break;
    default: break;
    }
}
/*
 * Function written for project 3 to go forwards
 */
void Go_Forward(void) {
    stop();
    if (!(P6OUT |= L_FORWARD)) {
        P6OUT |= L_FORWARD;
    }
    if (!(P6OUT |= R_FORWARD)) {
        P6OUT |= R_FORWARD;
    }
}

/*
 * Function written for project 5 to go backwards
 */
void Go_Reverse(void) {
    stop();
    if (!(P6OUT |= L_REVERSE)) {
        P6OUT |= L_REVERSE;
    }
    if (!(P6OUT |= R_REVERSE)) {
        P6OUT |= R_REVERSE;
    }
}

/*
 * Function written for project 5 to spin clockwise
 */
void Go_CW(void) {
    stop();
    if (!(P6OUT |= L_FORWARD)) {
        P6OUT |= L_FORWARD;
    }
    if (!(P6OUT |= R_REVERSE)) {
        P6OUT |= R_REVERSE;
    }
}

/*
 * Function written for project 5 to spin counterclockwise
 */
void Go_CCW(void) {
    stop();
    if (!(P6OUT |= L_REVERSE)) {
        P6OUT |= L_REVERSE;
    }
    if (!(P6OUT |= R_FORWARD)) {
        P6OUT |= R_FORWARD;
    }
}

/*
 * Function written for project 3 to stop
 */
void stop(void) {
    P6OUT &= ~L_FORWARD;
    P6OUT &= ~R_FORWARD;

    P6OUT &= ~L_REVERSE;
    P6OUT &= ~R_REVERSE;
}

/*
 * State case written for Project 4
 * The third state "RUN" controls the movement
 * This case is to have a different version for each different shape
 */
void run_forward(void){
    switch(left_motor_count++){
    case 0:
        P6OUT |= L_FORWARD;
        P6OUT |= R_FORWARD;
        break;
    case 7:
        P6OUT &= ~L_FORWARD;
        break;
    case 10:
        P6OUT &= ~R_FORWARD;
        break;
    default:
        break;
    }
}



