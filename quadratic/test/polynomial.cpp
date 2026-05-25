#include "equation_solvers.h"

#ifdef USE_GTEST

#include <gtest/gtest.h>

#else /* USE_GTEST */

#include "test_machine.h"

#endif /* USE_GTEST */

void polynomial_test(struct polynom pol, struct polynom_roots roots);

void polynomial_test(struct polynom pol, struct polynom_roots roots) {
	struct polynom_roots ret_roots = {0};

	int ret = solve_polynomial(pol, &ret_roots);

	ASSERT_EQ(ret, 0);

	ASSERT_EQ(ret_roots.nRoots, roots.nRoots);

	if (ret_roots.nRoots == SQ_INF_ROOTS) {
		return;
	}

	for (int i = 0; i < ret_roots.nRoots; i++) {
		ASSERT_DOUBLE_EQ(ret_roots.roots[i], roots.roots[i]);
	}
}

TEST(PolynomialTest, OneCoeffEqInfRoots) {
	polynomial_test(
		(struct polynom)       { .nCoeffs = 1, .coeffs = {0} },
		(struct polynom_roots) { .nRoots = SQ_INF_ROOTS, .roots = {NAN} }
	);
}

TEST(PolynomialTest, OneCoeffEqNoRoots) {
	polynomial_test(
		(struct polynom) { .nCoeffs = 1, .coeffs = {1} },
		(struct polynom_roots) { .nRoots = 0, .roots = {NAN} }
	);
}

TEST(PolynomialTest, NormalLinearEq) {
	polynomial_test(
		(struct polynom) { .nCoeffs = 2, .coeffs = {1, 2} },
		(struct polynom_roots) { .nRoots = 1, .roots = {-2} }
	);
}

TEST(PolynomialTest, NormalQuadraticEq) {
	polynomial_test(
		(struct polynom) { .nCoeffs = 3, .coeffs = {1, -5, 6} },
		(struct polynom_roots) { .nRoots = 2, .roots = {2, 3} }
	);
}

TEST(PolynomialTest, NotImplementedEq) {
	struct polynom pol =  { .nCoeffs = 4, .coeffs = {0} };
	struct polynom_roots roots;

	int ret = solve_polynomial(pol, &roots);

	ASSERT_EQ(ret, (int)SOLVING_NOT_IMPLEMENTED);
}
