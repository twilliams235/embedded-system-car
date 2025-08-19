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

// STATES Project 4 ======================================================================
#define NOT_OKAY (0)
#define OKAY (1)
#define DEBOUNCE_TIME (10000)
#define DEBOUNCE_RESTART (0)
#define PRESSED (0)
#define RELEASED (1)

// WHEELS_PROCESS STATES ================================================================
#define IDLE                ('I')
#define CONFIGSPEED         ('C')
#define FMOVE               ('F')
#define FADJUST             ('D')
#define INITSTOP            ('P')
#define STOP                ('S')
#define ALIGN               ('A')


#endif /* MACROS_H_ */
