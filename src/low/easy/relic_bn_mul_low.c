/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (c) 2009 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or modify it under the
 * terms of the version 2.1 (or later) of the GNU Lesser General Public License
 * as published by the Free Software Foundation; or version 2.0 of the Apache
 * License as published by the Apache Software Foundation. See the LICENSE files
 * for more details.
 *
 * RELIC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the LICENSE files for more details.
 *
 * You should have received a copy of the GNU Lesser General Public or the
 * Apache License along with RELIC. If not, see <https://www.gnu.org/licenses/>
 * or <https://www.apache.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the low-level multiple precision integer multiplication
 * functions.
 *
 * @ingroup bn
 */

#include "relic_bn.h"
#include "relic_bn_low.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

/**
 * Accumulates a double precision digit in a triple register variable.
 *
 * @param[in,out] R2		- most significant word of the triple register.
 * @param[in,out] R1		- middle word of the triple register.
 * @param[in,out] R0		- lowest significant word of the triple register.
 * @param[in] A				- the first digit to multiply.
 * @param[in] B				- the second digit to multiply.
 */
#define COMBA_STEP_MUL(R2, R1, R0, A, B)									\
	dig_t _r0, _r1;															\
	RLC_MUL_DIG(_r1, _r0, A, B);											\
	dig_t _r = (R1);														\
	(R0) += _r0;															\
	(R1) += (R0) < _r0;														\
	(R2) += (R1) < _r;														\
	(R1) += _r1;															\
	(R2) += (R1) < _r1;														\

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

dig_t bn_mula_low(dig_t *c, const dig_t *a, dig_t digit, int size) {
	dig_t _c, r0, r1, carry = 0;
	for (int i = 0; i < size; i++, a++, c++) {
		/* Multiply the digit *a by d and accumulate with the previous
		 * result in the same columns and the propagated carry. */
		RLC_MUL_DIG(r1, r0, *a, digit);
		_c = r0 + carry;
		carry = r1 + (_c < carry);
		/* Increment the column and assign the result. */
		*c = *c + _c;
		/* Update the carry. */
		carry += (*c < _c);
	}
	return carry;
}

dig_t bn_mul1_low(dig_t *c, const dig_t *a, dig_t digit, int size) {
	dig_t r0, r1, carry = 0;
	for (int i = 0; i < size; i++, a++, c++) {
		RLC_MUL_DIG(r1, r0, *a, digit);
		*c = r0 + carry;
		carry = r1 + (*c < carry);
	}
	return carry;
}

void bn_muln_low(dig_t *c, const dig_t *a, const dig_t *b, int size) {
	int i, j;
	const dig_t *tmpa, *tmpb;
	dig_t r0, r1, r2;

	r0 = r1 = r2 = 0;
	for (i = 0; i < size; i++, c++) {
		tmpa = a;
		tmpb = b + i;
		for (j = 0; j <= i; j++, tmpa++, tmpb--) {
			COMBA_STEP_MUL(r2, r1, r0, *tmpa, *tmpb);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	for (i = 0; i < size; i++, c++) {
		tmpa = a + i + 1;
		tmpb = b + (size - 1);
		for (j = 0; j < size - (i + 1); j++, tmpa++, tmpb--) {
			COMBA_STEP_MUL(r2, r1, r0, *tmpa, *tmpb);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
}

void bn_muld_low(dig_t *c, const dig_t *a, int sa, const dig_t *b, int sb,
		int l, int h) {
	int i, j, ta;
	const dig_t *tmpa, *tmpb;
	dig_t r0, r1, r2;

	c += l;

	r0 = r1 = r2 = 0;
	for (i = l; i < sb; i++, c++) {
		tmpa = a;
		tmpb = b + i;
		for (j = 0; j <= i; j++, tmpa++, tmpb--) {
			COMBA_STEP_MUL(r2, r1, r0, *tmpa, *tmpb);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	ta = 0;
	for (i = sb; i < sa; i++, c++) {
		tmpa = a + ++ta;
		tmpb = b + (sb - 1);
		for (j = 0; j < sb; j++, tmpa++, tmpb--) {
			COMBA_STEP_MUL(r2, r1, r0, *tmpa, *tmpb);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	for (i = sa; i < h; i++, c++) {
		tmpa = a + ++ta;
		tmpb = b + (sb - 1);
		for (j = 0; j < sa - ta; j++, tmpa++, tmpb--) {
			COMBA_STEP_MUL(r2, r1, r0, *tmpa, *tmpb);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
}
