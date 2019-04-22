#ifndef _MY_FUNCTORS_
#define _MY_FUNCTORS_

#include <stddef.h> 

namespace fyj
{

// Base unary functor
template <class Arg, class Result>
struct unary_functor
{
	typedef Arg    argument_type;
	typedef Result result_type;
};

// Base binary functor
template <class Arg1, class Arg2, class Result>
struct binary_functor
{
	typedef Arg1    first_argument_type;
	typedef Arg2    second_argument_type;
	typedef Result  result_type;
};

//======================== RELATIONAL FUNCTORS =============================

template <class T>
struct greater : public binary_functor<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x > y;
	}
};

template <class T>
struct less : public binary_functor<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x < y;
	}
};

template <class T>
struct greater_equal : public binary_functor<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x >= y;
	}
};

template <class T>
struct less_equal : public binary_functor<T, T, bool>
{
	bool operator()(const T& x, const T& y)
	{
		return x <= y;
	}
};


//====================== IDENTITY ========================================
// Return the same value as passed into function
// Used for Set container: Key and Value for set is identity
template <class T>
struct identity : public unary_functor<T, T>
{
	const T& operator()(const T& x) const {return x;}	
};


//====================== SELECT  ========================================
// Return the 1st / 2nd element of a pair
template <class Pair>
struct select1st : public unary_functor<Pair, typename Pair::first_type>
{
	const typename Pair::first_type& operator()(const Pair& x) const
	{
		return x.first;
	}
};
template <class Pair>
struct select2nd : public unary_functor<Pair, typename Pair::second_type>
{
	const typename Pair::second_type& operator()(const Pair& x) const
	{
		return x.second;
	}
};

//========================= HASH ==================================
template <class Key>
struct hash{};

size_t __hash_string(const char* s)
{
	unsigned long h = 0;
	for(; *s; ++s)
		h = 5*h + *s;
	return size_t(h);
}

template<>
struct hash<char*>
{
	size_t operator()(const char* s) const
	{return __hash_string(s);}
};

template<>
struct hash<const char*>
{
	size_t operator()(const char* s) const
	{return __hash_string(s);}
};

template<>
struct hash<char>
{
	size_t operator()(char c) const {return c;}
};

template<>
struct hash<unsigned char>
{
	size_t operator()(unsigned char c) const {return c;}
};

template<>
struct hash<signed char>
{
	size_t operator()(signed char c) const {return c;}
};

template<>
struct hash<int>
{
	size_t operator()(int c) const {return c;}
};

template<>
struct hash<unsigned int>
{
	size_t operator()(unsigned int c) const {return c;}
};

template<>
struct hash<short>
{
	size_t operator()(short c) const {return c;}
};

template<>
struct hash<unsigned short>
{
	size_t operator()(unsigned short c) const {return c;}
};

template<>
struct hash<long>
{
	size_t operator()(long c) const {return c;}
};

template<>
struct hash<unsigned long>
{
	size_t operator()(unsigned long c) const {return c;}
};

} // end of namespace

#endif