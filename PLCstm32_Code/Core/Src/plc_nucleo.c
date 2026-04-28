/*
 * pcl_nucleo.c
 *
 *  Created on: 14 abr 2026
 *      Author: Yo
 */


#include "plc_nucleo.h"

uint32_t state[MAX_BLOCKS];
uint32_t state2[MAX_BLOCKS];
uint8_t  flags[MAX_BLOCKS];
uint8_t  B[MAX_BLOCKS];


uint32_t better_rand(void);


/* =========================================================
			ACCESO A SEÑALES lectura y escritura
========================================================= */

uint8_t read_signal(uint16_t sig)
{
	uint16_t idx = SIG_INDEX(sig);

    switch (SIG_TYPE(sig))
    {
        case SIG_TYPE_I:
            if (idx >= 32) return 0;
            return (I >> idx) & 1;

        case SIG_TYPE_Q:
            if (idx >= 32) return 0;
            return (Q >> idx) & 1;

        case SIG_TYPE_M:
            return (M >> idx) & 1;

        case SIG_TYPE_B:
            return B[idx];
    }

    return 0;
}

void write_signal(uint16_t sig, uint8_t val)
{
    uint16_t idx = SIG_INDEX(sig);

    switch (SIG_TYPE(sig))
    {
    	case SIG_TYPE_I:
    		if (idx >= 32) return;
            I = (I & ~(1UL << idx)) | ((uint32_t)val << idx);
            break;

        case SIG_TYPE_Q:
            if (idx >= 32) return;
            Q = (Q & ~(1UL << idx)) | ((uint32_t)val << idx);
            break;

        case SIG_TYPE_M:
            M = (M & ~(1ULL << idx)) | ((uint64_t)val << idx);
            break;
    }
}

/* =========================================================
			BASE DE TIEMPOS Para Temporizadores
========================================================= */

uint32_t get_time_base(uint8_t cfg)
{
	switch (cfg & CFG_TIME_MASK)
	{
		case CFG_TIME_10MS: return 10;
		case CFG_TIME_1S:   return 1000;
		case CFG_TIME_1MIN: return 60000;
	}

	return 1;
}

/* =========================================================
			FLANCOS
========================================================= */

uint8_t rising_edge(uint8_t i, uint8_t in)
{
	uint8_t prev = flags[i] & FLAG_PREV;
	flags[i] = (flags[i] & ~FLAG_PREV) | (in ? FLAG_PREV : 0);
    return (!prev && in);
}

uint8_t falling_edge(uint8_t i, uint8_t in)
{
	uint8_t prev = flags[i] & FLAG_PREV;
	flags[i] = (flags[i] & ~FLAG_PREV) | (in ? FLAG_PREV : 0);
	return (prev && !in);
}




/* =========================================================
		MOTOR DEL miniPLC
========================================================= */

void execute_block(Block16 *b, uint8_t i)
{
	uint8_t v1 = (b->in1 != SIG_NONE);
	uint8_t v2 = (b->in2 != SIG_NONE);
	uint8_t v3 = (b->in3 != SIG_NONE);
	uint8_t v4 = (b->in4 != SIG_NONE);

	uint8_t s1 = v1 ? read_signal(b->in1) : 0;
	uint8_t s2 = v2 ? read_signal(b->in2) : 0;
	uint8_t s3 = v3 ? read_signal(b->in3) : 0;
	uint8_t s4 = v4 ? read_signal(b->in4) : 0;

	    /* NEGACIONES cfg */
	if ((b->cfg & NEG1) && v1) s1 = !s1;
	if ((b->cfg & NEG2) && v2) s2 = !s2;
	if ((b->cfg & NEG3) && v3) s3 = !s3;
	if ((b->cfg & NEG4) && v4) s4 = !s4;

	uint8_t out = 0;

	switch (b->op)

	{

/* =========================================================
      		LOGICA
========================================================= */

	case OP_AND:
		{
			uint8_t r = 1, any = 0;
        	if (v1) { r &= s1; any = 1; }
        	if (v2) { r &= s2; any = 1; }
        	if (v3) { r &= s3; any = 1; }
        	if (v4) { r &= s4; any = 1; }
        	out = any ? r : 0;
        	break;
        }


	case OP_OR:
		{
			uint8_t r = 0, any = 0;
        	if (v1) { r |= s1; any = 1; }
        	if (v2) { r |= s2; any = 1; }
        	if (v3) { r |= s3; any = 1; }
        	if (v4) { r |= s4; any = 1; }
        	out = any ? r : 0;
        	break;
		}


	case OP_NAND:
     	 {
     		 uint8_t r = 1, any = 0;
             if (v1) { r &= s1; any = 1; }
             if (v2) { r &= s2; any = 1; }
             if (v3) { r &= s3; any = 1; }
             if (v4) { r &= s4; any = 1; }

             out = any ? !r : 0;
             break;
     	 }


	case OP_NOR:
		{
			uint8_t r = 0, any = 0;
			if (v1) { r |= s1; any = 1; }
			if (v2) { r |= s2; any = 1; }
			if (v3) { r |= s3; any = 1; }
			if (v4) { r |= s4; any = 1; }

			out = any ? !r : 0;
			break;
		}

	case OP_XOR: out = s1 ^ s2; break;
	case OP_NOT: out = !s1; break;


/* =========================================================
			FLANCOS
========================================================= */

	case OP_RISE:
	out = rising_edge(i, s1);
	break;

	case OP_FALL:
	out = falling_edge(i, s1);
	break;


/* =========================================================
			TON (ON delay)
========================================================= */

	case OP_TON:
		{
			if (b->param_A == 0)
			{
				out = 0;
				state[i] = 0;
				break;
			}


	uint32_t base = get_time_base(b->cfg);


	if (s1) {

		if (state[i] == 0)
			{
				state[i] = GetTick();

			}
				if ((GetTick() - state[i]) >= b->param_A * base)
					out = 1;
				} else {
					state[i] = 0;
					out = 0;
				}
	break;

		}

/* =========================================================
			TOF (OFF delay)
========================================================= */

	case OP_TOF:
		{
			if (b->param_A == 0)
			{
				out = 0;
				state[i] = 0;
				break;
			}

			{
			uint32_t base = get_time_base(b->cfg);

				if (s1) {
					out = 1;
						state[i] = GetTick();
						} else {
							//revisar posible mal
							out = ((GetTick() - state[i]) < b->param_A * base);
						}
					  	break;
			}
		}

/* =========================================================
          			TP ()
========================================================= */

	case OP_TP:
		{
			if (b->param_A == 0)
			{
				out = 0;
				state[i] = 0;
				break;
			}

		{
			uint32_t base = get_time_base(b->cfg);

				if (rising_edge(i, s1)) {
					state[i] = GetTick();
					flags[i] |= FLAG_OUT;
				}

				if ((GetTick() - state[i]) >= b->param_A * base)
					flags[i] &= ~FLAG_OUT;

				  	  out = (flags[i] & FLAG_OUT) != 0;
				  	  break;
				}
		}

/* =========================================================
					Intermitente
========================================================= */

	case OP_BLINK:
	{
	    uint32_t base = get_time_base(b->cfg);
	    uint32_t now  = GetTick();

	    // Reset total si IN=0
	    if (!s1)
	    {
	        flags[i] &= ~FLAG_OUT;
	        state[i] = 0;
	        out = 0;
	        break;
	    }

	    // Validación tiempos
	    if (b->param_A == 0 || b->param_B == 0)
	    {
	        flags[i] &= ~FLAG_OUT;
	        out = 0;
	        break;
	    }

	    // Inicialización (primer ciclo)
	    if (state[i] == 0)
	    {
	        state[i] = now;

	        if (s2)
	            flags[i] |= FLAG_OUT;   // empieza en ON
	        else
	            flags[i] &= ~FLAG_OUT;  // empieza en OFF
	    }

	    uint32_t elapsed = now - state[i];

	    if (flags[i] & FLAG_OUT)
	    {
	        // Estado ON
	        if (elapsed >= b->param_A * base)
	        {
	            flags[i] &= ~FLAG_OUT; // pasar a OFF
	            state[i] = now;
	        }
	    }
	    else
	    {
	        // Estado OFF
	        if (elapsed >= b->param_B * base)
	        {
	            flags[i] |= FLAG_OUT; // pasar a ON
	            state[i] = now;
	        }
	    }

	    out = (flags[i] & FLAG_OUT) != 0;
	    break;
	}

/* =========================================================
				Rele tipo barrido
========================================================= */

	case OP_SWEEP:
	{
	    uint32_t base = get_time_base(b->cfg);
	    uint32_t now  = GetTick();

	    // Si entrada = 0 → salida inmediata a 0 + reset
	    if (!s1)
	    {
	        flags[i] &= ~FLAG_OUT;
	        state[i] = 0;
	        out = 0;
	        break;
	    }

	    // Tiempo inválido
	    if (b->param_A == 0)
	    {
	        out = 0;
	        break;
	    }

	    // Flanco de subida → iniciar temporizador
	    if (state[i] == 0)
	    {
	        state[i] = now;
	        flags[i] |= FLAG_OUT; // activar salida
	    }

	    // Mientras esté activo
	    if ((now - state[i]) >= b->param_A * base)
	    {
	        flags[i] &= ~FLAG_OUT; // apagar al terminar
	    }

	    out = (flags[i] & FLAG_OUT) != 0;
	    break;
	}


/* =========================================================
			Relé de barrido activado por flancos
========================================================= */

	case OP_SWEEP_EDGE:
	{
	    uint32_t base = get_time_base(b->cfg);
	    uint32_t now  = GetTick();

	    uint32_t TL = b->param_A * base;
	    uint32_t TH = b->param_B * base;

	    // Detectar flanco de subida
	    if (rising_edge(i, s1))
	    {
	        state[i] = now;
	        flags[i] = 1; // estado TL
	    }

	    switch (flags[i])
	    {
	        case 0: // IDLE
	            out = 0;
	            break;

	        case 1: // TL (espera)
	            if ((now - state[i]) >= TL)
	            {
	                state[i] = now;
	                flags[i] = 2; // pasar a TH
	            }
	            out = 0;
	            break;

	        case 2: // TH (salida activa)
	            if ((now - state[i]) >= TH)
	            {
	                flags[i] = 0; // volver a idle
	            }
	            out = 1;
	            break;
	    }

	    break;
	}

/* =========================================================
				Generador aleatorio
========================================================= */

	case OP_RANDOM:
	{
	    uint32_t base = get_time_base(b->cfg);
	    uint32_t now  = GetTick();

	    uint32_t TH = b->param_A * base;
	    uint32_t TL = b->param_B * base;

	    // RESET total
	    if (!s1)
	    {
	        flags[i] = 0;
	        state[i] = 0;
	        state2[i] = 0;
	        out = 0;
	        break;
	    }

	    // FLANCO SUBIDA
	    if (rising_edge(i, s1))
	    {
	        state[i] = now;                       // tiempo inicio
	        state2[i] = better_rand() % (TH + 1);       // delay ON aleatorio
	        flags[i] = 1;                        // esperando ON
	    }

	    // FLANCO BAJADA
	    if (falling_edge(i, s1))
	    {
	        state[i] = now;                       // tiempo inicio
	        state2[i] = better_rand() % (TL + 1);       // delay OFF aleatorio
	        flags[i] = 2;                        // esperando OFF
	    }

	    uint32_t elapsed = now - state[i];

	    // Estado ON pendiente
	    if (flags[i] == 1)
	    {
	        if (elapsed >= state2[i])
	        {
	            out = 1;
	            flags[i] = 0;
	        }
	    }
	    // Estado OFF pendiente
	    else if (flags[i] == 2)
	    {
	        if (elapsed >= state2[i])
	        {
	            out = 0;
	            flags[i] = 0;
	        }
	    }

	    break;
	}

/* =========================================================
			CTU (count up)
========================================================= */

	case OP_CTU:
		{
			uint8_t reset = read_signal(b->in2);

			if (reset) {
				state[i] = 0;

			} else {

				if (rising_edge(i, s1)) {
					if (state[i] < b->param_A)
					state[i]++;
				}
			}

		  out = (state[i] >= b->param_A);
		  break;
		}

/* =========================================================
			CTD (count down)
========================================================= */

	case OP_CTD:
		{
			uint8_t reset = read_signal(b->in2);

			if (reset) {
				state[i] = b->param_A;  // recarga valor inicial

			} else {
				if (rising_edge(i, s1)) {
					if (state[i] > 0)
						state[i]--;
				}
			}

			out = (state[i] == 0);
			break;
		}

/* =========================================================
			RS (SET/RESET)
========================================================= */

	case OP_RS:
		{
			uint8_t q = (flags[i] & FLAG_OUT) != 0;

			if (s1) q = 1;
			if (s2) q = 0;

			if (q) flags[i] |= FLAG_OUT;

			else   flags[i] &= ~FLAG_OUT;

				out = q;
				break;
		}

/* =========================================================
			RANDOM
========================================================= */

	case OP_RND:
		{
			uint16_t lfsr = state[i];
			if (!lfsr) lfsr = 0xACE1;

				lfsr ^= lfsr << 7;
				lfsr ^= lfsr >> 9;
				lfsr ^= lfsr << 8;

				state[i] = lfsr;
				out = lfsr & 1;
				break;
		}

	}

	B[i] = out;

	if (b->out != SIG_NONE)
	write_signal(b->out, out);

}
