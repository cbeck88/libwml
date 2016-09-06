#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor.hpp>

/***
 * FOREACH macro
 */

#define FIRST(X, ...) X
#define SECOND(X, Y, ...) Y
#define THIRD(X, Y, Z, ...) Z
#define FOURTH(X, Y, Z, W, ...) W

// Preprocessor macro to apply a different macro to each element of a sequence
#define DOUBLE_PARENS_ONE(...) ((__VA_ARGS__)) DOUBLE_PARENS_TWO
#define DOUBLE_PARENS_TWO(...) ((__VA_ARGS__)) DOUBLE_PARENS_ONE
#define DOUBLE_PARENS_ONE_END
#define DOUBLE_PARENS_TWO_END
#define DOUBLE_PARENS(SEQ) BOOST_PP_CAT(DOUBLE_PARENS_ONE SEQ, _END)


#define APPLY_MACRO(R, MACRO, TUPLE)                                           \
  MACRO TUPLE

#define FOR_EACH(MACRO, SEQ)                                                   \
  BOOST_PP_SEQ_FOR_EACH(APPLY_MACRO, MACRO, DOUBLE_PARENS(SEQ))

/* quick test of macros */

#define TEST_SUM_TEN(ONE, TWO) \
static_assert( ONE + TWO == 10 , " sum check failed");

FOR_EACH(TEST_SUM_TEN, (5, 5)(6, 4)(3, 7)(8, 2))

#define TEST_SUM_ZERO(ONE, TWO) \
static_assert( ONE + TWO == 0 , " sum check failed");

FOR_EACH(TEST_SUM_ZERO, (1+2, -3)( 1+ 1, -2)(1+2+3,4-10))

#define TEST_SUM_FIVE(ONE, TWO) \
static_assert( ONE + TWO == 5 , " sum check failed");

FOR_EACH(TEST_SUM_FIVE, (1+2, 2)(2, 1+2)(5+5, 5-10)(-1+3,3))

#undef TEST_SUM_TEN
#undef TEST_SUM_ZERO
#undef TEST_SUM_FIVE

// Second version which incorporates "data". The DATA parameter is duplicated at
// start of each call to macro.

#define EXPAND(...) __VA_ARGS__
#define PREPEND_ARG(R, DATA, Z)                                                \
 ((DATA, EXPAND Z))

#define PREPEND_DATA(DATA, SEQ)                                                \
BOOST_PP_SEQ_FOR_EACH(PREPEND_ARG, DATA, DOUBLE_PARENS(SEQ))

// inline void ignore_me() {
//   struct { int a; int b; int c; } s BOOST_PP_SEQ_FOR_EACH(THIRD, , PREPEND_DATA(10, (5, 5))) ;
// }

#define FOR_EACH_DATA(MACRO, DATA, SEQ)                                        \
  BOOST_PP_SEQ_FOR_EACH(APPLY_MACRO, MACRO, PREPEND_DATA(DATA, SEQ))

/* quick test of macros */

#define TEST_SUM(GOAL, ONE, TWO) \
static_assert(GOAL == ONE + TWO, "sum check failed");

FOR_EACH_DATA(TEST_SUM, 10, (5, 5)(6,4)(3, 7)(8, 2));
FOR_EACH_DATA(TEST_SUM, 0, (1+1, -2)(2+2, -4)(1+2+3-4,-10+8));
FOR_EACH_DATA(TEST_SUM, -7, (3, -10)(-14, 21-14));

#undef TEST_SUM
