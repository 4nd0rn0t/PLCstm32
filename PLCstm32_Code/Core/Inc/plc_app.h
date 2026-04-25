/*
 * plc_app.h
 *
 *  Created on: 15 abr 2026
 *      Author: Yo
 */

#ifndef INC_PLC_APP_H_
#define INC_PLC_APP_H_


#include <config.h>
#include "main.h"


void plc_init(void);
void plc_scan(void);

uint8_t detectar_pulsacion(void);
void actualizar_modo(void);
void parpadeo_led(void);

void rng_init(void);



#endif /* INC_PLC_APP_H_ */
