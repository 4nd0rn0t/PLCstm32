/*
 * pcl_nucleo.h
 *
 *  Created on: 14 abr 2026
 *      Author: Yo
 */

#ifndef INC_PLC_NUCLEO_H_
#define INC_PLC_NUCLEO_H_

#include <config.h>

#define MAX_BLOCKS	 200  		// 200 comprobar final si afecta ,300 Ajusta según RAM, ver si ampliar

#define FLAG_PREV  0x01
#define FLAG_OUT   0x02

extern uint32_t state[MAX_BLOCKS];
extern uint8_t  flags[MAX_BLOCKS];
extern uint8_t  B[MAX_BLOCKS];
extern uint32_t state2[MAX_BLOCKS];

void execute_block(Block16 *b, uint8_t i);


uint32_t get_uid(void);


#endif /* INC_PLC_NUCLEO_H_ */
