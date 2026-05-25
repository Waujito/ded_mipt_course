/**
 * @file
 *
 * @brief Declarations of equation solvers
 */

#ifndef EQUATIONS_H
#define EQUATIONS_H

#include <string.h>
#include <stdint.h>

static const int SQ_INF_ROOTS = -1;

// static const int LINEAR_COEFFICIENTS_LEN = 2;
// static const int QUADRATIC_COEFFICIENTS_LEN = 3;
//
// static const int MAX_COEFFICIENTS = 10;
// static const int MAX_ROOTS = 10;
#define LINEAR_COEFFICIENTS_LEN		(2)
#define QUADRATIC_COEFFICIENTS_LEN	(3)
#define MAX_COEFFICIENTS		(10)
#define MAX_ROOTS			(10)

int is_finite(double n);

struct polynom {
	int nCoeffs;
	double coeffs[MAX_COEFFICIENTS];
};

struct polynom_roots {
	int nRoots;
	double roots[MAX_ROOTS];
};

/**
 * @brief An enumeration of all possible return values for equation solvers
 */
enum solving_status {
	SOLVING_SUCCESS		= 0,
	SOLVING_ERROR		= -1,
	SOLVING_NOT_IMPLEMENTED	= -2
};

/**
 *
 * @brief Attempts to find roots of a polynomial equation.
 *
 * Currently implemented root finders up to quadratic equation.
 *
 * @param [in] pol A polynom of \p nCoeffs coefficients
 * @param [out] roots A pointer to polynom_roots being filled. nRoots is the number of roots found, or SQ_INF_ROOTS if the equation has infinitely many solutions
 *
 * @return enum solving_status
 */
enum solving_status solve_polynomial(struct polynom pol, struct polynom_roots *roots);

/**
 * @brief Solves the linear equation \f$kx + b = 0\f$.
 *
 * @param [in] pol A polynom of two coefficients \f$k\f$ and \f$b\f$
 * @param [out] roots A pointer to polynom_roots being filled. nRoots is the number of roots found, or SQ_INF_ROOTS if the equation has infinitely many solutions
 *
 * @return enum solving_status
 */
enum solving_status solve_linear(struct polynom pol, struct polynom_roots *roots);

/**
 * @brief Solves a quadratic equation \f$ ax^2 + bx + c = 0 \f$.
 *
 * Calculates the discriminant using the formula:
 * \f[ D = b^2 - 4ac \f]
 *
 * Then calculates the roots:
 * \f[ x_{1,2} = \frac{-b \pm \sqrt{D}}{2a} \f]
 *
 * @param [in] pol A polynom of three coefficients \f$a\f$, \f$b\f$ and \f$c\f$
 * @param [out] roots A pointer to polynom_roots being filled. nRoots is the number of roots found, or SQ_INF_ROOTS if the equation has infinitely many solutions
 *
 * @return enum solving_status
 *
 * @note If \f$a\f$ is zero, although it is not a quadratic equation, the function will solve it as linear.
 */
enum solving_status solve_quadratic(struct polynom pol, struct polynom_roots *roots);

#endif /* EQUATIONS_H */
