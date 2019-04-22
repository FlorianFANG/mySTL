#ifndef _MY_CONSTRUCT_
#define _MY_CONSTRUCT_

#include <new>  //for placement new
#include "my_iterator.h"
#include "my_type_traits.h"

namespace fyj
{

/* Construct
 */
template <class T1, class T2>
void construct(T1* p, const T2& value)
{
	//placement new: construct an object in the pre-allocated memory addressed by p
	new(p) T1(value);
}

/* Destroy: 1st version;
 * Call destructor directly
 */
template <class T>
void destroy(T* p)
{
	p->~T();
}

/* Destroy: 2nd version;
 * Passed by two iterators: first, second;
 * To delete objects in range [first, second] by destructor
 *
 * If destructor of each object is:
 *    - trivial destructor     -> do nothing (to save time)
 *    - non-trivial destructor -> call 1st version destroy
 *
 * "trivial destructor" = no much business when destruct the object...
 * which is juged by _type_traits<T>
 */
template <class ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last)
{
	__destroy(first, last, value_type(first)); //value_type defined in my_iterator.h
}

/* __destroy
 * See if value_type(T) has trivial_destructor:
 *     - yes --> trivial_destructor = __true_type
 *     - no  --> trivial_destructor = __false_type
 */
template <class ForwardIterator, class T>
void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
	//Note: because "has_trivial_destructor" is depend of T, so we
	//add "typename" to clarify this is really a type, instead of var

	__destroy_aux(first, last, trivial_destructor());
}

/* __destroy_aux when value_type has trivial_destructor
 */
template <class ForwardIterator>
void __destroy_aux(ForwardIterator first, ForwardIterator last, __true_type)
{
	//do nothing
}

/* __destroy_aux when value_type has non-trivial_destructor
 * call 1st version destroy to each object
 */
template <class ForwardIterator>
void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
	for (; first < last; ++first)
		destroy(&*first);
}

/* Destroy: 3rd version -- special cases
 * Do nothing for the following
 */
void destroy(int*, int*){}
void destroy(float*, float*){}
void destroy(char*, char*){}
void destroy(double*, double*){}


}

#endif