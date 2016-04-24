Print function
==============

Say, for example, we would like to write a print function. We could start by writing the function that prints using `std::cout`, like this:

    FIT_STATIC_LAMBDA_FUNCTION(print) = [](const auto& x)
    {
        std::cout << x << std::endl;
    };

However, there is lot of things that don't print directly to `std::cout` such as `std::vector` or `std::tuple`. Instead, we want to iterate over these data structures and print each element in them.

Overloading
-----------

Now, Fit provides two ways of doing overloading. The [`match`](match.md) adaptor will call a function based on C++ overload resolution, which tries to find the best match, like this:

    FIT_STATIC_LAMBDA_FUNCTION(print) = match(
        [](int x)
        {
            std::cout << "Integer: " << x << std::endl;
        },
        [](const std::string& x)
        {
            std::cout << "String: " << x << std::endl;
        }
    );

However, when trying to do overloading involving something more generic, it can lead to ambiguities. 

The Fit library provides several ways to do overloading. One of the ways is with the [`conditional`](conditional.md) adaptor which will pick the first function that is callable. This allows ordering the functions based on which one is more important. So then the first function will print to `std::cout` if possible otherwise we will add an overload to print a range:


    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range)
        {
            for(const auto& x:range) std::cout << x << std::endl;
        }
    );

The `-> decltype(std::cout << x, void())` is added to the function to constrain it on whether `std::cout << x` is a valid expression. Then the `void()` is used to return `void` from the function. So, now the function can called with a vector:

    std::vector<int> v = { 1, 2, 3, 4 };
    print(v);

This will print each element in the vector. 

We can also constrain the second overload as well, which will be important to add more overloads. So a `for` range loop calls `begin` and `end` to iterated over the range, but we will need some helper function in order to call `std::begin` using ADL lookup:

    namespace adl {

    using std::begin;

    template<class R>
    auto adl_begin(R&& r) FIT_RETURNS(begin(r));
    }

Now we can add `-> decltype(std::cout << *adl::adl_begin(range), void())` to the second function to constrain it to ranges:

    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range) -> decltype(std::cout << *adl::adl_begin(range), void())
        {
            for(const auto& x:range) std::cout << x << std::endl;
        }
    );

So now calling this will work:

    std::vector<int> v = { 1, 2, 3, 4 };
    print(v);

And print out:

    1
    2
    3
    4

Tuples
------

We could extend this to printing tuples as well. We will need to combine a couple of functions to make a `for_each_tuple`, which lets us call a function for each element. First, the [`by`](by.md) adaptor will let us apply a function to each argument passed in, and the [`unpack`](unpack.md) adaptor will unpack the elements of a tuple and apply them to the function:

    FIT_STATIC_LAMBDA_FUNCTION(for_each_tuple) = [](const auto& sequence, auto f)
    {
        return unpack(by(f))(sequence);
    };

So now if we call:

    for_each_tuple(std::make_tuple(1, 2, 3), [](auto i)
    {
        std::cout << i << std::endl;
    });

This will print out:

    1
    2
    3

We can integrate this into our `print` function by adding an additional overload:

    FIT_STATIC_LAMBDA_FUNCTION(print) = conditional(
        [](const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](const auto& range) -> decltype(std::cout << *adl::adl_begin(range), void())
        {
            for(const auto& x:range) std::cout << x << std::endl;
        },
        [](const auto& tuple)
        {
            for_each_tuple(tuple, [](const auto& x)
            {
                std::cout << x << std::endl;
            });
        }
    );

So now we can call `print` with a tuple:

    print(std::make_tuple(1, 2, 3));

And it will print out:

    1
    2
    3

Recursive
---------

Even though this will print for ranges and tuples, if we were to nest a range into a tuple this would not work. What we need to do is make the function call itself recursively. Even though we are using lambdas, we can easily make this recursive using the [`fix`](fix.md) adaptor. This implements a fix point combinator, which passes the function(i.e. itself) in as the first argument. 

So now we add an additional arguments called `self` which is the `print` function itself. This extra argument is called by the [`fix`](fix.md) adaptor, and so the user would still call this function with a single argument:

    FIT_STATIC_LAMBDA_FUNCTION(print) = fix(conditional(
        [](auto, const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](auto self, const auto& range) -> decltype(self(*adl::adl_begin(range)), void())
        {
            for(const auto& x:range) self(x);
        },
        [](auto self, const auto& tuple)
        {
            return for_each_tuple(tuple, self);
        }
    ));

This will let us print nested structures:

    std::vector<int> v = { 1, 2, 3, 4 };
    auto t = std::make_tuple(1, 2, 3, 4);
    auto m = std::make_tuple(3, v, t);
    print(m);

Which outputs this:

    3
    1
    2
    3
    4
    1
    2
    3
    4 

Variadic
--------

We can also make this `print` function variadic, so it prints every argument passed into it. We can use the [`by`](by.md) adaptor, which already calls the function on every argument passed in. First, we just rename our original `print` function to `simple_print`:

    FIT_STATIC_LAMBDA_FUNCTION(simple_print) = fix(conditional(
        [](auto, const auto& x) -> decltype(std::cout << x, void())
        {
            std::cout << x << std::endl;
        },
        [](auto self, const auto& range) -> decltype(self(*adl::adl_begin(range)), void())
        {
            for(const auto& x:range) self(x);
        },
        [](auto self, const auto& tuple)
        {
            return for_each_tuple(tuple, self);
        }
    ));

And then apply the [`by`](by.md) adaptor to `simple_print`:

    FIT_STATIC_LAMBDA_FUNCTION(print) = by(simple_print);

Now we can call `print` with several arguments:

    print(5, "Hello world");

Which outputs:

    5
    Hello world