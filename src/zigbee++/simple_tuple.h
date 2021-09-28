#pragma once


namespace std {
// Use chained linear inheritance to store variadic types
// The last template param is the greatest ancestor,
// the first is the 'youngest'.
template<typename... Es> struct tuple;      // definition
template<> struct tuple<> {};               // base case
template<typename E, typename... Es>        // inductive case
struct tuple<E,Es...> : public tuple<Es...>
{
    using base = tuple<Es...>;
    tuple() {} // default constructor
    tuple(E e, Es... es) : base{es...}, first(e)  {}
    E first;
};

// This looks like it's missing a recursive base-case, but due to
// inheritance, this will actually bind to the appropriate ancestor
// to return the right type. 

template<typename E, typename... Es>
E& get(tuple<E,Es...>& t) {
    return t.first;
}

template<typename E, typename... Es>
const E& get(const tuple<E,Es...>& t) {
    return t.first;
}

// NOTE: This is called 'simple' for a reason, it has questionable semantics:
// tuple<float>() x; typle<int,float>() y; x = y; will succeed :D
}
