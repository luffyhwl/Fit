/*=============================================================================
    Copyright (c) 2012 Paul Fultz II
    conditional.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef BOOST_FIT_GUARD_FUNCTION_CONDITIONAL_H
#define BOOST_FIT_GUARD_FUNCTION_CONDITIONAL_H

/// conditional
/// ===========
/// 
/// Description
/// -----------
/// 
/// The `conditional` function adaptor combines several functions together. If
/// the first function can not be called, then it will try to call the next
/// function. This can be very useful when overloading functions using
/// template constraints(such as with `enable_if`).
/// 
/// Note: This is different than the [`match`](match.md) function adaptor, which
/// can lead to ambiguities. Instead, `conditional` will call the first function
/// that is callable, regardless if there is another function that could be
/// called as well.
/// 
/// Synopsis
/// --------
/// 
///     template<class... Fs>
///     constexpr conditional_adaptor<Fs...> conditional(Fs... fs);
/// 
/// Requirements
/// ------------
/// 
/// Fs must be:
/// 
/// * [ConstCallable](concepts.md#constcallable)
/// * MoveConstructible
/// 
/// Example
/// -------
/// 
///     struct for_ints
///     {
///         void operator()(int) const
///         {
///             printf("Int\n");
///         }
///     };
/// 
///     struct for_floats
///     {
///         void operator()(float) const
///         {
///             printf("Float\n");
///         }
///     };
/// 
///     conditional(for_ints(), for_floats())(3.0);
/// 
/// This will print `Int` because the `for_floats` function object won't ever be
/// called. Due to the conversion rules in C++, the `for_ints` function can be
/// called on floats, so it is chosen by `conditional` first, even though
/// `for_floats` is a better match.
/// 
/// So, the order of the functions in the `conditional_adaptor` are very important
/// to how the function is chosen.

#include <boost/fit/reveal.hpp>
#include <boost/fit/detail/callable_base.hpp>
#include <boost/fit/detail/delegate.hpp>
#include <boost/fit/detail/join.hpp>
#include <boost/fit/detail/seq.hpp>
#include <boost/fit/detail/make.hpp>
#include <boost/fit/detail/static_const_var.hpp>

namespace boost { namespace fit {

namespace detail {

template<class F1, class F2>
struct conditional_kernel : F1, F2
{
    BOOST_FIT_INHERIT_DEFAULT(conditional_kernel, F1, F2)

    template<class A, class B,
        BOOST_FIT_ENABLE_IF_CONVERTIBLE(A, F1),
        BOOST_FIT_ENABLE_IF_CONVERTIBLE(B, F2)>
    constexpr conditional_kernel(A&& f1, B&& f2) : F1(BOOST_FIT_FORWARD(A)(f1)), F2(BOOST_FIT_FORWARD(B)(f2))
    {}

    template<class X,
        class=typename std::enable_if<
        BOOST_FIT_IS_CONVERTIBLE(X, F1) && 
        BOOST_FIT_IS_DEFAULT_CONSTRUCTIBLE(F2)
    >::type>
    constexpr conditional_kernel(X&& x) : F1(BOOST_FIT_FORWARD(X)(x))
    {} 

    template<class... Ts>
    struct select
    : std::conditional
    <
        is_callable<F1, Ts...>::value, 
        F1,
        F2
    >
    {};

    BOOST_FIT_RETURNS_CLASS(conditional_kernel);

    template<class... Ts, class F=typename select<Ts...>::type>
    constexpr BOOST_FIT_SFINAE_RESULT(typename select<Ts...>::type, id_<Ts>...) 
    operator()(Ts && ... x) const
    BOOST_FIT_SFINAE_RETURNS
    (
        BOOST_FIT_RETURNS_STATIC_CAST(const F&)(*BOOST_FIT_CONST_THIS)(BOOST_FIT_FORWARD(Ts)(x)...)
    );
};
}

template<class F, class... Fs>
struct conditional_adaptor 
: detail::conditional_kernel<F, BOOST_FIT_JOIN(conditional_adaptor, Fs...) >
{
    typedef conditional_adaptor fit_rewritable_tag;
    typedef BOOST_FIT_JOIN(conditional_adaptor, Fs...) kernel_base;
    typedef detail::conditional_kernel<F, kernel_base > base;

    BOOST_FIT_INHERIT_DEFAULT(conditional_adaptor, base)

    template<class X, class... Xs, 
        BOOST_FIT_ENABLE_IF_CONSTRUCTIBLE(base, X, kernel_base), 
        BOOST_FIT_ENABLE_IF_CONSTRUCTIBLE(kernel_base, Xs...)>
    constexpr conditional_adaptor(X&& f1, Xs&& ... fs) 
    : base(BOOST_FIT_FORWARD(X)(f1), kernel_base(BOOST_FIT_FORWARD(Xs)(fs)...))
    {}

    template<class X, class... Xs, 
        BOOST_FIT_ENABLE_IF_CONSTRUCTIBLE(base, X)>
    constexpr conditional_adaptor(X&& f1) 
    : base(BOOST_FIT_FORWARD(X)(f1))
    {}

    struct failure
    : failure_for<F, Fs...>
    {};
};

template<class F>
struct conditional_adaptor<F> : F
{
    typedef conditional_adaptor fit_rewritable_tag;
    BOOST_FIT_INHERIT_CONSTRUCTOR(conditional_adaptor, F);

    struct failure
    : failure_for<F>
    {};
};

template<class F1, class F2>
struct conditional_adaptor<F1, F2> 
: detail::conditional_kernel<F1, F2>
{
    typedef detail::conditional_kernel<F1, F2> base;
    typedef conditional_adaptor fit_rewritable_tag;
    BOOST_FIT_INHERIT_CONSTRUCTOR(conditional_adaptor, base);

    struct failure
    : failure_for<F1, F2>
    {};
};

BOOST_FIT_DECLARE_STATIC_VAR(conditional, detail::make<conditional_adaptor>);

}} // namespace boost::fit

#endif