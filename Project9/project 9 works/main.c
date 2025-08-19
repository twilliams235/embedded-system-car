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
void Init_Conditions(void);                   // Initialize Variables and Initial Conditions
void Init_Timer_B0(void);
void Init_Timer_B3(void);
void Init_LCD(void);                          // Initialize LCD
void Init_LEDs(void);
void Init_ADC(void);
void Display_Process(void);
void Wheels_Process(void);
void USCI_A1_transmit(void);
void IOT_Process(void);
void Init_Serial_UCA0(char speed); // Initialize Serial Port for USB
void Init_Serial_UCA1(char speed);

  // Global Variables
volatile char slow_input_down;

extern char *display[4];
unsigned char display_mode;
extern char display_line[4][11];
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
unsigned int delay_state;
unsigned int left_motor_count;
unsigned int right_motor_count;
unsigned int segment_count;
char event;
unsigned int dec_sec;
unsigned int step;
unsigned int second;
unsigned int startflag;

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
unsigned int two_rotations;
unsigned int turn_time;
unsigned int enter_time;

extern char process_buffer[25]; // Size for appropriate Command Length
extern char pb_index; // Index for process_buffer
extern volatile char IOT_Ring_Rx[SMALL_RING_SIZE];
extern volatile char IOT_Ring_Tx[SMALL_RING_SIZE];
extern volatile char IOT_Process_Tx[SMALL_RING_SIZE];
extern volatile char USB_Ring_Rx[SMALL_RING_SIZE];
extern volatile char USB_Process_Tx[SMALL_RING_SIZE];
extern unsigned int iot_rx_wr;
extern unsigned int direct_iot;
extern unsigned int iot_tx;
extern unsigned int usb_rx_wr;
extern volatile unsigned int usb_rx_ring_wr;
extern volatile char USB_Char_Rx[SMALL_RING_SIZE];
unsigned int x;

unsigned int displayflag1;
unsigned int displayflag0;
extern unsigned int usb_rx_rd;
unsigned int process_index;
extern unsigned int baud;
unsigned int tranlen;
unsigned int reclen;
unsigned int i;
unsigned int p;
unsigned int j;
unsigned int process;
unsigned int ring_index;

extern unsigned int no_transmit;

//char process_buffer[25]; // Size for appropriate Command Length
//char pb_index; // Index for process_buffer
//extern volatile char IOT_Ring_Rx[SMALL_RING_SIZE];
//extern volatile char iot_TX_buf[SMALL_RING_SIZE];
//char USB_Ring_Rx[SMALL_RING_SIZE];
//extern unsigned int iot_rx_wr;
//unsigned int direct_iot;
//extern unsigned int iot_tx;
//unsigned int usb_rx_wr;
//volatile unsigned int usb_rx_ring_wr;
//volatile char USB_Char_Rx[SMALL_RING_SIZE];
//unsigned int usb_tx_ring_wr;
//unsigned int usb_tx_ring_rd;
//unsigned int usb_rx_ring_rd;
//unsigned char USB_Char_Tx[SMALL_RING_SIZE];


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

  Init_Ports();                        // Initialize Ports
  Init_Clocks();                       // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  Init_Timer_B0();
  Init_Timer_B3();
  Init_LCD();                          // Initialize LCD
  Init_LEDs();
  Init_ADC();
//P2OUT &= ~RESET_LCD;
  // Place the contents of what you want on the display, in between the quotes
// Limited to 10 characters per line
  strcpy(display_line[0], " Waiting  ");
  strcpy(display_line[1], "          ");
  strcpy(display_line[2], " 115,200  ");
  strcpy(display_line[3], "          ");
  display_changed = TRUE;
  update_display = TRUE;
//  Display_Update(0,0,0,0);


  dec_sec = LOW;

  wheel_state = IDLE;
  configure_wheels_flag = LOW;
  ADC_Channel = LOW;
  two_rotations = LOW;
  turn_time = LOW;
  enter_time = LOW;


  PWM_PERIOD = WHEEL_PERIOD; // PWM Period IN Timers.C [Set this to 50005]
  LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle
  RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
  RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
  LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
  LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
  P2OUT |= IR_LED;           // Initial Value = High
  line = MEASURE;

  //iot_TX_buf = "NCSU #1\0";

  x = 0;
  i = 0;
  p = 0;
  j = 0;
  process = 0;
  displayflag1 = LOW;
  process_index = LOW;
  ring_index = 0;
//  baud = LOW;
  tranlen = 0;
  Init_Serial_UCA0('a');
  Init_Serial_UCA1('a');

//------------------------------------------------------------------------------
// Beginning of the "While" Operating System
//------------------------------------------------------------------------------


  while(ALWAYS) {

      if(displayflag1 == HIGH) {
          displayflag1 = LOW;
          for(p = 0; p < 10; p++) {
              USB_Process_Tx[p] = USB_Ring_Rx[usb_rx_rd];
              if(USB_Process_Tx[p]== '\n') {
                  //If we get this \n we have a complete transaction
                  // When we have a complete transaction we process it
                  // IE: Printing, transmitting, reset buffer
                  strcpy(display_line[3], "          ");
                  display_changed = TRUE;
                  update_display = TRUE;
                  usb_rx_rd++;
                  if(usb_rx_rd >= sizeof(USB_Ring_Rx)) {
                      usb_rx_rd = 0;
                  }
                  break;
              }
              usb_rx_rd++;
              if(usb_rx_rd >= sizeof(USB_Ring_Rx)) {
                  usb_rx_rd = 0;
              }
          }
          for (i = 0; i < 10; i++) {
              if(USB_Process_Tx[i] == '\r') {
                  USCI_A0_transmit();
                  break;
              }
              else {
                  display_line[3][i] = USB_Process_Tx[i];
                  display_changed = TRUE;
                  update_display = TRUE;
              }
          }
      }

      if(displayflag0 == HIGH) {
          displayflag0 = LOW;
          for(process = 0; process < 10; process++) {
              USB_Process_Tx[process] = USB_Ring_Rx[usb_rx_rd];
              if(USB_Process_Tx[process]== '\n') {
                  //If we get this \n we have a complete transaction
                  // When we have a complete transaction we process it
                  // IE: Printing, transmitting, reset buffer
                  strcpy(display_line[3], "          ");
                  display_changed = TRUE;
                  update_display = TRUE;
                  usb_rx_rd++;
                  if(usb_rx_rd >= sizeof(USB_Ring_Rx)) {
                      usb_rx_rd = 0;
                  }
                  break;
              }
              usb_rx_rd++;
              if(usb_rx_rd >= sizeof(USB_Ring_Rx)) {
                  usb_rx_rd = 0;
              }
          }
          for (j = 0; j < 10; j++) {
              if(USB_Process_Tx[j] == '\r') {
                  USCI_A1_transmit();
                  break;
              }
              else {
                  display_line[3][j] = USB_Process_Tx[j];
                  display_changed = TRUE;
                  update_display = TRUE;
              }
          }
      }


      Display_Process();                 // Update Display
      //Wheels_Process();
      //IOT_Process();


  }
//------------------------------------------------------------------------------

}






