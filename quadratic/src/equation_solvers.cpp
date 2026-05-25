/**
 * @file
 *
 * @brief Equation solvers
 */

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "equation_solvers.h"

// static const double DOUBLE_EPS = 1e-9;
#define DOUBLE_EPS (1e-9)

static inline int is_zero(double n) {
	return fabs(n) < DOUBLE_EPS;
}

static const uint64_t DOUBLE_NAN_SPEC = 0x7ff0LL << 48;

int is_finite(double n) {
	uint64_t d = 0;

	static_assert (sizeof(d) == sizeof(n), "sizeof d and n");

	memcpy(&d, &n, sizeof(n));

	if ((d & DOUBLE_NAN_SPEC) == DOUBLE_NAN_SPEC) {
		return 0;
	}

	return 1;
}

static inline int is_polynom_finite(struct polynom *pol) {
	assert (pol);
	assert (pol->nCoeffs < MAX_COEFFICIENTS);

	for (int i = 0; i < pol->nCoeffs; i++) {
		if (!is_finite(pol->coeffs[i]))
			return 0;
	}

	return 1;
}

#define ES_SWAP_BYTES(p1, p2, dtype) {		\
	dtype dt1 = 0;				\
	memcpy(&dt1, p1, sizeof(dt1));		\
	memcpy(p1, p2, sizeof(dt1));		\
	memcpy(p2, &dt1, sizeof(dt1));		\
	p1 += sizeof(dt1);			\
	p2 += sizeof(dt1);			\
}

static int vectorized_swap(char *p1, char *p2, size_t sz) {
	size_t processed_sz = 0;

	while (processed_sz + sizeof(uint64_t) <= sz) {
		ES_SWAP_BYTES(p1, p2, uint64_t);
		processed_sz += sizeof(uint64_t);
	}

	if (processed_sz + sizeof(uint32_t) <= sz) {
		ES_SWAP_BYTES(p1, p2, uint32_t);
		processed_sz += sizeof(uint32_t);
	}

	if (processed_sz + sizeof(uint16_t) <= sz) {
		ES_SWAP_BYTES(p1, p2, uint16_t);
		processed_sz += sizeof(uint16_t);
	}

	while (processed_sz + sizeof(char) <= sz) {
		ES_SWAP_BYTES(p1, p2, char);
		processed_sz += sizeof(char);
	}

	return 0;
}

#undef SWAP_BYTES

enum solving_status 
solve_polynomial(struct polynom pol,
                 struct polynom_roots *roots) {

	assert (roots != NULL);
	assert (pol.nCoeffs >= 0 && pol.nCoeffs <= MAX_COEFFICIENTS);
	assert (is_polynom_finite(&pol));

	if (pol.nCoeffs == 0) {
		roots->nRoots = SQ_INF_ROOTS;
		return SOLVING_SUCCESS;
	}

	switch (pol.nCoeffs) {
		case 0:
			roots->nRoots = SQ_INF_ROOTS;
			return SOLVING_SUCCESS;
		case 1:
			roots->nRoots = (is_zero(pol.coeffs[0]) ? SQ_INF_ROOTS : 0);
			return SOLVING_SUCCESS;
		case 2:
			return solve_linear(pol, roots);
		case 3:
			return solve_quadratic(pol, roots);
		default:
			return SOLVING_NOT_IMPLEMENTED;
	}

	return SOLVING_ERROR;
}

static const int LINEAR_COEFF_K = 0;
static const int LINEAR_COEFF_B = 1;

enum solving_status solve_linear(struct polynom pol, struct polynom_roots *roots) {
	assert (roots		!= NULL);
	assert (pol.nCoeffs	== 2);
	assert (is_polynom_finite(&pol));

	double	k = pol.coeffs[LINEAR_COEFF_K],
		b = pol.coeffs[LINEAR_COEFF_B];

	if (is_zero(k)) {
		if (is_zero(b)) {
			roots->nRoots = SQ_INF_ROOTS;
		} else {
			roots->nRoots = 0;
		}

		return SOLVING_SUCCESS;
	}

	double x = (-b) / k;

	roots->nRoots	= 1;
	roots->roots[0]	= x;

	return SOLVING_SUCCESS;
}

static const int QUADRATIC_COEFF_A = 0;
static const int QUADRATIC_COEFF_B = 1;
static const int QUADRATIC_COEFF_C = 2;

enum solving_status solve_quadratic(struct polynom pol, struct polynom_roots *roots) {
	assert (roots		!= NULL);
	assert (pol.nCoeffs	== 3);
	assert (is_polynom_finite(&pol));

	double	a = pol.coeffs[QUADRATIC_COEFF_A],
		b = pol.coeffs[QUADRATIC_COEFF_B],
		c = pol.coeffs[QUADRATIC_COEFF_C];

	double discriminant = 0;

	if (is_zero(a)) {
		struct polynom linear_pol = {
			.nCoeffs = 2,
			.coeffs = { pol.coeffs[1], pol.coeffs[2] },
		};

		return solve_linear(linear_pol, roots);
	}

	discriminant = b * b - 4 * a * c;

	double x1 = 0, x2 = 0;

	if (is_zero(discriminant)) {
		roots->nRoots = 1;

		x1 = -b / (2 * a);
		x2 = x1;
	} else if (discriminant < 0) {
		roots->nRoots = 0;
	} else {
		roots->nRoots = 2;

       		discriminant = sqrt(discriminant);
		x1 = (-b - discriminant) / (2 * a);
		x2 = (-b + discriminant) / (2 * a);

		// Return in ascending order
		if (a < 0) {
			vectorized_swap((char *)&x1, (char *)&x2, sizeof(x1));
		}
	}

	roots->roots[0] = x1;
	roots->roots[1] = x2;

	return SOLVING_SUCCESS;
}

