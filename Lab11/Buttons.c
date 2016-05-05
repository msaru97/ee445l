
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include "Buttons.h"
#include "Timer1A.h"
#include "TEC.h"

void Buttons_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0); // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;	  // 2) unlock PF0-3
  GPIO_PORTF_CR_R = 0xFF;						//		allow PF to be modified.
	GPIO_PORTF_PCTL_R &= ~0x0000FFFF; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x0F;      // 4) disable analog function on PF0-3
                                    // 5) no pullup for external switches
  GPIO_PORTF_DIR_R &= ~0x0F;        // 5) set direction to input
  GPIO_PORTF_AFSEL_R &= ~0x0F;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x0F;         // 7) enable digital port
	
	Timer1_Init();
}

uint16_t debounce[3];

#define CYCLE_WAIT 5

void Buttons_ReadInput(void) {
	int32_t data = GPIO_PORTF_DATA_R;
	
	if((data & 0x01) != 0)
		debounce[0] = ((data & 0x1)     )*CYCLE_WAIT;
	if((data & 0x02) != 0)
		debounce[1] = ((data & 0x2) >> 1)*CYCLE_WAIT;
	if((data & 0x04) != 0)
		debounce[2] = ((data & 0x4) >> 2)*CYCLE_WAIT;
}

void Buttons_Pressed(uint32_t button) {
	if(button == 0) { // Temp Down
		TEC_Temp_Down();
	}
	else if(button == 1) { // Temp Up
		TEC_Temp_Up();
	}
	else if (button == 2) { // Start/Stop
		if(TEC_Status()){
			TEC_Stop();
		} 
		else TEC_Start();
	}
}

void Buttons_1ms_Handler(void) {
	Buttons_ReadInput();
	for(uint32_t i = 0; i < 3; i += 1) {
		if(debounce[i] > 0) {
			debounce[i] -= 1;
			if(debounce[i] == 0) {
				Buttons_Pressed(i);
			}
		}
	}
}
