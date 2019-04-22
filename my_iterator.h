#ifndef _MY_ITERATOR_
#define _MY_ITERATOR_

#include <stddef.h>
#include <iostream>

namespace fyj
{

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag : public input_iterator_tag{};
struct bidirectional_iterator_tag : public forward_iterator_tag{};
struct random_access_iterator_tag : public bidirectional_iterator_tag{};


template<class Category, class T, class Distance = ptrdiff_t,
		 class Pointer = T*, class Reference = T&>
struct iterator
{
	typedef Category 	iterator_category;
	typedef T 			value_type;
	typedef Distance 	difference_type;
	typedef Pointer 	pointer;
	typedef Reference 	reference;
};


// Given an iterator type Itr
// If Itr is exactly a class type like defined above...
// ...then iterator_traits has no much sense because...
// ...we can directly use Itr::value_type to get its value_type
// Otherwise, Itr is NOT a class type, but a naive pointer...
// ...then we cannot use scope resolution "::" for it
// ==> To unify class-like iterator and naive pointer iterator...
//     ...iterator_traits is needed  
template<class Iterator>
struct iterator_traits
{
	typedef typename Iterator::iterator_category iterator_category;
	typedef typename Iterator::value_type 		 value_type;
	typedef typename Iterator::difference_type 	 difference_type;
	typedef typename Iterator::pointer 			 pointer;
	typedef typename Iterator::reference     	 reference;
};


//partial specification for native pointer
template <class T>
struct iterator_traits<T*>
{
	typedef random_access_iterator_tag 	 iterator_category;
	typedef T 							 value_type;
	typedef ptrdiff_t  					 difference_type;
	typedef T* 							 pointer;
	typedef T& 							 reference;
};


//partial specification for native pointer-to-const
template<class T>
struct iterator_traits<const T*>
{
	typedef random_access_iterator_tag 	 iterator_category;
	typedef T   						 value_type;
	typedef ptrdiff_t  			   		 difference_type;
	typedef const T*					 pointer;
	typedef const T&					 reference;
};


//determine category
template <class Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{
	typedef typename iterator_traits<Iterator>::iterator_category category;
	return category();
}


//determine difference type
template <class Iterator>
inline typename iterator_traits<Iterator>::difference_type*
difference_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}


//determine value type
template <class Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}
		   

} //end of namespace


#endif