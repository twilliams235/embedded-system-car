//------------------------------------------------------------------------------
//
//  Description: This file contains the Main Routine - "While" Operating System
//
//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include  "Compiled/functions.h"
#include  "Compiled/LCD.h"
#include  "Compiled/ports.h"
#include  "Compiled/macros.h"
#include  "Compiled/timers.h"

// Function Prototypes
void main(void);
void Init_Ports(void);                        // Initialize Ports
void Init_Clocks(void);                       // Initialize Clock System
void Init_Timer_B0(void);
void Init_Timer_B3(void);
void Init_LCD(void);                          // Initialize LCD
void Init_ADC(void);
void Display_Process(void);
void Wheels_Process(void);
void USCI_A1_transmit(void);
void IOT_Process(void);
void Init_Serial_UCA0(char speed); // Initialize Serial Port for USB
void Init_Serial_UCA1(char speed);
void USCI_A1_transmit(void);
void USCI_A0_transmit(void);
void IOT_Process(void);
void Leave_Process(void);
void Init_DAC(void);

  // Global Variables
extern char *display[4];
unsigned char display_mode;
extern char display_line[4][11];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;

unsigned int dec_sec;
unsigned char ADC_Channel;
unsigned int ADC_Left_Detect;
unsigned int ADC_Right_Detect;
unsigned int ADC_Thumb_Detect;
unsigned char adc_char[4];

char wheel_state;
unsigned int configure_wheels_flag;
unsigned int waitflag;
unsigned int going_forward;
unsigned int spinflag;
unsigned int traceflag;
unsigned int turn_time;
unsigned int wheel_flag;


extern char process_buffer[25]; // Size for appropriate Command Length
unsigned int usb_pb_index; // Index for process_buffer
unsigned int iot_pb_index;
extern volatile char IOT_Ring_Rx[SMALL_RING_SIZE];
extern volatile char IOT_Process_Tx[SMALL_RING_SIZE];
extern volatile char USB_Ring_Rx[SMALL_RING_SIZE];
extern volatile char USB_Process_Tx[SMALL_RING_SIZE];
extern unsigned int iot_rx_wr;
extern unsigned int direct_iot;
extern unsigned int iot_tx;
extern unsigned int usb_rx_wr;
extern volatile unsigned int usb_rx_ring_wr;
extern volatile char USB_Char_Rx[SMALL_RING_SIZE];

unsigned int displayflag1;
unsigned int displayflag0;
unsigned int fram_only;
extern unsigned int usb_rx_rd;
extern unsigned int iot_rx_rd;
extern unsigned int baud;
unsigned int ring_index;
extern unsigned int iot_flag;
extern unsigned int line;
extern unsigned int ready_trans;
extern unsigned int short_count;
extern unsigned int pad_count;
extern unsigned int start_timer;

extern unsigned int leave_time;
extern unsigned char leave_state;
unsigned int leave_circle;
char blackline;
extern unsigned int power;
unsigned int circle_stop_count;
unsigned int circle_timer;
extern unsigned int count_to_stop_timer;
extern unsigned int count_to_stop;
unsigned int wait_to_leave_timer;
unsigned int wait_to_leave;


void main(void){
//    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------
  PM5CTL0 &= ~LOCKLPM5;
// Disable the GPIO power-on default high-impedance mode to activate
// previously configured port settings

  Init_DAC();
  Init_Ports();                         // Initialize Ports
  Init_Clocks();                        // Initialize Clock System
  Init_Conditions();                    // Initialize Variables and Initial Conditions
  Init_Timer_B0();                      // Initialize Timer B0
  Init_Timer_B3();                      // Initialize Timer B3
  Init_LCD();                           // Initialize LCD
  Init_ADC();                           // Initialize ADC
  Init_Serial_UCA0('a');                // Initialize UCA0
  Init_Serial_UCA1('a');                // Initialize UCA1


  PWM_PERIOD = WHEEL_PERIOD; // PWM Period IN Timers.C [Set this to 50005]
  LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle
  RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
  RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
  LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
  LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
  P2OUT |= IR_LED;           // Initial Value = High

// Initialize a bunch of flag and variables low
  usb_pb_index = LOW;
  iot_pb_index = LOW;
  displayflag1 = LOW;
  fram_only = FALSE;
  displayflag0 = LOW;
  ring_index = LOW;
  line = LOW;
  blackline = MEASURE;          // Initialize blackline state to MEASURE
  ready_trans = LOW;
  short_count = LOW;
  pad_count = HEXZERO;     // Initialize my pad count to HEX zero
  leave_time = LOW;
  leave_state = IDLE;           // Initialize leave_state state to IDLE
  wheel_flag = LOW;
  leave_circle = LOW;
  power = LOW;
  dec_sec = LOW;
  wheel_state = IDLE;               // Initialize leave_state state to IDLE
  configure_wheels_flag = LOW;
  ADC_Channel = LOW;
  start_timer = FALSE;
  circle_timer = LOW;
  circle_stop_count = LOW;
  count_to_stop = LOW;
  count_to_stop_timer = LOW;
  wait_to_leave = LOW;
  wait_to_leave_timer = LOW;        //set flag low


  //Enable IOT
  P3OUT |= IOT_EN;
  P3OUT &= ~IOT_EN;
  iot_flag = HIGH;
  strcpy(display_line[2], " ENABLING ");
  display_changed = TRUE;
  update_display = TRUE;

//  baud = LOW;

//------------------------------------------------------------------------------
// Beginning of the "While" Operating System
//------------------------------------------------------------------------------


  while(ALWAYS) {

      IOT_Process();                // Read the IOT
      Display_Process();            // Update Display
      Wheels_Process();             // Black line detection process, starts once it receives a W from the IOT
      Leave_Process();              // Dismount process, starts once it receives an L from the IOT


  }
//------------------------------------------------------------------------------

}






