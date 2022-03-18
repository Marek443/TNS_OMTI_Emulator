/*
 * Input.h
 *
 * Created: 17.03.2022 19:32:29
 *  Author: marek.hummel
 */ 


#ifndef INPUT_H_
#define INPUT_H_

typedef struct {
	uint8_t value;		// Aktualni hodnota vstupu, co bit to vstup
	uint8_t change;		// Priznak zmeny vstupu, co bit to vstup
} BUTTONS;

BUTTONS GetButtons(void);

void Input_Timer10mS_Tick(void);
void Input_Init(void);
void Input_Test(void);

#define BUTTON_SW1		(1 << 0)
#define BUTTON_SW2		(1 << 1)


#endif /* INPUT_H_ */
