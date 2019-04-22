/* Define three global utility functions:
 * 		- uninitialized_copy
 * 		- uninitialized_fill
 * 		- uninitialized_fill_n
 * by using techniques: iterator, value_type(), __type_traits, is_POD_type
 */

#ifndef _MY_UNINTIALIZED_
#define _MY_UNINTIALIZED_

#include "my_algo.h"
#include "my_construct.h"
#include "my_iterator.h"
#include "my_type_traits.h"

namespace fyj
{
/*=========================uninitialized_copy============================*/

/* Copy from [first, last) to [result, result+(last-first))
 * Use copy constructor
 */
template <class InputIterator, class ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last,
									ForwardIterator result, __true_type)
{
	return copy(first, last, result); //define in "my_algo.h"
}

template <class InputIterator, class ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last,
									ForwardIterator result, __false_type)
{
	ForwardIterator curr = result;
	for(; first != last; ++first, ++curr)
	{
		construct(&*curr, *first);   //define in "my_construct.h" 
	}
}

template <class InputIterator, class ForwardIterator, class T>
ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last,
									ForwardIterator result, T*)
{
	typedef typename __type_traits<T>::is_POD_type is_POD;
	return __uninitialized_copy_aux(first, last, result, is_POD());
}

template <class InputIterator, class ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first, InputIterator last,
									ForwardIterator result)
{
	//test if first is of POD type
	return __uninitialized_copy(first, last, result, value_type(first));
}	


/*===========================uninitialized_fill==================================*/

/* Copy x into each iterator in range of [first, last)
 * Use copy constructor
 */
template <class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
							  const T& x, __true_type)
{
	fill(first, last, x); //define in "my_algo.h"
}

template <class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
							  const T& x, __false_type)
{
	ForwardIterator curr = first;
	for(; first != last; ++first, ++curr)
		construct(&*curr, x); //define in "my_construct.h"
}
template <class ForwardIterator, class T, class T1>
void __unintialized_fill(ForwardIterator first, ForwardIterator last, 
						 const T& x, T1*)
{
	typedef typename __type_traits<T1>::is_POD_type is_POD;
	__uninitialized_fill_aux(first, last, x, is_POD());
}

template <class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last,
						const T& x)
{
	__unintialized_fill(first, last, x, value_type(first));
}


/*============================uninitialized_fill_n================================*/

/* Copy x into each iterator in range of [first, first+n)
 * Use copy constructor
 */
template <class ForwardIterator, class Size, class T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __true_type)
{
	return fill_n(first, n, x); //define in "my_algo.h"
}

template <class ForwardIterator, class Size, class T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __false_type)
{
	ForwardIterator curr = first;
	for(; n > 0; --n, ++curr)
		construct(&*curr, x);   //define in "my_construct.h"   
	return curr;
}

template <class ForwardIterator, class Size, class T, class T1>
ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*)
{
	//__type_traits trick
	typedef typename __type_traits<T1>::is_POD_type is_POD;
	//is_POD = __true_type or is_POD = __false_type
	return __uninitialized_fill_n_aux(first, n, x, is_POD());
}

template <class ForwardIterator, class Size, class T>
ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
	//test if first is of POD type
	return __uninitialized_fill_n(first, n, x, value_type(first));
}


} //end of namespace

#endif