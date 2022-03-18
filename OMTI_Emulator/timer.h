/*
 * timer.h
 *
 * Created: 05.03.2022 20:20:58
 *  Author: marek.hummel
 */ 


#ifndef TIMER_H_
#define TIMER_H_



#define PRESCALE0_STOP	((0 << CS02) | (0 << CS01) | (0 << CS00))
#define PRESCALE0_1		((0 << CS02) | (0 << CS01) | (1 << CS00))
#define PRESCALE0_8		((0 << CS02) | (1 << CS01) | (0 << CS00))
#define PRESCALE0_64	((0 << CS02) | (1 << CS01) | (1 << CS00))
#define PRESCALE0_256	((1 << CS02) | (0 << CS01) | (0 << CS00))
#define PRESCALE0_1024	((1 << CS02) | (0 << CS01) | (1 << CS00))

void timer_init(void);


#endif /* TIMER_H_ */