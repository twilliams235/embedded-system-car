//------------------------------------------------------------------------------
//
//  Description: This file contains the System Configurations
//
//  Jim Carlson
//  Sept 2023
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

unsigned int dec_sec;
extern volatile unsigned char update_display;
extern volatile unsigned char display_changed;
extern char display_line[4][11];
extern unsigned int step;
char event;
extern unsigned int dec_sec;
extern unsigned int second;
unsigned int startflag;

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

extern char *display[4];
unsigned char display_mode;

unsigned char clock_count_dec_sec;
unsigned char clock_count_sec;
unsigned char clock_count_ten_sec;
unsigned char clock_count_hundred_sec;
unsigned int clockflag;

char process_buffer[25]; // Size for appropriate Command Length
char pb_index; // Index for process_buffer
volatile char IOT_Ring_Rx[SMALL_RING_SIZE];
volatile char IOT_Process_Tx[SMALL_RING_SIZE];
volatile char USB_Ring_Rx[SMALL_RING_SIZE];
volatile char USB_Process_Tx[SMALL_RING_SIZE];
unsigned int iot_rx_wr;
unsigned int direct_iot;
unsigned int iot_tx;
unsigned int usb_rx_wr;
unsigned int usb_rx_rd;
unsigned int iot_rx_rd;
extern unsigned int x;

extern unsigned int displayflag1;
extern unsigned int displayflag0;
extern unsigned int fram_only;

unsigned int baud;
extern unsigned int reclen;
unsigned int i;
extern unsigned int iot_flag;
unsigned int iot_count;
unsigned int iot_process_count;
unsigned int usb_process_count;

extern unsigned int runflag;
extern unsigned int runtime;
unsigned int ready_trans;
extern unsigned int short_wait;
unsigned int short_count;

void enable_interrupts(void);
void USCI_A1_transmit(void);
void USCI_A0_transmit(void);
void stop(void);

//------------------------------------------------------------------------------
// System Configurations

// Tells the compiler to provide the value in reg as an input to an
// in line assembly block.  Even though the block contains no instructions,
// the compiler may not optimize it away, and is told that the value
// may change and should not be relied upon.

//in line void READ_AND_DISCARD(unsigned int reg) __attribute__((always_inline));
//void READ_AND_DISCARD(unsigned int reg){
//  asm volatile ("" : "=m" (reg) : "r" (reg));
//}

//in line void enable_interrupts(void) __attribute__((always_inline));
void enable_interrupts(void){
  __bis_SR_register(GIE);     // enable interrupts
//  asm volatile ("eint \n");
}

//inline void disable_interrupts(void) __attribute__((always_inline));
//void disable_interrupts(void){
//  asm volatile ("dint \n");
//}


// TimerB0 0 Interrupt handler
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void){

    clockflag++;
    if(clockflag == MS200) {
        clockflag = LOW;
        clock_count_dec_sec += INCREMENT2;
        if(clock_count_dec_sec > HEXNINE) {
            clock_count_dec_sec = HEXZERO;
            clock_count_sec += INCREMENT1;
            if(clock_count_sec > HEXNINE) {
                clock_count_sec = HEXZERO;
                clock_count_ten_sec += INCREMENT1;
                if(clock_count_ten_sec > HEXNINE) {
                    clock_count_ten_sec = HEXZERO;
                    clock_count_hundred_sec += INCREMENT1;
                    if(clock_count_hundred_sec > HEXZERO) {
                        clock_count_dec_sec = HEXZERO;
                        clock_count_sec = HEXZERO;
                        clock_count_ten_sec = HEXZERO;
                        clock_count_hundred_sec = HEXZERO;
                    }
                }
            }
        }
    }
    if (waitflag == HIGH) {
        dec_sec++;
        if(dec_sec == 50) {
            waitflag = LOW;
            dec_sec = LOW;
        }
    }
    if(iot_flag == HIGH) {
        iot_count++;
        if(iot_count == 25) {
            iot_flag = LOW;
            iot_count = LOW;
            P3OUT |= IOT_EN;
            strcpy(display_line[2], "IOT  READY");
            display_changed = TRUE;
            update_display = TRUE;
        }
    }
    if(short_wait == HIGH) {
        short_count++;
        if(short_count == 20) {
            short_wait = LOW;
            short_count = 0;
        }
    }

    if(runflag == HIGH) {
        runtime--;
        if(runtime == 0) {
            runflag = LOW;
            stop();
        }
    }
//    display_line[1][9] = clock_count_dec_sec;
//    display_line[1][8] = DOT;
//    display_line[1][7] = clock_count_sec;
//    display_line[1][6] = clock_count_ten_sec;
//    display_line[1][5] = clock_count_hundred_sec;
//    display_changed = TRUE;
//    update_display = TRUE;
    TB0CCR0 += TB0CCR0_INTERVAL; // Add Offset to TBCCR0
    ADCCTL0 |= ADCSC; // Start next sample
    update_display = HIGH;
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void){
//----------------------------------------------------------------------------
// TimerB0 1-2, Overflow Interrupt Vector (TBIV) handler
//----------------------------------------------------------------------------
    switch(__even_in_range(TB0IV,14)){
        case 0: break; // No interrupt
        case 2: // CCR1 Used for SW1 Debounce
            TB0CCTL1 &= ~CCIE; // Disable Timer B0 CCR1
            P4IFG &= ~SW1; // IFG SW1 cleared
            P4IE |= SW1; // SW1 interrupt Enabled
            TB0CCR1 += TB0CCR1_INTERVAL; // Add Offset to TBCCR1
            break;
        case 4: // CCR2 Used for SW2 Debounce
            TB0CCTL2 &= ~CCIE; // Disable Timer B0 CCR1
            P2IFG &= ~SW2; // IFG SW2 cleared
            P2IE |= SW2; // SW2 interrupt Enabled
            TB0CCR2 += TB0CCR2_INTERVAL; // Add Offset to TBCCR1
            break;
        case 14: // overflow available for greater than 1 second timer
            break;
        default: break;
    }
}



// Timer B0 initialization sets up both B0_0, B0_1-B0_2 and overflow
void Init_Timer_B0(void) {
    TB0CTL = TBSSEL__SMCLK; // SMCLK source
    TB0CTL |= TBCLR; // Resets TB0R, clock divider, count direction
    TB0CTL |= MC__CONTINOUS; // Continuous up
    TB0CTL |= ID__8; // Divide clock by 8
    TB0EX0 = TBIDEX__8; // Divide clock by an additional 8

    TB0CCR0 = TB0CCR0_INTERVAL; // CCR0
    TB0CCTL0 |= CCIE; // CCR0 enable interrupt

    TB0CCR1 = TB0CCR1_INTERVAL; // CCR1
    TB0CCTL1 |= CCIE; // CCR1 enable interrupt

    TB0CCR2 = TB0CCR2_INTERVAL; // CCR2
    TB0CCTL2 |= CCIE; // CCR2 enable interrupt

    TB0CTL &= ~TBIE; // Disable Overflow Interrupt
    TB0CTL &= ~TBIFG; // Clear Overflow Interrupt flag

    clock_count_dec_sec = HEXZERO;
    clock_count_sec = HEXZERO;
    clock_count_ten_sec = HEXZERO;
    clock_count_hundred_sec = HEXZERO;
    clockflag = LOW;
}


void Init_Timer_B3(void) {
//------------------------------------------------------------------------------
// SMCLK source, up count mode, PWM Right Side
// TB3.1 P6.0 LCD_BACKLITE
// TB3.2 P6.1 R_FORWARD
// TB3.3 P6.2 R_REVERSE
// TB3.4 P6.3 L_FORWARD
// TB3.5 P6.4 L_REVERSE
//------------------------------------------------------------------------------
    TB3CTL = TBSSEL__SMCLK; // SMCLK
    TB3CTL |= MC__UP; // Up Mode
    TB3CTL |= TBCLR; // Clear TAR
    PWM_PERIOD = WHEEL_PERIOD; // PWM Period [Set this to 50005]
    TB3CCTL1 = OUTMOD_7; // CCR1 reset/set
    LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle
    TB3CCTL2 = OUTMOD_7; // CCR2 reset/set
    RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
    TB3CCTL3 = OUTMOD_7; // CCR3 reset/set
    RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.2 Right Reverse PWM duty cycle
    TB3CCTL4 = OUTMOD_7; // CCR4 reset/set
    LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.3 Left Forward PWM duty cycle
    TB3CCTL5 = OUTMOD_7; // CCR5 reset/set
    LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
//------------------------------------------------------------------------------
}


//------------------------------------------------------------------------------
// Port 4 interrupt for switch 1, it is disabled for the duration
// of the debounce time. Debounce time is set for 1 second
#pragma vector=PORT4_VECTOR
__interrupt void switch1_interrupt(void) {
// Switch 1
    if (P4IFG & SW1) {

        waitflag = HIGH;
        configure_wheels_flag = HIGH;
        P4IE &= ~SW1; //Disable Switch 1
        P4IFG &= ~SW1; // Clear Switch 1 flag
        TB0CCTL1 &= ~CCIFG; // Clear TimerB0 Interrupt Flag for Capture Compare Register 1
        TB0CCR1 += TB0CCR1_INTERVAL; // Add Offset to TBCCR1
        TB0CCTL1 |= CCIE; // Enable TimerB0_1
        //USCI_A0_transmit();
        //Baud stuff
//        if(baud == LOW) {
//            baud = HIGH;
//            strcpy(display_line[2], " 57,600  ");
//            display_changed = TRUE;
//            update_display = TRUE;
//            UCA1MCTLW = 0xF7A1; //57,600 baud
//            UCA1BRW = 8; // 57,600 baud
//        }
//        else {
//            baud = LOW;
//            strcpy(display_line[2], " 115,200  ");
//            display_changed = TRUE;
//            update_display = TRUE;
//            UCA1MCTLW = 0x5551; //115,200 baud
//            UCA1BRW = 4; // 115,200 baud
//        }
        //IOT_Ring_Rx[SMALL_RING_SIZE] = "115,200";
//        strcpy(display_line[0], " 115,200 ");
//        strncpy(IOT_Ring_Rx, " NCSU #1  ", 10);
    }
//------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------
// Port 2 interrupt for switch 2, it is disabled for the duration
// of the debounce time. Debounce time is set for 1 second
#pragma vector=PORT2_VECTOR
__interrupt void switch2_interrupt(void) {
// Switch 2
    if (P2IFG & SW2) {

        P2IE &= ~SW2;
        P2IFG &= ~SW2; // IFG SW2 cleared
        TB0CCTL1 &= ~CCIFG; //Clear TimerB0 Interrupt Flag for Capture Compare Register 1
        TB0CCR2 += TB0CCR2_INTERVAL; // Add Offset to TBCCR2
        TB0CCTL2 |= CCIE; // Enable TimerB0_2
        //transmit stuff
        strcpy(display_line[1], "          ");
        display_changed = TRUE;
        update_display = TRUE;
        //USCI_A1_transmit();
//        for (i = 0; i < 10; i++) {
//            if(IOT_Process_Tx[i] == '\r') {
//                break;
//            }
//            else {
//                display_line[1][i] = IOT_Process_Tx[i];
//                display_changed = TRUE;
//                update_display = TRUE;
//             }
//         }
    }
//
}

void Init_ADC(void){
//------------------------------------------------------------------------------
// V_DETECT_L (0x04) // Pin 2 A2
// V_DETECT_R (0x08) // Pin 3 A3
// V_THUMB (0x20) // Pin 5 A5
//------------------------------------------------------------------------------
    // ADCCTL0 Register
    ADCCTL0 = LOW; // Reset
    ADCCTL0 |= ADCSHT_2; // 16 ADC clocks
    ADCCTL0 |= ADCMSC; // MSC
    ADCCTL0 |= ADCON; // ADC ON

    // ADCCTL1 Register
    ADCCTL1 = LOW; // Reset
    ADCCTL1 |= ADCSHS_0; // 00b = ADCSC bit
    ADCCTL1 |= ADCSHP; // ADC sample-and-hold SAMPCON signal from sampling timer.
    ADCCTL1 &= ~ADCISSH; // ADC invert signal sample-and-hold.
    ADCCTL1 |= ADCDIV_0; // ADC clock divider - 000b = Divide by 1
    ADCCTL1 |= ADCSSEL_0; // ADC clock MODCLK
    ADCCTL1 |= ADCCONSEQ_0; // ADC conversion sequence 00b = Single-channel single-conversion
    // ADCCTL1 & ADCBUSY identifies a conversion is in process

    // ADCCTL2 Register
    ADCCTL2 = LOW; // Reset
    ADCCTL2 |= ADCPDIV0; // ADC pre-divider 00b = Pre-divide by 1
    ADCCTL2 |= ADCRES_2; // ADC resolution 10b = 12 bit (14 clock cycle conversion time)
    ADCCTL2 &= ~ADCDF; // ADC data read-back format 0b = Binary unsigned.
    ADCCTL2 &= ~ADCSR; // ADC sampling rate 0b = ADC buffer supports up to 200 ksps

    // ADCMCTL0 Register
    ADCMCTL0 |= ADCSREF_0; // VREF - 000b = {VR+ = AVCC and VR– = AVSS }
    ADCMCTL0 |= ADCINCH_5; // V_THUMB (0x20) Pin 5 A5

    ADCIE |= ADCIE0; // Enable ADC conv complete interrupt
    ADCCTL0 |= ADCENC; // ADC enable conversion.
    ADCCTL0 |= ADCSC; // ADC start conversion.
}


#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG)){
    case ADCIV_NONE:
        break;
    case ADCIV_ADCOVIFG: // When a conversion result is written to the ADCMEM0
        // before its previous conversion result was read.
        break;
    case ADCIV_ADCTOVIFG: // ADC conversion-time overflow
        break;
    case ADCIV_ADCHIIFG: // Window comparator interrupt flags
        break;
    case ADCIV_ADCLOIFG: // Window comparator interrupt flag
        break;
    case ADCIV_ADCINIFG: // Window comparator interrupt flag
        break;
    case ADCIV_ADCIFG: // ADCMEM0 memory register with the conversion result
        ADCCTL0 &= ~ADCENC; // Disable ENC bit.
        switch (ADC_Channel++){
            case 0x00:                  // Channel A2 Interrupt
                ADC_Left_Detect = ADCMEM0; // Move result into Global
                ADC_Left_Detect = ADC_Left_Detect >> 2; // Divide the result by 4
                HEXtoBCD(ADC_Left_Detect); // Convert result to String
                adc_line(2, 0); // Place String in Display
                ADCMCTL0 &= ~ADCINCH_2; // Disable Last channel A2
                ADCMCTL0 |= ADCINCH_3; // Enable Next channel A3
                break;
            case 0x01:                  //Channel A3 interrupt
                ADC_Right_Detect = ADCMEM0; // Move result into Global Values
                ADC_Right_Detect = ADC_Right_Detect >> 2; // Divide the result by 4
                HEXtoBCD(ADC_Right_Detect); // Convert result to String
                adc_line(3, 0); // Place String in Display
                ADCMCTL0 &= ~ADCINCH_3; // Disable Last channel A3
                ADCMCTL0 |= ADCINCH_5; // Enable Next channel A5
                break;
            case 0x02:                  //Channel A5 interrupt
                ADC_Thumb_Detect = ADCMEM0; // Move result into Global
                ADC_Thumb_Detect = ADC_Thumb_Detect >> 2; // Divide the result by 4
                HEXtoBCD(ADC_Thumb_Detect); // Convert result to String
                adc_line(4, 0); // Place String in Display
                ADCMCTL0 &= ~ADCINCH_5; // Disable Last channel A5
                ADCMCTL0 |= ADCINCH_2; // Enable Next channel A2
                ADC_Channel=RESET_STATE;
                break;
            default:
                break;
        }
        ADCCTL0 |= ADCENC; // Enable Conversions
//        ADCCTL0 |= ADCSC; // Start next sample
        break;
    default:
        break;
    }
}

// Virtual link terminal script
// ls /dev/ | grep cu
// picocom /dev/cu.usbmodem212403 --baud 115200 --omap crcrlf --echo

#pragma vector = EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void){

    char iot_receive;
    switch(__even_in_range(UCA0IV,0x08)){
    case 0: break;
    case 2:{
        iot_receive = UCA0RXBUF;
        IOT_Ring_Rx[iot_rx_wr++] = iot_receive;
        if(iot_rx_wr >= sizeof(IOT_Ring_Rx)){
            iot_rx_wr = BEGINNING;
        }
        if(iot_receive == '^') {
            fram_only = TRUE;
        }
        //UCA1IE |= UCTXIE;
        UCA1TXBUF = iot_receive;
//        if(iot_receive == '\n') {
//            displayflag0 = TRUE;
//        }
    }break;
    case 4:{
        UCA0TXBUF = USB_Process_Tx[iot_tx];
        USB_Process_Tx[iot_tx++] = NULL;

//        if(iot_tx >= sizeof(IOT_Process_Tx[t0])) {
//            t0++;
//            iot_tx = 0;
//        }

        if(USB_Process_Tx[iot_tx] == 0x00){
            UCA0IE &= ~UCTXIE;
            iot_tx = 0; // Set Array index to first location
            ready_trans++;
            //t0 = 0;
        }
    }break;
    default: break;
    }

}

#pragma vector = EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void){

    char usb_value;
    switch(__even_in_range(UCA1IV,0x08)){
    case 0: break;
    case 2:{
        usb_value = UCA1RXBUF;
        USB_Ring_Rx[usb_rx_wr++] = usb_value; // Add to Ring Buffer
        if(usb_rx_wr >= sizeof(USB_Ring_Rx)){
            usb_rx_wr = BEGINNING;
        }
        UCA0TXBUF = usb_value;
        //UCA0IE |= UCTXIE;
//        if(usb_value == '^') {
//            fram_only = TRUE;
//        }
//        if(usb_value == '\n') {
//            displayflag1 = TRUE;
//        }
    }break;
    case 4:{
        UCA1TXBUF = IOT_Ring_Rx[direct_iot];
        IOT_Ring_Rx[direct_iot++]  = 0;
        if(direct_iot >= sizeof(IOT_Ring_Rx)) {
            direct_iot = BEGINNING;
        }
        if(iot_rx_wr == direct_iot) {
            UCA1IE &= ~UCTXIE;
        }

//        if(direct_iot >= sizeof(IOT_Process_Tx[t1])) {
//            t1++;
//            direct_iot = 0;
//        }

//        if(IOT_Process_Tx[direct_iot] == 0x00){
//            UCA1IE &= ~UCTXIE;
//            direct_iot = 0; // Set Array index to first location
//            //t1 = 0;
//        }
    }break;
    default:
        break;
    }
//------------------------------------------------------------------------------
}

void Init_Serial_UCA0(char speed){
//------------------------------------------------------------------------------
// TX error (%) RX error (%)
// BRCLK Baudrate UCOS16 UCBRx UCFx UCSx neg pos neg pos
// 8000000 4800 1 104 2 0xD6 -0.08 0.04 -0.10 0.14
// 8000000 9600 1 52 1 0x49 -0.08 0.04 -0.10 0.14
// 8000000 19200 1 26 0 0xB6 -0.08 0.16 -0.28 0.20
// 8000000 57600 1 8 10 0xF7 -0.32 0.32 -1.00 0.36
// 8000000 115200 1 4 5 0x55 -0.80 0.64 -1.12 1.76
// 8000000 460800 0 17 0 0x4A -2.72 2.56 -3.76 7.28
//------------------------------------------------------------------------------
// Configure eUSCI_A0 for UART mode
    UCA0CTLW0 = 0;
    UCA0CTLW0 |= UCSWRST ; // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK; // Set SMCLK as fBRCLK
    UCA0CTLW0 &= ~UCMSB; // MSB, LSB select
    UCA0CTLW0 &= ~UCSPB; // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA0CTLW0 &= ~UCPEN; // No Parity
    UCA0CTLW0 &= ~UCSYNC;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 |= UCMODE_0;
    // BRCLK Baudrate UCOS16 UCBRx UCFx UCSx neg pos neg pos
    // 8000000 115200 1 4 5 0x55 -0.80 0.64 -1.12 1.76
    // UCA?MCTLW = UCSx + UCFx + UCOS16
    UCA0BRW = 4 ; // 115,200 baud
    UCA0MCTLW = 0x5551 ;
    UCA0CTLW0 &= ~UCSWRST ; // release from reset
    UCA0TXBUF = 0x00; // Prime the Pump
    UCA0IE |= UCRXIE; // Enable RX interrupt
//------------------------------------------------------------------------------
}

void Init_Serial_UCA1(char speed){
//------------------------------------------------------------------------------
// TX error (%) RX error (%)
// BRCLK Baudrate UCOS16 UCBRx UCFx UCSx neg pos neg pos
// 8000000 4800 1 104 2 0xD6 -0.08 0.04 -0.10 0.14
// 8000000 9600 1 52 1 0x49 -0.08 0.04 -0.10 0.14
// 8000000 19200 1 26 0 0xB6 -0.08 0.16 -0.28 0.20
// 8000000 57600 1 8 10 0xF7 -0.32 0.32 -1.00 0.36
// 8000000 115200 1 4 5 0x55 -0.80 0.64 -1.12 1.76
// 8000000 460800 0 17 0 0x4A -2.72 2.56 -3.76 7.28
//------------------------------------------------------------------------------
// Configure eUSCI_A0 for UART mode
    UCA1CTLW0 = 0;
    UCA1CTLW0 |= UCSWRST ; // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK; // Set SMCLK as fBRCLK
    UCA1CTLW0 &= ~UCMSB; // MSB, LSB select
    UCA1CTLW0 &= ~UCSPB; // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA1CTLW0 &= ~UCPEN; // No Parity
    UCA1CTLW0 &= ~UCSYNC;
    UCA1CTLW0 &= ~UC7BIT;
    UCA1CTLW0 |= UCMODE_0;
    // BRCLK Baudrate UCOS16 UCBRx UCFx UCSx neg pos neg pos
    // 8000000 115200 1 4 5 0x55 -0.80 0.64 -1.12 1.76
    // UCA?MCTLW = UCSx + UCFx + UCOS16
    UCA1BRW = 4 ; // 115,200 baud
    UCA1MCTLW = 0x5551 ;
    UCA1CTLW0 &= ~UCSWRST ; // release from reset
    UCA1TXBUF = 0x00; // Prime the Pump
    UCA1IE |= UCRXIE; // Enable RX interrupt
//------------------------------------------------------------------------------
}

void USCI_A1_transmit(void){ // Transmit Function for USCI_A1
// Contents must be in process_buffer
// End of Transmission is identified by NULL character in process_buffer
// process_buffer includes Carriage Return and Line Feed
    //usb_rx_wr = 0;
    strcpy(display_line[1], "          ");
    display_changed = TRUE;
    update_display = TRUE;
    UCA1IE |= UCTXIE; // Enable TX interrupt
}

void USCI_A0_transmit(void){ // Transmit Function for USCI_A1
// Contents must be in process_buffer
// End of Transmission is identified by NULL character in process_buffer
// process_buffer includes Carriage Return and Line Feed
    //usb_rx_wr = 0;
    strcpy(display_line[1], "          ");
    display_changed = TRUE;
    update_display = TRUE;
    UCA0IE |= UCTXIE; // Enable TX interrupt
}



//-----------------------------------------------------------------
// Hex to BCD Conversion
// Convert a Hex number to a BCD for display on an LCD or monitor
//
//-----------------------------------------------------------------
void HEXtoBCD(int hex_value){
    int value;
    int i;
    for(i=0; i<4; i++) {
        adc_char[i] = '0';
    }
    while (hex_value > 999){
        hex_value = hex_value - 1000;
        value = value + 1;
        adc_char[0] = 0x30 + value;
    }
    value = 0;
    while (hex_value > 99){
        hex_value = hex_value - 100;
        value = value + 1;
        adc_char[1] = 0x30 + value;
    }
    value = 0;
    while (hex_value > 9){
        hex_value = hex_value - 10;
        value = value + 1;
        adc_char[2] = 0x30 + value;
    }
    adc_char[3] = 0x30 + hex_value;
}
//-----------------------------------------------------------------

//-------------------------------------------------------------
// ADC Line insert
// Take the HEX to BCD value in the array adc_char and place it
// in the desired location on the desired line of the display.
// char line => Specifies the line 1 thru 4
// char location => Is the location 0 thru 9
//
//-------------------------------------------------------------
void adc_line(char line, char location){
//-------------------------------------------------------------
    int i;

    unsigned int real_line;
    real_line = line - 1;
    for(i=0; i < 4; i++) {
//        display_line[real_line][i+location] = adc_char[i];
//        display_changed = TRUE;
//        update_display = TRUE;
    }
}
//-------------------------------------------------------------

