/*
 * mcp23017.h
 *
 *  Created on: 28 mar 2026
 *      Author: Yo
 */

#ifndef PLC_CONFIG_H
#define PLC_CONFIG_H

#include <stdint.h>



/* =========================================================
			Definición de flags del systema
========================================================= */

#define MCP23017_OK_FLAG   (1 << 0)
#define RESERVA_1_FLAG     (1 << 1)
#define RESERVA_2_FLAG     (1 << 2)
#define RESERVA_3_FLAG     (1 << 3)
#define RESERVA_4_FLAG     (1 << 4)
#define RESERVA_5_FLAG     (1 << 5)
#define RESERVA_6_FLAG     (1 << 6)
#define RESERVA_7_FLAG     (1 << 7)

/* =========================================================
	Definición de negaciones entradas y base tiempos
========================================================= */

#define NEG1 (1 << 0)
#define NEG2 (1 << 1)
#define NEG3 (1 << 2)
#define NEG4 (1 << 3)

/* Base de tiempo (2 bits) */
#define CFG_TIME_MASK  (3 << 4)

#define CFG_TIME_10MS  (0 << 4)
#define CFG_TIME_1S    (1 << 4)
#define CFG_TIME_1MIN  (2 << 4)

/* =========================================================
			CONFIGURACIÓN
========================================================= */

#define SIG_NONE	 0xFFFF
#define CFG_NONE     0x00
#define PARAM_NONE   0x0000

/* =========================================================
			CODIFICACIÓN DE SEÑALES
========================================================= */

#define SIG_TYPE(sig)   ((sig) >> 12)
#define SIG_INDEX(sig)  ((sig) & 0x0FFF)

#define SIG_TYPE_I 0
#define SIG_TYPE_Q 1
#define SIG_TYPE_M 2
#define SIG_TYPE_B 3

#define SIG_I(n)   ((SIG_TYPE_I << 12) | (n))
#define SIG_Q(n)   ((SIG_TYPE_Q << 12) | (n))
#define SIG_M(n)   ((SIG_TYPE_M << 12) | (n))
#define SIG_B(n)   ((SIG_TYPE_B << 12) | (n))

/* =========================================================
 	 	 	 Variable global (declaración)
========================================================= */

extern volatile uint8_t system_flags;		//  Flag del systema

extern volatile  uint32_t I;       			// Entradas I0-I31 (bitwise)
extern volatile  uint32_t Q;        		// Salidas Q0-Q31  (bitwise)
extern volatile  uint64_t M;       			// Marcas M0-M63  (bitwise)

/* =========================================================
 	 	 	 Estructuras
========================================================= */

typedef enum {
    MODE_STOP = 0,
	MODE_RUN,
    MODE_ERROR,
    MODE_CARGA
 } SystemMode_t;

/* =========================================================
			ESTRUCTURA DE BLOQUES
========================================================= */

typedef struct {
      uint8_t op;
      uint8_t cfg;
      uint16_t in1;
      uint16_t in2;
      uint16_t in3;
      uint16_t in4;
      uint16_t out;
      uint16_t param_A;
      uint16_t param_B;
  } Block16;

/* =========================================================
			OPCODES
========================================================= */

typedef enum {
      OP_AND,
      OP_OR,
      OP_NAND,
      OP_NOR,
      OP_XOR,
      OP_NOT,

      OP_RISE,
      OP_FALL,

      OP_TON,
      OP_TOF,
      OP_TP,
	  OP_T_BLINK,
	  OP_T_SWEEP,
	  OP_T_SWEEP_EDGE,
	  OP_T_RANDOM,

      OP_CTU,
      OP_CTD,

      OP_RS,
      OP_RND
  } OPCODE_t;

/* =========================================================
 	 	 	 Variable global
========================================================= */

extern volatile uint32_t msTicks;
uint32_t millis(void);

extern volatile SystemMode_t mode;
















#endif
