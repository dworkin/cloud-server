#ifndef _NKLIB_H_
#define _NKLIB_H_

#include <Continuation.h>
#include <Iterator.h>
#include <String.h>
#include <float.h>
#include <kfun.h>
#include <limits.h>
#include <trace.h>
#include <type.h>

#define NK_KFUN "/lib/util/nk_kfun"
#define NK_DATA_DIR "/usr/NK/data"
#define NK_ETC_DIR "/usr/NK/etc"

#define DICE_EXPR_COUNT 10000

#define ARRAY_ARITHMETIC_ADD 0
#define ARRAY_ARITHMETIC_SUBTRACT 1
#define ARRAY_ARITHMETIC_MULTIPLY 2
#define ARRAY_ARITHMETIC_DIVIDE 3

#define STRING_ALIGN_RIGHT 0
#define STRING_ALIGN_LEFT 1
#define STRING_ALIGN_CENTRE 2

#define STRING_JUSTIFY_FULL 0
#define STRING_JUSTIFY_LEFT 1
#define STRING_JUSTIFY_RIGHT 2

#define BOX_STRING_SINGLE 1
#define BOX_STRING_DOUBLE 2
#define BOX_STRING_ROUND 3
#define BOX_STRING_DOUBLE_HORIZONTAL 4

#define TOLERANCE 1e-8
#define MAX_ITERATIONS 100

#define TERM_ESC "\033["


#define ARRAY_LIB "/lib/array/Array"
#define Array object ARRAY_LIB

#define ARRAY_ARITHMETIC_REDUCER "/lib/array/ArithmeticReducer"
#define ArrayArithmeticReducer object ARRAY_ARITHMETIC_REDUCER

#define ARRAY_TO_LIST_REDUCER "/lib/array/ToListReducer"
#define ArrayToListReducer object ARRAY_TO_LIST_REDUCER

#define ARRAY_TO_MARKED_LIST_REDUCER "/lib/array/ToMarkedListReducer"
#define ArrayToMarkedListReducer object ARRAY_TO_MARKED_LIST_REDUCER

#define ARRAY_TABULATE_REDUCER "/lib/array/ArrayTabulate"
#define ArrayTabulate object ARRAY_TABULATE_REDUCER

#define ARRAY_TO_BOX_REDUCER "/lib/array/ToBoxReducer"
#define ToBoxReducer object ARRAY_TO_BOX_REDUCER


#define DICE_LIB "/lib/dice/Dice"
#define Dice object DICE_LIB

#define DICE_PROBABILITY_LIB "/lib/dice/DiceProbability"
#define DiceProbability object DICE_PROBABILITY_LIB

#define DICE_ROLLER_LIB "/lib/dice/DiceRoller"
#define DiceRoller object DICE_ROLLER_LIB

#define DICE_DATA_GENERATOR "/lib/dice/DiceDataGenerator"
#define DiceDataGenerator object DICE_DATA_GENERATOR


#define CHECK_EQUALS_LIB "/lib/CheckEquals"
#define CheckEquals object CHECK_EQUALS_LIB


#define FUNCTION_LIB "/lib/Function"
#define Function object FUNCTION_LIB


#define INTEGRATOR_LIB "/lib/integration/Integrator"
#define Integrator object INTEGRATOR_LIB

#define SIMPSONS_RULE_INTEGRATOR_LIB "/lib/integration/SimpsonsRuleIntegrator"
#define SimpsonsRuleIntegrator object SIMPSONS_RULE_INTEGRATOR_LIB


#define JSON_PARSER_LIB "/lib/JsonParser"
#define JsonParser object JSON_PARSER_LIB


#define NUMBER_LIB "/lib/Number"
#define Number object NUMBER_LIB


#define POLYNOMIAL_LIB "/lib/polynomial/Polynomial"
#define Polynomial object POLYNOMIAL_LIB

#define POLYNOMIAL_DERIVATIVE_ITERATOR "/lib/polynomial/DerivativeIterator"
#define PolynomialDerivativeIterator object POLYNOMIAL_DERIVATIVE_ITERATOR

#define POLYNOMIAL_STRING_REDUCER "/lib/polynomial/StringReducer"
#define PolynomialStringReducer object POLYNOMIAL_STRING_REDUCER

#define POLYNOMIAL_PROBABILITY_REDUCER "/lib/polynomial/ProbabilityReducer"
#define ProbabilityReducer object POLYNOMIAL_PROBABILITY_REDUCER


#define RATIONAL_LIB "/lib/rational/Rational"
#define Rational object RATIONAL_LIB


#define SEARCH_LIB "/lib/Search"
#define Search object SEARCH_LIB


#define SORT_LIB "/lib/Sort"
#define Sort object SORT_LIB


#define STATISTICS_LIB "/lib/statistics/Statistics"
#define Statistics object STATISTICS_LIB

#define CORRELATION_FUNCTION "/lib/statistics/Correlation"
#define Correlation object CORRELATION_FUNCTION

#define COVARIANCE_FUNCTION "/lib/statistics/Covariance"
#define Covariance object COVARIANCE_FUNCTION

#define DATA_VISITOR_FUNCTION "/lib/statistics/DataVisitor"
#define DataVisitor object DATA_VISITOR_FUNCTION

#define EXPONENTIAL_PROBABILITY_DENSITY_FUNCTION "/lib/statistics/ExponentialPDF"
#define ExponentialPDF object EXPONENTIAL_PROBABILITY_DENSITY_FUNCTION

#define GEOMETRIC_MEAN_FUNCTION "/lib/statistics/GeometricMean"
#define GeometricMean object GEOMETRIC_MEAN_FUNCTION

#define MEDIAN_FUNCTION "/lib/statistics/Median"
#define Median object MEDIAN_FUNCTION

#define MEDIAN_FUNCTION "/lib/statistics/Median"
#define Median object MEDIAN_FUNCTION

#define RANDOM_LIB "/lib/statistics/Random"
#define Random object RANDOM_LIB

#define STANDARD_DEVIATION_FUNCTION "/lib/statistics/StandardDeviation"
#define StandardDeviation object STANDARD_DEVIATION_FUNCTION

#define VARIANCE_FUNCTION "/lib/statistics/Variance"
#define Variance object VARIANCE_FUNCTION


#define LONGEST_STRING_FUNCTION "/lib/string/LongestString"
#define LongestString object LONGEST_STRING_FUNCTION

#define STRINGIFY_FUNCTION "/lib/string/Stringify"
#define Stringify object STRINGIFY_FUNCTION

#define TRIM_STRING_FUNCTION "/lib/string/TrimString"
#define TrimString object TRIM_STRING_FUNCTION

#define BOXIFY_STRING_FUNCTION "/lib/string/BoxifyString"
#define BoxifyString object BOXIFY_STRING_FUNCTION


#define TERMINAL_LIB "/lib/Terminal"
#define Terminal object TERMINAL_LIB

#endif
