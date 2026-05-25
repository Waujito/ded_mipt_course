
#ifdef USE_GTEST

#include <gtest/gtest.h>

#else /* USE_GTEST */

#include "test_machine.h"

#endif /* USE_GTEST */

#include "equation_solvers.h"

struct quadratic_coeffs {
	double a;
	double b;
	double c;
};

struct quadratic_roots {
	int nRoots;
	double x1;
	double x2;
};

void quadratic_test(struct quadratic_coeffs coeffs, struct quadratic_roots roots);

void quadratic_test(struct quadratic_coeffs coeffs, struct quadratic_roots roots) {
	struct polynom pol = {
		.nCoeffs = 3,
		.coeffs = { coeffs.a, coeffs.b, coeffs.c },
	};
	struct polynom_roots ret_roots = {0};

	int ret = solve_quadratic(pol, &ret_roots);

	ASSERT_EQ(ret, 0);

	ASSERT_EQ(ret_roots.nRoots, roots.nRoots);

	if (roots.nRoots >= 1) {
		ASSERT_DOUBLE_EQ(ret_roots.roots[0], roots.x1);
	}
	if (roots.nRoots >= 2) {
		ASSERT_DOUBLE_EQ(ret_roots.roots[1], roots.x2);
	}
}

TEST(EqTest, NormalEquation) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1, .b = -5, .c = 6 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = 2, .x2 = 3 }
	);
}

TEST(EqTest, NegativeCoefficients) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 2, .b = 3, .c = -2 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = -2, .x2 = 0.5 }
	);
}

TEST(EqTest, DoubleRoot) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1, .b = -6, .c = 9 },
		(struct quadratic_roots) { .nRoots = 1, .x1 = 3, .x2 = NAN }
	);
}

TEST(EqTest, NoRationalSolutions) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1, .b = 4, .c = 5 },
		(struct quadratic_roots) { .nRoots = 0, .x1 = NAN, .x2 = NAN }
	);
}

TEST(EqTest, Zero_b) { 
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1, .b = 0, .c = -9 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = -3, .x2 = 3 }
	);
}

TEST(EqTest, Zero_c) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1, .b = 5, .c = 0 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = -5, .x2 = 0 }
	);
}

TEST(EqTest, Zero_bc) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 4, .b = 0, .c = 0 },
		(struct quadratic_roots) { .nRoots = 1, .x1 = 0, .x2 = NAN }
	);
}

TEST(EqTest, Large_coeffs) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 1e6, .b = -4e6, .c = 3e6 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = 1, .x2 = 3 }
	);
}

TEST(EqTest, Negative_a) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = -1, .b = 1, .c = 2 },
		(struct quadratic_roots) { .nRoots = 2, .x1 = -1, .x2 = 2 }
	);
}

TEST(EqTest, Linear_eq) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 0, .b = 2, .c = -4 },
		(struct quadratic_roots) { .nRoots = 1, .x1 = 2, .x2 = NAN }
	);
}

TEST(EqTest, Linear_no_solution) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 0, .b = 0, .c = 7 },
		(struct quadratic_roots) { .nRoots = 0, .x1 = NAN, .x2 = NAN }
	);
}

TEST(EqTest, Linear_infinity_solutions) {
	quadratic_test(
		(struct quadratic_coeffs) { .a = 0, .b = 0, .c = 0 },
		(struct quadratic_roots) { 
			.nRoots = SQ_INF_ROOTS, .x1 = NAN, .x2 = NAN
		}
	);
}
