/*=============================================================================
    Copyright (c) 2016 Paul Fultz II
    config.hpp
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef FIT_GUARD_CONFIG_HPP
#define FIT_GUARD_CONFIG_HPP


// Check for std version
#if __cplusplus >= 201402
#define FIT_HAS_STD_14 1
#else
#define FIT_HAS_STD_14 0
#endif

#if __cplusplus >= 201103
#define FIT_HAS_STD_11 1
#else
#define FIT_HAS_STD_11 0
#endif


// This determines if it safe to use inheritance for EBO. On every platform
// except clang, compilers have problems with ambigous base conversion. So
// this configures the library to use a different technique to achieve empty
// optimization.
#ifndef FIT_HAS_EBO
#ifdef __clang__
#define FIT_HAS_EBO 1
#else
#define FIT_HAS_EBO 0
#endif
#endif

// This configures the library whether expression sfinae can be used to detect
// callability of a function.
#ifndef FIT_NO_EXPRESSION_SFINAE
#ifdef _MSC_VER
#define FIT_NO_EXPRESSION_SFINAE 1
#else
#define FIT_NO_EXPRESSION_SFINAE 0
#endif
#endif

// This configures the library to use manual type deduction in a few places
// where it problematic on a few platforms.
#ifndef FIT_HAS_MANUAL_DEDUCTION
#if (defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define FIT_HAS_MANUAL_DEDUCTION 1
#else
#define FIT_HAS_MANUAL_DEDUCTION 0
#endif
#endif

// Whether the compiler has relaxed constexpr.
#ifndef FIT_HAS_RELAXED_CONSTEXPR
#ifdef __cpp_constexpr
#if __cpp_constexpr >= 201304
#define FIT_HAS_RELAXED_CONSTEXPR 1
#else
#define FIT_HAS_RELAXED_CONSTEXPR 0
#endif
#else
#define FIT_HAS_RELAXED_CONSTEXPR FIT_HAS_STD_14
#endif
#endif

// Whether the compiler supports generic lambdas
#ifndef FIT_HAS_GENERIC_LAMBDA
#if defined(__cpp_generic_lambdas) || defined(_MSC_VER)
#define FIT_HAS_GENERIC_LAMBDA 1
#else
#define FIT_HAS_GENERIC_LAMBDA FIT_HAS_STD_14
#endif
#endif

// Whether the compiler supports variable templates
#ifndef FIT_HAS_VARIABLE_TEMPLATES
#if defined(__clang__) && __clang_major__ == 3 && __clang_minor__ < 5
#define FIT_HAS_VARIABLE_TEMPLATES 0
#elif defined(__cpp_variable_templates)
#define FIT_HAS_VARIABLE_TEMPLATES 1
#else
#define FIT_HAS_VARIABLE_TEMPLATES FIT_HAS_STD_14
#endif
#endif

// Whether a constexpr function can use a void return type
#ifndef FIT_NO_CONSTEXPR_VOID
#if FIT_HAS_RELAXED_CONSTEXPR
#define FIT_NO_CONSTEXPR_VOID 0
#else
#define FIT_NO_CONSTEXPR_VOID 1
#endif
#endif

// Whether to use template aliases
#ifndef FIT_HAS_TEMPLATE_ALIAS
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 8
#define FIT_HAS_TEMPLATE_ALIAS 0
#else
#define FIT_HAS_TEMPLATE_ALIAS 1
#endif
#endif

// Whether evaluations of function in brace initialization is ordered from
// left-to-right.
#ifndef FIT_NO_ORDERED_BRACE_INIT
#if (defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9) || defined(_MSC_VER)
#define FIT_NO_ORDERED_BRACE_INIT 1
#else
#define FIT_NO_ORDERED_BRACE_INIT 0
#endif 
#endif

// Whether the compiler has trouble mangling some expressions used in
// decltype.
#ifndef FIT_HAS_MANGLE_OVERLOAD
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
#define FIT_HAS_MANGLE_OVERLOAD 0
#else
#define FIT_HAS_MANGLE_OVERLOAD 1
#endif
#endif

// Whether an incomplete 'this' pointer can be used in a trailing decltype.
#ifndef FIT_HAS_COMPLETE_DECLTYPE
#if !FIT_HAS_MANGLE_OVERLOAD || (defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define FIT_HAS_COMPLETE_DECLTYPE 0
#else
#define FIT_HAS_COMPLETE_DECLTYPE 1
#endif
#endif

// Some type expansion failures on gcc 4.6
#ifndef FIT_NO_TYPE_PACK_EXPANSION_IN_TEMPLATE
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
#define FIT_NO_TYPE_PACK_EXPANSION_IN_TEMPLATE 1
#else
#define FIT_NO_TYPE_PACK_EXPANSION_IN_TEMPLATE 0
#endif
#endif

// Whether to use std::default_constructible, it is a little buggy on gcc 4.6.
#ifndef FIT_NO_STD_DEFAULT_CONSTRUCTIBLE
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
#define FIT_NO_STD_DEFAULT_CONSTRUCTIBLE 1
#else
#define FIT_NO_STD_DEFAULT_CONSTRUCTIBLE 0
#endif
#endif

#endif
