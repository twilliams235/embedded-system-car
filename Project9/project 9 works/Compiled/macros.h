/*
 * macros.h
 *
 *  Created on: Sep 14, 2023
 *      Author: tylerwilliams
 */

#ifndef MACROS_H_
#define MACROS_H_


#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define START_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01)
#define FALSE                   (0)
#define DOT                  (0x2E)

// STATES ======================================================================
#define NOT_OKAY            (0)
#define OKAY                (1)
#define DEBOUNCE_TIME       (10000)
#define DEBOUNCE_RESTART    (0)
#define PRESSED             (0)
#define RELEASED            (1)
#define HIGH                (1)
#define LOW                 (0)

// WHEELS_PROCESS STATES ================================================================
#define ALIGN               ('A')
#define CONFIGSPEED         ('C')
#define FADJUST             ('D')
#define ENTER               ('E')
#define FMOVE               ('F')
#define IDLE                ('I')
#define TTEADJUST           ('J')
#define EADJUST             ('K')
#define INITSTOP            ('P')
#define STOP                ('S')
#define TRACE               ('T')
#define TURNTOENTER         ('U')
#define TADJUST             ('Z')
#define BOTHON              ('B')
#define BOTHOFF             ('G')
#define LEFTOFF             ('H')
#define RIGHTOFF            ('L')
#define MEASURE             ('M')

// IOT PROCESS ============================================================================
#define RED_LED_ON          (1)
#define GRN_LED_OFF         (0)
#define BEGINNING (0)
#define SMALL_RING_SIZE (16)
#define BAUD                (115200)




#endif /* MACROS_H_ */
