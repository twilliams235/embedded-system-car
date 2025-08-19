/*
 * timers.h
 *
 *  Created on: Oct 09, 2023
 *      Author: tylerwilliams
 */



#define TB0CCR0_INTERVAL    (5000) // 8,000,000 / 8 / 8 / (1 / 50msec)
#define TB0CCR1_INTERVAL    (50000) // 8,000,000 / 8 / 8 / (1 / 800msec)
#define TB0CCR2_INTERVAL    (50000) // 8,000,000 / 8 / 8 / (1 / 800msec)
#define P4PUD               (P4OUT)
#define P2PUD               (P2OUT)

#define PWM_PERIOD                  (TB3CCR0)
#define LCD_BACKLITE_DIMING         (TB3CCR1)
#define SLOW                        (9500)
#define FAST                        (40000)
#define PERCENT_100                 (50000)
#define PERCENT_80                  (45000)
#define LOOKING                     (0)
#define FOUND                       (1)

