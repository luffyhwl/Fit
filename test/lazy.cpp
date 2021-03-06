#include <fit/lazy.hpp>
#include <memory>
#include "test.hpp"

template<int N>
struct test_placeholder
{};

namespace std {
    template<int N>
    struct is_placeholder<test_placeholder<N>>
    : std::integral_constant<int, N>
    {};
}

FIT_TEST_CASE()
{
    int i = 5;

    static_assert(std::is_reference<decltype(fit::detail::ref_transformer()(std::ref(i))(0,0,0))>::value, "Reference wrapper failed");
    static_assert(std::is_reference<decltype(fit::detail::pick_transformer(std::ref(i))(0,0,0))>::value, "Reference wrapper failed");
    static_assert(std::is_reference<decltype(fit::detail::lazy_transform(std::ref(i), fit::pack(0,0,0)))>::value, "Reference wrapper failed");

    FIT_TEST_CHECK(&fit::detail::ref_transformer()(std::ref(i))(0,0,0) == &i);
    FIT_TEST_CHECK(&fit::detail::pick_transformer(std::ref(i))(0,0,0) == &i);
    FIT_TEST_CHECK(&fit::detail::lazy_transform(std::ref(i), fit::pack(0,0,0)) == &i);
}

FIT_TEST_CASE()
{
    int i = 5;

    FIT_TEST_CHECK(fit::detail::id_transformer()(i)(0,0,0) == i);
    FIT_TEST_CHECK(fit::detail::pick_transformer(i)(0,0,0) == i);
    FIT_TEST_CHECK(fit::detail::lazy_transform(i, fit::pack(0,0,0)) == i);
}

FIT_TEST_CASE()
{
    auto id =[](int i){ return i;};
    auto fi = std::bind(id, 5);

    FIT_TEST_CHECK(fit::detail::bind_transformer()(fi)(0,0,0) == id(5));
    FIT_TEST_CHECK(fit::detail::pick_transformer(fi)(0,0,0) == id(5));
    FIT_TEST_CHECK(fit::detail::lazy_transform(fi, fit::pack(0,0,0)) == id(5));
}

struct f_0 {
constexpr long operator()() const
{
    return 17041L;
}
};

struct f_1 {
constexpr long operator()(long a) const
{
    return a;
}
};

struct f_2 {
constexpr long operator()(long a, long b) const
{
    return a + 10 * b;
}
};

static long global_result;

struct fv_0 {
void operator()() const
{
    global_result = 17041L;
}
};

struct fv_1 {
void operator()(long a) const
{
    global_result = a;
}
};

struct fv_2 {
void operator()(long a, long b) const
{
    global_result = a + 10 * b;
}
};

struct Y
{
    short operator()(short & r) const { return ++r; }
    int operator()(int a, int b) const { return a + 10 * b; }
    long operator() (long a, long b, long c) const { return a + 10 * b + 100 * c; }
    void operator() (long a, long b, long c, long d) const { global_result = a + 10 * b + 100 * c + 1000 * d; }
};

FIT_TEST_CASE()
{
    short i(6);

    int const k = 3;

    FIT_TEST_CHECK( fit::lazy(Y())( std::ref(i))() == 7 );
    FIT_TEST_CHECK( fit::lazy(Y())( std::ref(i))() == 8 );
    FIT_TEST_CHECK( fit::lazy(Y())( i,std::placeholders::_1)(k) == 38 );
    FIT_TEST_CHECK( fit::lazy(Y())( i,std::placeholders::_1, 9)(k) == 938 );

    global_result = 0;
    fit::lazy(Y())( i,std::placeholders::_1, 9, 4)(k);
    FIT_TEST_CHECK( global_result == 4938 );

}

FIT_TEST_CASE()
{
    int const x = 1;
    int const y = 2;

    FIT_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_1())(std::placeholders::_1))(x) == 1L );
    FIT_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_2())(std::placeholders::_1, std::placeholders::_2))(x, y) == 21L );
    FIT_TEST_CHECK( fit::lazy(f_2())( fit::lazy(f_1())(std::placeholders::_1), fit::lazy(f_1())(std::placeholders::_1))(x) == 11L );
    FIT_TEST_CHECK( fit::lazy(f_2())( fit::lazy(f_1())(std::placeholders::_1), fit::lazy(f_1())( std::placeholders::_2))(x, y) == 21L );
    FIT_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_0())())() == 17041L );

    FIT_STATIC_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_1())(test_placeholder<1>()))(x) == 1L );
    FIT_STATIC_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_2())(test_placeholder<1>(), test_placeholder<2>()))(x, y) == 21L );
    FIT_STATIC_TEST_CHECK( fit::lazy(f_2())( fit::lazy(f_1())(test_placeholder<1>()), fit::lazy(f_1())(test_placeholder<1>()))(x) == 11L );
    FIT_STATIC_TEST_CHECK( fit::lazy(f_2())( fit::lazy(f_1())(test_placeholder<1>()), fit::lazy(f_1())( test_placeholder<2>()))(x, y) == 21L );
    FIT_STATIC_TEST_CHECK( fit::lazy(f_1())( fit::lazy(f_0())())() == 17041L );

    FIT_TEST_CHECK( (fit::lazy(fv_1())( fit::lazy(f_1())(std::placeholders::_1))(x), (global_result == 1L)) );
    FIT_TEST_CHECK( (fit::lazy(fv_1())( fit::lazy(f_2())(std::placeholders::_1, std::placeholders::_2))(x, y), (global_result == 21L)) );
    FIT_TEST_CHECK( (fit::lazy(fv_2())( fit::lazy(f_1())(std::placeholders::_1), fit::lazy(f_1())(std::placeholders::_1))(x), (global_result == 11L)) );
    FIT_TEST_CHECK( (fit::lazy(fv_2())( fit::lazy(f_1())(std::placeholders::_1), fit::lazy(f_1())( std::placeholders::_2))(x, y), (global_result == 21L)) );
    FIT_TEST_CHECK( (fit::lazy(fv_1())( fit::lazy(f_0())())(), (global_result == 17041L)) );
}


struct X
{
    mutable unsigned int hash;

    X(): hash(0) {}

    int f0() { f1(17); return 0; }
    int g0() const { g1(17); return 0; }

    int f1(int a1) { hash = (hash * 17041 + a1) % 32768; return 0; }
    int g1(int a1) const { hash = (hash * 17041 + a1 * 2) % 32768; return 0; }

    int f2(int a1, int a2) { f1(a1); f1(a2); return 0; }
    int g2(int a1, int a2) const { g1(a1); g1(a2); return 0; }

    int f3(int a1, int a2, int a3) { f2(a1, a2); f1(a3); return 0; }
    int g3(int a1, int a2, int a3) const { g2(a1, a2); g1(a3); return 0; }

    int f4(int a1, int a2, int a3, int a4) { f3(a1, a2, a3); f1(a4); return 0; }
    int g4(int a1, int a2, int a3, int a4) const { g3(a1, a2, a3); g1(a4); return 0; }

    int f5(int a1, int a2, int a3, int a4, int a5) { f4(a1, a2, a3, a4); f1(a5); return 0; }
    int g5(int a1, int a2, int a3, int a4, int a5) const { g4(a1, a2, a3, a4); g1(a5); return 0; }

    int f6(int a1, int a2, int a3, int a4, int a5, int a6) { f5(a1, a2, a3, a4, a5); f1(a6); return 0; }
    int g6(int a1, int a2, int a3, int a4, int a5, int a6) const { g5(a1, a2, a3, a4, a5); g1(a6); return 0; }

    int f7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) { f6(a1, a2, a3, a4, a5, a6); f1(a7); return 0; }
    int g7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) const { g6(a1, a2, a3, a4, a5, a6); g1(a7); return 0; }

    int f8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { f7(a1, a2, a3, a4, a5, a6, a7); f1(a8); return 0; }
    int g8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) const { g7(a1, a2, a3, a4, a5, a6, a7); g1(a8); return 0; }
};

struct V
{
    mutable unsigned int hash;

    V(): hash(0) {}

    void f0() { f1(17); }
    void g0() const { g1(17); }

    void f1(int a1) { hash = (hash * 17041 + a1) % 32768; }
    void g1(int a1) const { hash = (hash * 17041 + a1 * 2) % 32768; }

    void f2(int a1, int a2) { f1(a1); f1(a2); }
    void g2(int a1, int a2) const { g1(a1); g1(a2); }

    void f3(int a1, int a2, int a3) { f2(a1, a2); f1(a3); }
    void g3(int a1, int a2, int a3) const { g2(a1, a2); g1(a3); }

    void f4(int a1, int a2, int a3, int a4) { f3(a1, a2, a3); f1(a4); }
    void g4(int a1, int a2, int a3, int a4) const { g3(a1, a2, a3); g1(a4); }

    void f5(int a1, int a2, int a3, int a4, int a5) { f4(a1, a2, a3, a4); f1(a5); }
    void g5(int a1, int a2, int a3, int a4, int a5) const { g4(a1, a2, a3, a4); g1(a5); }

    void f6(int a1, int a2, int a3, int a4, int a5, int a6) { f5(a1, a2, a3, a4, a5); f1(a6); }
    void g6(int a1, int a2, int a3, int a4, int a5, int a6) const { g5(a1, a2, a3, a4, a5); g1(a6); }

    void f7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) { f6(a1, a2, a3, a4, a5, a6); f1(a7); }
    void g7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) const { g6(a1, a2, a3, a4, a5, a6); g1(a7); }

    void f8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { f7(a1, a2, a3, a4, a5, a6, a7); f1(a8); }
    void g8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) const { g7(a1, a2, a3, a4, a5, a6, a7); g1(a8); }
};

FIT_TEST_CASE()
{

    X x;

    // 0

    fit::lazy(&X::f0)(&x)();
    fit::lazy(&X::f0)(std::ref(x))();

    fit::lazy(&X::g0)(&x)();
    fit::lazy(&X::g0)(x)();
    fit::lazy(&X::g0)(std::ref(x))();

    // 1

    fit::lazy(&X::f1)(&x, 1)();
    fit::lazy(&X::f1)(std::ref(x), 1)();

    fit::lazy(&X::g1)(&x, 1)();
    fit::lazy(&X::g1)(x, 1)();
    fit::lazy(&X::g1)(std::ref(x), 1)();

    // 2

    fit::lazy(&X::f2)(&x, 1, 2)();
    fit::lazy(&X::f2)(std::ref(x), 1, 2)();

    fit::lazy(&X::g2)(&x, 1, 2)();
    fit::lazy(&X::g2)(x, 1, 2)();
    fit::lazy(&X::g2)(std::ref(x), 1, 2)();

    // 3

    fit::lazy(&X::f3)(&x, 1, 2, 3)();
    fit::lazy(&X::f3)(std::ref(x), 1, 2, 3)();

    fit::lazy(&X::g3)(&x, 1, 2, 3)();
    fit::lazy(&X::g3)(x, 1, 2, 3)();
    fit::lazy(&X::g3)(std::ref(x), 1, 2, 3)();

    // 4

    fit::lazy(&X::f4)(&x, 1, 2, 3, 4)();
    fit::lazy(&X::f4)(std::ref(x), 1, 2, 3, 4)();

    fit::lazy(&X::g4)(&x, 1, 2, 3, 4)();
    fit::lazy(&X::g4)(x, 1, 2, 3, 4)();
    fit::lazy(&X::g4)(std::ref(x), 1, 2, 3, 4)();

    // 5

    fit::lazy(&X::f5)(&x, 1, 2, 3, 4, 5)();
    fit::lazy(&X::f5)(std::ref(x), 1, 2, 3, 4, 5)();

    fit::lazy(&X::g5)(&x, 1, 2, 3, 4, 5)();
    fit::lazy(&X::g5)(x, 1, 2, 3, 4, 5)();
    fit::lazy(&X::g5)(std::ref(x), 1, 2, 3, 4, 5)();

    // 6

    fit::lazy(&X::f6)(&x, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&X::f6)(std::ref(x), 1, 2, 3, 4, 5, 6)();

    fit::lazy(&X::g6)(&x, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&X::g6)(x, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&X::g6)(std::ref(x), 1, 2, 3, 4, 5, 6)();

    // 7

    fit::lazy(&X::f7)(&x, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&X::f7)(std::ref(x), 1, 2, 3, 4, 5, 6, 7)();

    fit::lazy(&X::g7)(&x, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&X::g7)(x, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&X::g7)(std::ref(x), 1, 2, 3, 4, 5, 6, 7)();

    // 8

    fit::lazy(&X::f8)(&x, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&X::f8)(std::ref(x), 1, 2, 3, 4, 5, 6, 7, 8)();

    fit::lazy(&X::g8)(&x, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&X::g8)(x, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&X::g8)(std::ref(x), 1, 2, 3, 4, 5, 6, 7, 8)();

    FIT_TEST_CHECK( x.hash == 23558 );
}

FIT_TEST_CASE()
{
    V v;

    // 0

    fit::lazy(&V::f0)(&v)();
    fit::lazy(&V::f0)(std::ref(v))();

    fit::lazy(&V::g0)(&v)();
    fit::lazy(&V::g0)(v)();
    fit::lazy(&V::g0)(std::ref(v))();

    // 1

    fit::lazy(&V::f1)(&v, 1)();
    fit::lazy(&V::f1)(std::ref(v), 1)();

    fit::lazy(&V::g1)(&v, 1)();
    fit::lazy(&V::g1)(v, 1)();
    fit::lazy(&V::g1)(std::ref(v), 1)();

    // 2

    fit::lazy(&V::f2)(&v, 1, 2)();
    fit::lazy(&V::f2)(std::ref(v), 1, 2)();

    fit::lazy(&V::g2)(&v, 1, 2)();
    fit::lazy(&V::g2)(v, 1, 2)();
    fit::lazy(&V::g2)(std::ref(v), 1, 2)();

    // 3

    fit::lazy(&V::f3)(&v, 1, 2, 3)();
    fit::lazy(&V::f3)(std::ref(v), 1, 2, 3)();

    fit::lazy(&V::g3)(&v, 1, 2, 3)();
    fit::lazy(&V::g3)(v, 1, 2, 3)();
    fit::lazy(&V::g3)(std::ref(v), 1, 2, 3)();

    // 4

    fit::lazy(&V::f4)(&v, 1, 2, 3, 4)();
    fit::lazy(&V::f4)(std::ref(v), 1, 2, 3, 4)();

    fit::lazy(&V::g4)(&v, 1, 2, 3, 4)();
    fit::lazy(&V::g4)(v, 1, 2, 3, 4)();
    fit::lazy(&V::g4)(std::ref(v), 1, 2, 3, 4)();

    // 5

    fit::lazy(&V::f5)(&v, 1, 2, 3, 4, 5)();
    fit::lazy(&V::f5)(std::ref(v), 1, 2, 3, 4, 5)();

    fit::lazy(&V::g5)(&v, 1, 2, 3, 4, 5)();
    fit::lazy(&V::g5)(v, 1, 2, 3, 4, 5)();
    fit::lazy(&V::g5)(std::ref(v), 1, 2, 3, 4, 5)();

    // 6

    fit::lazy(&V::f6)(&v, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&V::f6)(std::ref(v), 1, 2, 3, 4, 5, 6)();

    fit::lazy(&V::g6)(&v, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&V::g6)(v, 1, 2, 3, 4, 5, 6)();
    fit::lazy(&V::g6)(std::ref(v), 1, 2, 3, 4, 5, 6)();

    // 7

    fit::lazy(&V::f7)(&v, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&V::f7)(std::ref(v), 1, 2, 3, 4, 5, 6, 7)();

    fit::lazy(&V::g7)(&v, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&V::g7)(v, 1, 2, 3, 4, 5, 6, 7)();
    fit::lazy(&V::g7)(std::ref(v), 1, 2, 3, 4, 5, 6, 7)();

    // 8

    fit::lazy(&V::f8)(&v, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&V::f8)(std::ref(v), 1, 2, 3, 4, 5, 6, 7, 8)();

    fit::lazy(&V::g8)(&v, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&V::g8)(v, 1, 2, 3, 4, 5, 6, 7, 8)();
    fit::lazy(&V::g8)(std::ref(v), 1, 2, 3, 4, 5, 6, 7, 8)();

    FIT_TEST_CHECK( v.hash == 23558 );
}

struct id
{
    int operator()(int i) const
    {
        return i;
    }
};

FIT_TEST_CASE()
{
    FIT_TEST_CHECK(fit::lazy(id())(3)() == 3);
}

struct deref
{
    int operator()(const std::unique_ptr<int>& i) const
    {
        return *i;
    }
};

FIT_TEST_CASE()
{
    FIT_TEST_CHECK(fit::lazy(deref())(std::unique_ptr<int>(new int(3)))() == 3);
}

void fv1( std::unique_ptr<int> p1 )
{
    FIT_TEST_CHECK( *p1 == 1 );
}

void fv2( std::unique_ptr<int> p1, std::unique_ptr<int> p2 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
}

void fv3( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
}

void fv4( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
}

void fv5( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4, std::unique_ptr<int> p5 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
    FIT_TEST_CHECK( *p5 == 5 );
}

void fv6( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4, std::unique_ptr<int> p5, std::unique_ptr<int> p6 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
    FIT_TEST_CHECK( *p5 == 5 );
    FIT_TEST_CHECK( *p6 == 6 );
}

void fv7( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4, std::unique_ptr<int> p5, std::unique_ptr<int> p6, std::unique_ptr<int> p7 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
    FIT_TEST_CHECK( *p5 == 5 );
    FIT_TEST_CHECK( *p6 == 6 );
    FIT_TEST_CHECK( *p7 == 7 );
}

void fv8( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4, std::unique_ptr<int> p5, std::unique_ptr<int> p6, std::unique_ptr<int> p7, std::unique_ptr<int> p8 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
    FIT_TEST_CHECK( *p5 == 5 );
    FIT_TEST_CHECK( *p6 == 6 );
    FIT_TEST_CHECK( *p7 == 7 );
    FIT_TEST_CHECK( *p8 == 8 );
}

void fv9( std::unique_ptr<int> p1, std::unique_ptr<int> p2, std::unique_ptr<int> p3, std::unique_ptr<int> p4, std::unique_ptr<int> p5, std::unique_ptr<int> p6, std::unique_ptr<int> p7, std::unique_ptr<int> p8, std::unique_ptr<int> p9 )
{
    FIT_TEST_CHECK( *p1 == 1 );
    FIT_TEST_CHECK( *p2 == 2 );
    FIT_TEST_CHECK( *p3 == 3 );
    FIT_TEST_CHECK( *p4 == 4 );
    FIT_TEST_CHECK( *p5 == 5 );
    FIT_TEST_CHECK( *p6 == 6 );
    FIT_TEST_CHECK( *p7 == 7 );
    FIT_TEST_CHECK( *p8 == 8 );
    FIT_TEST_CHECK( *p9 == 9 );
}

FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );

    fit::lazy( fv1 )( std::placeholders::_1 )( std::move( p1 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );

    fit::lazy( fv2 )( std::placeholders::_1, std::placeholders::_2 )( std::move( p1 ), std::move( p2 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );

    fit::lazy( fv3 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 )( std::move( p1 ), std::move( p2 ), std::move( p3 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );

    fit::lazy( fv4 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );
    std::unique_ptr<int> p5( new int(5) );

    fit::lazy( fv5 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ), std::move( p5 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );
    std::unique_ptr<int> p5( new int(5) );
    std::unique_ptr<int> p6( new int(6) );

    fit::lazy( fv6 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ), std::move( p5 ), std::move( p6 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );
    std::unique_ptr<int> p5( new int(5) );
    std::unique_ptr<int> p6( new int(6) );
    std::unique_ptr<int> p7( new int(7) );

    fit::lazy( fv7 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ), std::move( p5 ), std::move( p6 ), std::move( p7 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );
    std::unique_ptr<int> p5( new int(5) );
    std::unique_ptr<int> p6( new int(6) );
    std::unique_ptr<int> p7( new int(7) );
    std::unique_ptr<int> p8( new int(8) );

    fit::lazy( fv8 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ), std::move( p5 ), std::move( p6 ), std::move( p7 ), std::move( p8 ) );
}
FIT_TEST_CASE()
{
    std::unique_ptr<int> p1( new int(1) );
    std::unique_ptr<int> p2( new int(2) );
    std::unique_ptr<int> p3( new int(3) );
    std::unique_ptr<int> p4( new int(4) );
    std::unique_ptr<int> p5( new int(5) );
    std::unique_ptr<int> p6( new int(6) );
    std::unique_ptr<int> p7( new int(7) );
    std::unique_ptr<int> p8( new int(8) );
    std::unique_ptr<int> p9( new int(9) );

    fit::lazy( fv9 )( std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9 )( std::move( p1 ), std::move( p2 ), std::move( p3 ), std::move( p4 ), std::move( p5 ), std::move( p6 ), std::move( p7 ), std::move( p8 ), std::move( p9 ) );
}


struct X_ref
{
    int f( int x )
    {
        return x;
    }

    int g( int x ) const
    {
        return -x;
    }
};

FIT_TEST_CASE()
{
    X_ref x;

    FIT_TEST_CHECK( fit::lazy( &X_ref::f )( std::ref( x ), std::placeholders::_1 )( 1 ) == 1 );
    FIT_TEST_CHECK( fit::lazy( &X_ref::g )( std::cref( x ), std::placeholders::_1 )( 2 ) == -2 );
}

FIT_TEST_CASE()
{
    auto lazy_f_1 = fit::lazy(f_1())(std::placeholders::_1);
    static_assert(fit::is_callable<decltype(lazy_f_1), long>::value, "Callable");
    static_assert(fit::is_callable<decltype(lazy_f_1), long, long>::value, "Callable");
    
    auto lazy_f_2 = fit::lazy(f_2())(std::placeholders::_1, std::placeholders::_2);
    static_assert(fit::is_callable<decltype(lazy_f_2), long, long>::value, "Callable");
    static_assert(!fit::is_callable<decltype(lazy_f_2), long>::value, "Not SFINAE-friendly");
}

struct dummy_unary_fn
{
    template <typename S>
    int operator()(S const &) const { return 0; }
};

struct bad_unary_fn
{
    template <typename S>
    constexpr int operator()(S const &) const
    {
        static_assert(!std::is_same<S, S>::value, "Failure");
        return 0;
    }
};

FIT_TEST_CASE()
{
    auto b = fit::lazy(dummy_unary_fn())(bad_unary_fn());
    b(0);
}
