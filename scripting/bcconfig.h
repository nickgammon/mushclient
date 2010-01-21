/*
* config.h
* The version of number.c that comes with GNU bc-1.06 exports symbols without
* the bc_ prefix. This header file fixes this without touching number.c or
* number.h (luckily, number.c already wants to include a config.h).
* Clients of number.c should include config.h before number.h.
*/

#include <string.h>

#define _zero_		bc_zero
#define _one_		bc_one
#define _two_		bc_two
#define num2str		bc_num2str
#define mul_base_digits bc_mul_base_digits
#define pn		bc_pn
#define pv		bc_pv

#define rt_warn		bc_warn
#define rt_error	bc_error
#define out_of_memory	bc_nomemory

/* bc_warn, bc_error, bc_nomemory should be provided by clients */
