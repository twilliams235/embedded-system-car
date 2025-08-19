/*
 * buffer.c
 *
 *  Created on: Nov 17, 2023
 *      Author: tylerwilliams
 */

#include  "msp430.h"
#include  <string.h>
#include  "Compiled/functions.h"
#include  "Compiled/LCD.h"
#include  "Compiled/ports.h"
#include  "Compiled/macros.h"
#include  "Compiled/timers.h"



extern unsigned int displayflag1;
extern unsigned int displayflag0;
extern unsigned int fram_only;
extern unsigned int iot_rx_rd;
extern unsigned int iot_tx;
extern unsigned int usb_rx_rd;
extern unsigned int usb_pb_index; // Index for process_buffer
extern unsigned int iot_pb_index;
extern volatile char IOT_Ring_Rx[SMALL_RING_SIZE];
extern volatile char IOT_Process_Tx[SMALL_RING_SIZE];
extern volatile char USB_Ring_Rx[SMALL_RING_SIZE];
extern volatile char USB_Process_Tx[SMALL_RING_SIZE];
extern unsigned int iot_rx_wr;
extern unsigned int usb_rx_wr;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern char display_line[4][11];

unsigned int ch;
unsigned char command;
unsigned int iot_flag;

//IOT Process ================================================
unsigned int line;
unsigned int nextline;
volatile char IOT_Data[4][SMALL_RING_SIZE];
unsigned int character;
unsigned int test_Value;
unsigned int IOT_parse;
unsigned int boot_state;
unsigned int iot_index;
unsigned int ssid;
char movement;
unsigned int expo;
unsigned int runflag;
unsigned int runtime;
unsigned int carot_found;
char ip_sys[15] = "AT+SYSSTORE=0\r\n";
char ip_mux[13] = "AT+CIPMUX=1\r\n";
char ip_serv[21] = "AT+CIPSERVER=1,8091\r\n";
char AT[SMALL_RING_SIZE];
char ip_address[SMALL_RING_SIZE];
extern unsigned int ready_trans;
unsigned int short_wait;
unsigned int pad_count;
extern unsigned int leave_circle;
extern unsigned int leave_flag;
extern char leave_state;
extern char wheel_state;

unsigned int start_timer;

extern unsigned int wheel_flag;



void process_UCA0(void);
void process_UCA1(void);
void handle_fram(void);
void USCI_A0_transmit(void);
void USCI_A1_transmit(void);
void IOT_Process(void);
void stop(void);
void Wheels_Process(void);



void IOT_Process(void){ // Process IOT messages
    int i;
    unsigned int iot_rx_wr_temp;
    iot_rx_wr_temp = iot_rx_wr;
    if(iot_rx_wr_temp != iot_rx_rd){ // Determine if IOT is available
        IOT_Data[line][character] = IOT_Ring_Rx[iot_rx_rd++];       // Add character from Ring to Process buffer
        if(iot_rx_rd >= sizeof(IOT_Ring_Rx)){           // If read index is larger than the ring size, set read index to 0
            iot_rx_rd = BEGINNING;
        }

        if(IOT_Data[line][character] == 0x0A){      //0x0A is \r carriage return, indicating the end of a command
            character = 0;
            line++;         //Increment to the next process buffer
            if(line >= 4){      // if on fourth buffer, reset to 1st buffer
                line = 0;
            }
        }else{
            switch(character){
            case 0:
                if(IOT_Data[line][character] == '+'){ // Got "+"
                    test_Value++;
                    if(test_Value){
                        RED_LED_ON;
                    }
                    IOT_parse = 1;
                }
                break;
             case 1:
                 // GRN_LED_ON;
                 break;
              case 6:
                  if(IOT_Data[line][character] == 'R'){     // got ip address
                      strcpy(display_line[1], "IP Address");

                       for(i=0; i<sizeof(ip_address); i++){
                           ip_address[i] = 0;
                       }
                      display_changed = TRUE;
                      update_display = TRUE;
                  }
                  else if (IOT_Data[line][character] == 'G'){       // IOT ready to connect to wifi
                      if(ready_trans == 0) {
                          for(i=0; i<sizeof(ip_sys); i++){
                              USB_Process_Tx[i] = ip_sys[i];        // Send "AT+SYSSTORE=0\r\n"
                          }
                          iot_tx = 0;
                          UCA0IE |= UCTXIE;                     //Enable transmit
                      }
                      short_wait = HIGH;                    // Wait shortly to send next command
                      while(short_wait == HIGH) {
                          //wait
                      }
                      for(i=0; i<sizeof(ip_mux); i++){
                          USB_Process_Tx[i] = ip_mux[i];        // Send "AT+CIPMUX=1\r\n"
                      }
                      iot_tx = 0;
                      UCA0IE |= UCTXIE;                     //Enable transmit
                      short_wait = HIGH;                    // Wait shortly to send next command
                      while(short_wait == HIGH) {
                          //wait
                      }
                      for(i=0; i<sizeof(ip_serv); i++){
                          USB_Process_Tx[i] = ip_serv[i];       // send "AT+CIPSERVER=1,8091\r\n" setting the port to 8091
                      }
                      iot_tx = 0;
                      UCA0IE |= UCTXIE;                         //Enable transmit
                      strcpy(display_line[0], "WAIT 4 IP ");
                      strcpy(display_line[1], "   ncsu   ");
                      strcpy(display_line[2], "10.154.   ");            // Display IP address
                      strcpy(display_line[3], "20.28     ");
                      display_changed = TRUE;
                      update_display = TRUE;
                  }
                  break;
              case 11:
                  if (IOT_Data[line][character] == '^') {       // got starter indication carot
                      runtime = 0;
                      carot_found = TRUE;
                      strcpy(display_line[1], "T WILLIAMS");
                  }
                  break;
              case 16:
                  start_timer = TRUE;                   // set start timer flag high
                  if(carot_found == TRUE) {
                      movement = IOT_Data[line][character];
                      switch(movement) {
                      case FWARD:                   // Forward with a timer case
                          stop();
                          RIGHT_FORWARD_SPEED = MIDMOVE;
                          LEFT_FORWARD_SPEED = MIDMOVE;
                          break;
                      case BWARD:       //Backwards case
                          stop();
                          RIGHT_REVERSE_SPEED = MIDMOVE;
                          LEFT_REVERSE_SPEED = MIDMOVE;
                          break;
                      case RIGHT:           // Right case
                          stop();
                          RIGHT_REVERSE_SPEED = MIDMOVE;
                          LEFT_FORWARD_SPEED = MIDMOVE;
                          break;
                      case LEFT:            // Left case
                          stop();
                          RIGHT_FORWARD_SPEED = MIDMOVE;
                          LEFT_REVERSE_SPEED = MIDMOVE;
                          break;
                      case ARRIVED:         // Arrived at X case
                          pad_count++;
                          strcpy(display_line[0], " Arrived  ");
                          display_line[0][9] = pad_count;       // Display what pad you're on
                          display_changed = TRUE;
                          update_display = TRUE;
                          break;
                      case WHEELSPROCESS:           // Follow the blackline case
                          carot_found = LOW;
                          wheel_flag = HIGH;
                          break;
                      case DISMOUNT:        // Dismount circle case
                          wheel_state = IDLE;               // end wheels process by sending back to9 idle process
                          carot_found = LOW;
                          leave_state = LEAVING;            // Start leaving process code to dismount off circle
                          break;        // Stop case
                      case STOP:
                          carot_found = LOW;
                          stop();
                          break;
                      case GO:              // Go forwards without a timer case
                          carot_found = LOW;
                          RIGHT_FORWARD_SPEED = MIDMOVE;
                          LEFT_FORWARD_SPEED = MIDMOVE;
                          break;
                      case TURBOMODE:           // TURBO MODE
                          carot_found = LOW;
                          RIGHT_FORWARD_SPEED = FAST;
                          LEFT_FORWARD_SPEED = FAST;
                      default:
                          break;
                      }
                  }
                  break;
              case 17:
                  if(carot_found == TRUE) {             // get the time from the IOT command
                      runtime = (IOT_Data[line][character] - 0x30)*1000;
                  }
                  break;
              case 18:
                  if(carot_found == TRUE) {         // get the time from the IOT command
                      runtime = runtime+(IOT_Data[line][character] - 0x30)*100;
                  }
                  break;
              case 19:
                  if(carot_found == TRUE) {             // get the time from the IOT command
                      runtime = runtime+(IOT_Data[line][character] - 0x30)*10;
                  }
                  break;
              case 20:
                  if(carot_found == TRUE) {             // get the time from the IOT command
                      runtime = runtime+(IOT_Data[line][character] - 0x30);
                      runflag = HIGH;                   // Start the timer based on the given runtime
                      carot_found = FALSE;
                  }
                  break;
              default: break;
              }
        }
    character++;            // Increment character count for process buffer
    }
}




