#ifndef _MY_ALGO_
#define _MY_ALGO_

#include "my_iterator.h"
#include "my_type_traits.h"
#include <stddef.h>
#include <cstdlib>
#include <cstring> // for memmove
#include "my_pair.h"

namespace fyj
{

//=============================== ACCUMULATE ============================================================
template <class InputIterator, class T>
T accumulate(InputIterator first, InputIterator last, T init)
{
    for(; first != last; ++first)
        init += *first;
    return init;
}
template <class InputIterator, class T, class BinaryOperation>
T accumulate(InputIterator first, InputIterator last, T init, BinaryOperation bin_op) //bin_op is functor
{
    for(; first != last; ++first)
        init = bin_op(init, *first);
    return init;
}


//======================================= ADVANCE =====================================================
template <class InputIterator, class Distance>
inline void __advance(InputIterator i, Distance n, input_iterator_tag)
{
    while(n--)
        ++i;
}
template <class BidirectionalIterator, class Distance>
inline void __advance(BidirectionalIterator i, Distance n, bidirectional_iterator_tag)
{
    if (n >= 0)
        while(n--) ++i;
    else
        while(n++) --i;
}
template <class RandomAccessIterator, class Distance>
inline void __advance(RandomAccessIterator i, Distance n, random_access_iterator_tag)
{
    i += n;
}
template <class InputIterator, class Distance>
inline void advance(InputIterator i, Distance n)
{
    __advance(i,n,iterator_traits<InputIterator>::iterator_category());
}


//======================================= ADJACENT_DIFFERENCE ===========================================
//1th version: difference calculated by normal operator-
template <class InputIterator, class OutputIterator>
OutputIterator adjacent_difference(InputIterator first, InputIterator last, OutputIterator result)
{
    if(first == result)
        return result;
    *result = *first;
    typename iterator_traits<InputIterator>::value_type value = *first;
    while(++first != last) //go to the next iterator until last
    {
        typename iterator_traits<InputIterator>::value_type temp = *first;
        *++result = temp - value; //*next_difference = *next_iterator - *current_iterator
        value = temp;
    }
    return ++result;
}
//2nd version: difference calculated by overload functor for operator-
template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator adjacent_difference(InputIterator first, InputIterator last, OutputIterator result,
                                   BinaryOperation bin_op)
{
    if(first == result)
        return result;
    *result = *first;
    typename iterator_traits<InputIterator>::value_type value = *first;
    while(++first != last) //go to the next iterator until last
    {
        typename iterator_traits<InputIterator>::value_type temp = *first;
        *++result = bin_op(temp, value); //*next_difference = *next_iterator - *current_iterator
        value = temp;
    }
    return ++result;
}

/* ================================= COPY =====================================*/
/* Copy the element from [first, last) to [result, result + (last-first))
 * Return = result + (last - first)
 */

// Two __copy implementations depending on the iterator types, i.e.
// input_iterator, random_access_iterator
template <class InputIterator, class OutputIterator>
inline OutputIterator __copy(InputIterator first, InputIterator last,
							OutputIterator result, input_iterator_tag)
{
	for(; first != last; ++first, ++result)
		*result = *first;
	return result;
}
template <class RandomAccessIterator, class OutputIterator>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
							 OutputIterator result, random_access_iterator_tag)
{
	return __copy_d(first, last, result, distance_type(first));
}							 
template <class RandomAccessIterator, class OutputIterator, class Distance>
inline OutputIterator __copy_d(RandomAccessIterator first, RandomAccessIterator last,
	     					   OutputIterator result, Distance*)
{
	for(Distance n = last - first; n > 0; --n, ++first, ++result)
		*result = *first;
	return result;
}	     					   


// generalized form
template <class InputIterator, class OutputIterator>
struct __copy_dispatch
{
	OutputIterator operator()(InputIterator first, InputIterator last,
							  OutputIterator result)
	{
		return __copy(first, last, result, iterator_category(first));
	}
};

//Specification 1: both parameters are pointers
template <class T>
struct __copy_dispatch<T*, T*>
{
	T* operator()(T* first, T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

//Specification 2: first param is const T* and second is T*
template <class T>
struct __copy_dispatch<const T*, T*>
{
	T* operator()(const T* first, const T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

//__copy_t depending on whether has trivial assignment operator
//         -> if yes, copy directly by memmove
//   	   -> otherwise copy by assignment operator
template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __true_type)
{
	memmove(result, first, sizeof(T)*(last-first));
	return result + (last - first);
}
template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __false_type)
{
	return __copy_d(first, last, result, (ptrdiff_t*)0);
}

//Standard interface
template <class InputIterator, class OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last,
						OutputIterator result)
{
	return __copy_dispatch<InputIterator, OutputIterator>()
					(first, last, result);
}	

//overload copy for char*
inline char* copy(const char* first, const char* last, char* result)
{
	std::memmove(result, first, last - first);
	return result + (last - first);
}


/* ================================= COPY_BACKWARD ================================*/
/* Copy the element from [first, last) to [result-(last-first), result-1)
 * Return = result - (last - first)
 */

// __copy_backward accept only BidirectionalIterator
template <class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 
__copy_backward(BidirectionalIterator1 first, 
		   		BidirectionalIterator1 last,
				BidirectionalIterator2 result)
{
	while(first != last)
		*--result = *--last;
	return result;
}

// generalized form
template <class BidirectionalIterator1, class BidirectionalIterator2>
struct __copy_backward_dispatch
{
	BidirectionalIterator2 operator()(BidirectionalIterator1 first, 
									  BidirectionalIterator1 last,
							  		  BidirectionalIterator2 result)
	{
		return __copy_backward(first, last, result);
	}
};

//Specification 1: both parameters are pointers
template <class T>
struct __copy_backward_dispatch<T*, T*>
{
	T* operator()(T* first, T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_backward_t(first, last, result, t());
	}
};

//Specification 2: first param is const T* and second is T*
template <class T>
struct __copy_backward_dispatch<const T*, T*>
{
	T* operator()(const T* first, const T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_backward_t(first, last, result, t());
	}
};

//__copy_t depending on whether has trivial assignment operator
//         -> if yes, copy directly by memmove
//   	   -> otherwise copy by assignment operator
template <class T>
inline T* __copy_backward_t(const T* first, const T* last, 
						    T* result, __true_type)
{
	memmove(result - (first - last), first, sizeof(T)*(last-first));
	return result - (last - first);
}
template <class T>
inline T* __copy_backward_t(const T* first, const T* last, 
						    T* result, __false_type)
{
	for(; first <= last; ++first, --last, --result)
		*result = *last;
	return result;
}

//Standard interface
template <class BidirectionalIterator1, class BidirectionalIterator2>
BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, 
			    					 BidirectionalIterator1 last,
									 BidirectionalIterator2 result)
{
	return __copy_backward_dispatch<BidirectionalIterator1, 
		      			   			BidirectionalIterator2>()
		      			   			(first, last, result);
}	

//overload copy for char*
inline char* copy_backward(const char* first, const char* last, 
						   char* result)
{
	std::memmove(result - (last - first), first, last - first);
	return result - (last - first);
}



//====================================== DISTANCE =======================================================
template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last,
           input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last)
    {
        ++first;
        ++n;
    }
    return n;
}
template <class RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last,
           random_access_iterator_tag)
{
    return last - first;
}
template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
    typedef typename iterator_traits<InputIterator>::iterator_category category;
    __distance(first, last, category());
}

//======================================= EQUAL =========================================================
template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
    for( ; first1 != last1; ++first1, ++first2)
    {
        if (*first1 != *first2)
            return false;
    }
    return true;
}

template <class InputIterator1, class InputIterator2, class BinaryPredicates>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicates bin_pred)
{
    for( ; first1 != last1; ++first1, ++first2)
        if(!bin_pred(*first1,*first2))
            return false;
    return true;
}

/*========================= FILL =====================================*/
template <class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& x)
{
	for(; first != last; ++first)
		*first = x;
}	

template <class ForwardIterator, class Size, class T>
ForwardIterator fill_n(ForwardIterator first, Size n, const T& x)
{
	for(; n > 0; --n, ++first)
		*first = x;
	return first;
}

//================================== ITER_SWAP ===========================================================
template <class ForwardIterator1, class ForwardIterator2>
inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
{
    __iter_swap(a, b, value_type(a));
}

template <class ForwardIterator1, class ForwardIterator2, class T>
inline void __iter_swap(ForwardIterator1 a, ForwardIterator2 b, T*)
{
    T temp = *a;
    *a = *b;
    *b = temp;
}

template <class T>
inline void swap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

//===================================== MAX ================================================================
template <class T>
inline const T& max(T a, T b)
{
    return a < b ? b : a;
}

template <class T, class Compare>
inline const T& max(T a, T b, Compare comp) //comp is overloaded operator< for class T
{
    return comp(a, b) ? b : a;
}

template <class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last)
{
    if(first == last)
        return first;
    ForwardIterator result = first;
    while(++first != last)
    {
        if(*first > *result)
            result = first;
    }
    return result;
}

template <class ForwardIterator, class Compare>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last,
                            Compare comp) //comp is overload operator>
{
    if(first == last)
        return first;
    ForwardIterator result = first;
    while(++first != last)
    {
        if(comp(*first, *result))
            result = first;
    }
    return result;
}

//===================================== MIN ================================================================
template <class T>
inline const T& min(T a, T b)
{
    return a < b ? a : b;
}

template <class T, class Compare>
inline const T& min(T a, T b, Compare comp) //comp is overloaded operator> for class T
{
    return comp(a, b) ? a : b;
}

template <class ForwardIterator>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last)
{
    if(first == last)
        return first;
    ForwardIterator result = first;
    while(++first != last)
    {
        if(*first < *result)
            result = first;
    }
    return result;
}

template <class ForwardIterator, class Compare>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last,
                            Compare comp) //comp is overload operator<
{
    if(first == last)
        return first;
    ForwardIterator result = first;
    while(++first != last)
    {
        if(comp(*first, *result))
            result = first;
    }
    return result;
}


//======================================= MISMATCH =========================================================
template <class InputIterator1, class InputIterator2>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
                                              InputIterator1 last1,
                                              InputIterator2 first2)
{
    while(first1 != last1 && *first1 == *first2)
    {
        ++first1;
        ++first2;
    }
    return pair<InputIterator1, InputIterator2>(first1, first2);
}

template <class InputIterator1, class InputIterator2, class BinaryPredicates>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
                                              InputIterator1 last1,
                                              InputIterator2 first2,
                                              BinaryPredicates bin_pred)
{
    while(first1 != last1 && bin_pred(*first1, *first2))
    {
        ++first1;
        ++first2;
    }
    return pair<InputIterator1, InputIterator2>(first1, first2);
}

//==========================================  COUNT =================================================
#include "my_iterator.h"
template <class InputIterator, class T>
typename iterator_traits<InputIterator>::difference_type
count(InputIterator first, InputIterator last, const T& value)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    for( ; first != last; ++first)
        if(*first == value)
            ++n;
    return n;
}

template <class InputIterator, class T, class Predicates>
typename iterator_traits<InputIterator>::difference_type
count_if(InputIterator first, InputIterator last, const T& value, Predicates pred)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    for( ; first != last; ++first)
        if(pred(*first,value))
            ++n;
    return n;
}


//======================================== FIND ===============================================
template <class InputIterator, class T>
InputIterator find(InputIterator first, InputIterator last, const T& value)
{
    while(first != last && *first != value)
        ++first;
    return first;
}

template <class InputIterator, class Predicate>
InputIterator find_if(InputIterator first, InputIterator last, Predicate pred)
{
    while(first != last && pred(*first))
        ++first;
    return first;
}

//======================================= GENERATE ============================================
template <class ForwardIterator, class Generator>
void generate(ForwardIterator first, ForwardIterator last, Generator gen)
{
    for(; first != last; ++first)
        *first = gen();
}

template <class OutputIterator, class Size, class Generator>
OutputIterator generate(OutputIterator first, Size n, Generator gen)
{
    for(; n > 0; --n, ++first)
        *first = gen();
    return first;
}


//================================ HEAP ALGO ===============================
// Heap is the base data structure for priority_queue
// Heap is complete binary tree, using vector for implementation
// 		- vec[0] is a whatever value
//  	- vec[1] is the root of the binary tree, for the advantage that:
//         		# vec[i]'s left child = vec[2*i]
// 				# vec[i]'s right child = vec[2*i+1]
// 				# vec[i]'s parent = vec[i/2]
// This is max-heap: the max element is at the root of binary tree
// Some heap algorithms are provided below

// 1. push_heap algorithm
// Assuming that the element already pushed at the end of vector
// Then need to adjust the position of this element
template <class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
				 Distance topIndex, T value)
{
	Distance parent = (holeIndex - 1) / 2;
	while (holeIndex > topIndex && *(first + parent) < value)
	{
		*(first + holeIndex) = *(first + parent);
		holeIndex = parent;
		parent = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}				 
template <class RandomAccessIterator, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first,
						    RandomAccessIterator last,
						    Distance*, T*)
{
	__push_heap(first, Distance((last - first) - 1),
		        Distance(0), T(*(last - 1)));
}						    
template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first,
	  			   	  RandomAccessIterator last)
{
	__push_heap_aux(first, last, distance_type(first), value_type(first));
}

// 2. pop_heap algorithm
// Pop the root (max) of the heap --> actually move the end of vector
// and temporily let the last element at root
// Then adjust the new max to root
template <class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex,
			       Distance len, T value)
{
	Distance topIndex = holeIndex;
	Distance secondChild = 2 * holeIndex + 2;
	while (secondChild < len)
	{
		if(*(first + secondChild) < *(first + (secondChild - 1)))
			secondChild--;
		*(first + holeIndex) = *(first + secondChild);
		holeIndex = secondChild;
		secondChild = 2 * (secondChild + 1);
	}
}
template <class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first, 
				  	   RandomAccessIterator last,
				  	   RandomAccessIterator result,
				  	   T value, Distance*)
{
	*result = *first;
	__adjust_heap(first, Distance(0), Distance(last - first), value);
}
template <class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first,
						   RandomAccessIterator last, T*)
{
	__pop_heap(first, last - 1, last - 1, T(*(last-1)), 
		       distance_type(first));
}
template <class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first,
					 RandomAccessIterator last)
{
	__pop_heap_aux(first, last, value_type(first));
}

// 3. sort_heap algorithm
// Each pop_heap operation will put the max to the end
// So if iteratively pop the heap, could have a increasing sorted vector
// After sort, the heap no more exist
template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first,
			   RandomAccessIterator last)
{
	while(last - first > 1)
		pop_heap(first, last--);
}

// 4. make_heap algorithm
// Construct a max-heap from given data
template <class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first,
				 RandomAccessIterator last, T*, Distance*)
{
	if (last - first < 2)
		return;
	Distance len = last - first;
	Distance holeIndex = (len - 2) / 2;
	while (true)
	{
		__adjust_heap(first, holeIndex, len, T(*(first+holeIndex)));
		if (holeIndex == 0)
			return;
		holeIndex--;
	}
}


//====================================== INCLUDES ============================================
// See if the sorted range S2 is included in the sorted range S1
// Each range could have duplicate elements
template <class InputIterator1, class InputIterator2>
bool includes(InputIterator1 first1, InputIterator1 last1,
              InputIterator2 first2, InputIterator2 last2)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first2 < *first1)
            return false;
        else if(*first1 < *first2)
            ++first1;
        else
            ++first1, ++first2;
    }
    // if S2 elements are looped, then certainly first2 == last2, final result is true
    // if S1 elements are looped, then final result is true i.f.f. S2 are also looped
    return first2 == last2;
}
template <class InputIterator1, class InputIterator2, class Compare>
bool includes(InputIterator1 first1, InputIterator1 last1,
              InputIterator2 first2, InputIterator2 last2,
              Compare comp) //comp could not any Binary Operation as u like, but must do comparison
                            //otherwise the case 3 below would not make sense
{
    while(first1 != last1 && first2 != last2)
    {
        if(comp(*first2,*first1))        //case 1
            return false;
        else if(comp(*first1, *first2))  //case 2
            ++first1;
        else                             //case 3
            ++first1, ++first2;
    }
    return first2 == last2;
}


//===================================== MERGE ======================================================
// Merge two sorted ranges S1 and S2
// Default sorting order is less_than()
template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
                     InputIterator2 first2, InputIterator2 last2,
                     OutputIterator result)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first1 < *first2)
        {
            *result = *first1;
            ++first1;
        }
        else
        {
            *result = *first2;
            ++first2;
        }
        ++result;
    }
    return copy(first2, last2, copy(first1, last1, result));
}
template <class InputIterator1, class InputIterator2, class OutputIterator, class Compare>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
                     InputIterator2 first2, InputIterator2 last2,
                     OutputIterator result,
                     Compare comp)
{
    while(first1 != last1 && first2 != last2)
    {
        if(comp(*first1, *first2))
        {
            *result = *first1;
            ++first1;
        }
        else
        {
            *result = *first2;
            ++first2;
        }
        ++result;
    }
    return copy(first2, last2, copy(first1, last1, result));
}


//===================================== PARTITION ====================================================
// Predicate is kind of bool function, e.g. pred(x) == true if x is odd
// If an element in [first, last) satisfy the Predicate condition, then move to head part of the range
// Otherwise move to tail part of the range
template <class BidirectionalIterator, class Predicate>
BidirectionalIterator partition(BidirectionalIterator first, BidirectionalIterator last,
                                Predicate pred) //pred is kind of bool function,
                                                //e.g. pred(x) == true if x is odd
{
    while(true)
    {
        while(true)
        {
            if(first == last)
                return first;
            else if(pred(*first)) //*first satisfy pred condition, so stay there
                ++first;
            else                  // does not satisfy pred condition, so wait for move to tail part
                break;
        }
        --last;
        while(true)
        {
            if(first == last)
                return first;
            else if(!pred(*last)) //*last does not satisfy pred condition, so stay there
                --last;
            else                  // satisfy pred condition, so wait for move to head part
                break;
        }
        //
        iter_swap(first, last);
        ++first;
    }
}

//========================================= REMOVE ================================================
// remove_copy: remove all the elements equal to value, and copy the rest to a new container
template <class InputIterator, class OutputIterator, class T>
OutputIterator remove_copy(InputIterator first, InputIterator last,
                           OutputIterator result, const T& value)
{
    for(; first != last; ++first)
    {
        if(*first != value)
        {
            *result = *first;
            ++result;
        }
    }
    return result;
}
// remove_copy_if: Predicate is overload operator==
template <class InputIterator, class OutputIterator, class T,
          class Predicate>
OutputIterator remove_copy_if(InputIterator first, InputIterator last,
                           OutputIterator result, const T& value,
                           Predicate pred)
{
    for(; first != last; ++first)
    {
        if(!pred(*first, value))
        {
            *result = *first;
            ++result;
        }
    }
    return result;
}
// remove: remove all the elements equal to value, and copy the rest to a new container
template <class ForwardIterator, class T>
ForwardIterator remove(ForwardIterator first, ForwardIterator last,
                       const T& value)
{
    first = find(first, last, value);
    ForwardIterator next = first;
    return first == last ? first : remove_copy(++next, last, first, value);
}
// remove_if
template <class ForwardIterator, class Predicate>
ForwardIterator remove(ForwardIterator first, ForwardIterator last,
                       Predicate pred)
{
    first = find_if(first, last, pred);
    ForwardIterator next = first;
    return first == last ? first : remove_copy_if(++next, last, first, pred);
}


//==================================== REPLACE ===================================================
// replace
template <class ForwardIterator, class T>
void replace(ForwardIterator first, ForwardIterator last,
             const T& old_value, const T& new_value)
{
    for(; first != last; ++first)
    {
        if(*first == old_value)
            *first = new_value;
    }
}
// replace_copy
template <class ForwardIterator, class OutputIterator, class T>
OutputIterator replace_copy(ForwardIterator first, ForwardIterator last,
                            OutputIterator result,
                            const T& old_value, const T& new_value)
{
    for(; first != last; ++first, ++result)
        result = *first == old_value ? new_value : *first;
    return result;
}
// replace_if
template <class ForwardIterator, class T, class Predicate>
void replace_if(ForwardIterator first, ForwardIterator last,
                Predicate pred,
                const T& old_value, const T& new_value)
{
    for(; first != last; ++first)
    {
        if(pred(*first, old_value))
            *first = new_value;
    }
}
// replace_copy_if
template <class ForwardIterator, class OutputIterator, class Predicate, class T>
OutputIterator replace_copy(ForwardIterator first, ForwardIterator last,
                            OutputIterator result,
                            Predicate pred,
                            const T& old_value, const T& new_value)
{
    for(; first != last; ++first, ++result)
        result = pred(*first, old_value) ? new_value : *first;
    return result;
}


//===================================== REVERSE ===================================================
template <class BidirectionalIterator>
inline void reverse(BidirectionalIterator first, BidirectionalIterator last)
{
    __reverse(first, last, iterator_category(first));
}
template <class BidirectionalIterator>
void __reverse(BidirectionalIterator first, BidirectionalIterator last,
               bidirectional_iterator_tag)
{
    while(true)
    {
        if(first == last || first == --last)
            return;
        else
            iter_swap(first++, last);
    }
}
template <class RandomAccessIterator>
void __reverse(RandomAccessIterator first, RandomAccessIterator last,
               random_access_iterator_tag)    // only random_access_iterator could compare first < last
{
    while(first < last)
        iter_swap(first++, --last);
}
template <class BidirectionalIterator, class OutputIterator>
OutputIterator reverse_copy(BidirectionalIterator first, BidirectionalIterator last,
                            OutputIterator result)
{
    while(first != last)
    {
        --last;
        *result = *last;
        ++result;
    }
    return result;
}

//=================================== SEARCH ==================================================
// Find the first iterator where [first2, last2) appear in [first1, last1) for the first time
// If [first2, last2) is not included in [first1, last1), then return last1

// 1st version: default compare operator=
template <class ForwardIterator1, class ForwardIterator2>
inline ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1,
                               ForwardIterator2 first2, ForwardIterator2 last2)
{
    return __search(first1, last1, first2, last2,
                    distance_type(first1), distance_type(first2));
}
template <class ForwardIterator1, class ForwardIterator2, class Distance1, class Distance2>
inline ForwardIterator1 __search(ForwardIterator1 first1, ForwardIterator1 last1,
                               ForwardIterator2 first2, ForwardIterator2 last2,
                               Distance1*, Distance2*)
{
    Distance1 d1 = 0;
    distance(first1, last1, d1);
    Distance2 d2 = 0;
    distance(first2, last2, d2);

    if (d1 < d2)
        return last1;
    ForwardIterator1 current1 = first1;
    ForwardIterator2 current2 = first2;

    while(current2 != last2)
    {
        if (*current1 == *current2)
        {
            ++current1;
            ++current2;
        }
        else
        {
            if (d1 == d2)
                return last1;  //at least one element of S2 could not been found in S1, so return
            else               //d1 > d2, so still possible to find all elements of S2 in S1
            {
                current1 = ++first1;
                current2 = first2;
                --d1;
            }
        }
    }
    return first1;
}

// 2nd version: overload compare operator=
template <class ForwardIterator1, class ForwardIterator2, class Compare>
inline ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1,
                               ForwardIterator2 first2, ForwardIterator2 last2,
                               Compare comp)
{
    return __search(first1, last1, first2, last2, comp,
                    distance_type(first1), distance_type(first2));
}
template <class ForwardIterator1, class ForwardIterator2,
          class Distance1, class Distance2, class Compare>
inline ForwardIterator1 __search(ForwardIterator1 first1, ForwardIterator1 last1,
                               ForwardIterator2 first2, ForwardIterator2 last2,
                               Compare comp,
                               Distance1*, Distance2*)
{
    Distance1 d1 = 0;
    distance(first1, last1, d1);
    Distance2 d2 = 0;
    distance(first2, last2, d2);

    if (d1 < d2)
        return last1;
    ForwardIterator1 current1 = first1;
    ForwardIterator2 current2 = first2;

    while(current2 != last2)
    {
        if (comp(*current1, *current2))
        {
            ++current1;
            ++current2;
        }
        else
        {
            if (d1 == d2)
                return last1;  //at least one element of S2 could not been found in S1, so return
            else               //d1 > d2, so still possible to find all elements of S2 in S1
            {
                current1 = ++first1;
                current2 = first2;
                --d1;
            }
        }
    }
    return first1;
}


//=================================== LOWER_BOUND ==============================================
// In the sorted range [first,last), return the first iterator whose dereference <= the passed value
// If value > any one in the range, return the last

// 1st version: default compare operator<
template <class ForwardIterator, class T>
inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last,
                                   const T& value)
{
    return __lower_bound(first, last, value, 
                         difference_type(first), 
                         iterator_category(first));
}
template <class ForwardIterator, class T, class Distance>
ForwardIterator __lower_bound(ForwardIterator first, ForwardIterator last,
                              const T& value,
                              Distance*,
                              forward_iterator_tag)
{
    Distance len = 0;
    distance(first, last, len); //calculate the range length
    Distance half;
    ForwardIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        //below two lines let middle points to middle iterator of the range
        middle = first;
        advance(middle, half);
        if(*middle < value)
        {
            first = middle;
            ++first;
            len = len - half - 1;
        }
        else
            len = half;
    }
    return first;
}
template <class RandomAccessIterator, class T, class Distance>
RandomAccessIterator __lower_bound(RandomAccessIterator first, RandomAccessIterator last,
                              const T& value,
                              Distance*,
                              random_access_iterator_tag)
{
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        middle = first + half; // only random_access_iterator can do such operation
        if(*middle < value)
        {
            first = middle + 1;
            len = len - half - 1;
        }
        else
            len = half;
    }
    return first;
}

// 2nd version: comp overload operator<
template <class ForwardIterator, class T, class Compare>
inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last,
                                   const T& value, Compare comp)
{
    return __lower_bound(first, last, value, difference_type(first),
                         iterator_category(first), comp);
}
template <class ForwardIterator, class T, class Distance, class Compare>
ForwardIterator __lower_bound(ForwardIterator first, ForwardIterator last,
                              const T& value,
                              Distance*,
                              forward_iterator_tag,
                              Compare comp)
{
    Distance len = 0;
    distance(first, last, len); //calculate the range length
    Distance half;
    ForwardIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        //below two lines let middle points to middle iterator of the range
        middle = first;
        advance(middle, half);
        if(comp(*middle, value))
        {
            first = middle;
            ++first;
            len = len - half - 1;
        }
        else
            len = half;
    }
    return first;
}
template <class RandomAccessIterator, class T, class Distance, class Compare>
RandomAccessIterator __lower_bound(RandomAccessIterator first, RandomAccessIterator last,
                              const T& value,
                              Distance*,
                              random_access_iterator_tag,
                              Compare comp)
{
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        middle = first + half; // only random_access_iterator can do such operation
        if(comp(*middle, value))
        {
            first = middle + 1;
            len = len - half - 1;
        }
        else
            len = half;
    }
    return first;
}


//=================================== UPPER_BOUND ===========================================
// In the sorted range [first,last), return the next of the first iterator whose dereference >= the passed value
// If value < any one in the range, return the first

// 1st version: default compare operator<
template <class ForwardIterator, class T>
inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last,
                                   const T& value)
{
    return __upper_bound(first, last, value, difference_type(first), 
                         iterator_category(first));
}
template <class ForwardIterator, class T, class Distance>
ForwardIterator __upper_bound(ForwardIterator first, ForwardIterator last,
                              const T& value,
                              Distance*,
                              forward_iterator_tag)
{
    Distance len = 0;
    distance(first, last, len); //calculate the range length
    Distance half;
    ForwardIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        //below two lines let middle points to middle iterator of the range
        middle = first;
        advance(middle, half);
        if(value < *middle)
            len = half;
        else
        {
            first = middle;
            ++first;
            len = len - half - 1;
        }
    }
    return first;
}
template <class RandomAccessIterator, class T, class Distance>
RandomAccessIterator __upper_bound(RandomAccessIterator first, RandomAccessIterator last,
                              const T& value,
                              Distance*,
                              random_access_iterator_tag)
{
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        middle = first + half; // only random_access_iterator can do such operation
        if(value < *middle)
            len = half;
        else
        {
            first = middle + 1;
            len = len - half - 1;
        }
    }
    return first;
}

// 2nd version: comp overload operator<
template <class ForwardIterator, class T, class Compare>
inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last,
                                   const T& value, Compare comp)
{
    return __upper_bound(first, last, value, difference_type(first),
                         iterator_category(first), comp);
}
template <class ForwardIterator, class T, class Distance, class Compare>
ForwardIterator __upper_bound(ForwardIterator first, ForwardIterator last,
                              const T& value,
                              Distance*,
                              forward_iterator_tag,
                              Compare comp)
{
    Distance len = 0;
    distance(first, last, len); //calculate the range length
    Distance half;
    ForwardIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        //below two lines let middle points to middle iterator of the range
        middle = first;
        advance(middle, half);
        if(comp(value, *middle))
            len = half;
        else
        {
            first = middle;
            ++first;
            len = len - half - 1;
        }
    }
    return first;
}
template <class RandomAccessIterator, class T, class Distance, class Compare>
RandomAccessIterator __upper_bound(RandomAccessIterator first, RandomAccessIterator last,
                              const T& value,
                              Distance*,
                              random_access_iterator_tag,
                              Compare comp)
{
    Distance len = last - first;
    Distance half;
    RandomAccessIterator middle;

    while(len > 0)
    {
        half = len >> 1;
        middle = first + half; // only random_access_iterator can do such operation
        if(comp(value, *middle))
            len = half;
        else
        {
            first = middle + 1;
            len = len - half - 1;
        }
    }
    return first;
}


//================================== BINARY_SEARCH ============================================
// Binary search for a sorted range using lower_bound

template <class ForwardIterator, class T>
bool binary_search(ForwardIterator first, ForwardIterator last, const T& value)
{
    ForwardIterator i = lower_bound(first, last, value); // value >= *i
    return i != last && !(value < *i);
}

template <class ForwardIterator, class T, class Compare>
bool binary_search(ForwardIterator first, ForwardIterator last, const T& value, Compare comp)
{
    ForwardIterator i = lower_bound(first, last, value, comp);
    return i != last && !comp(value, *i);
}


//================================== NEXT_PERMUTATION ===========================================
template <class BidirectionalIterator>
bool next_permutation(BidirectionalIterator first, BidirectionalIterator last)
{
    BidirectionalIterator i = first;
    ++i;
    if (first == last || i == last)
        return false;
    i = last;
    --i;
    while(true)
    {
        BidirectionalIterator ii = i;
        --i;
        if (*i < *ii)
        {
            BidirectionalIterator j = last;
            while(!(*i < *j))
                --j;
            iter_swap(i, j);
            reverse(ii, last);
            return true;
        }
        if(i == first)
        {
            reverse(first, last);
            return false;
        }
    }
}

template <class BidirectionalIterator, class Compare>
bool next_permutation(BidirectionalIterator first, BidirectionalIterator last,
                      Compare comp)
{
    BidirectionalIterator i = first;
    ++i;
    if (first == last || i == last)
        return false;
    i = last;
    --i;
    while(true)
    {
        BidirectionalIterator ii = i;
        --i;
        if (comp(*i, *ii))
        {
            BidirectionalIterator j = last;
            while(!comp(*i, *j))
                --j;
            iter_swap(i, j);
            reverse(ii, last);
            return true;
        }
        if(i == first)
        {
            reverse(first, last);
            return false;
        }
    }
}

//================================== PREV_PERMUTATION ===========================================
template <class BidirectionalIterator>
bool prev_permutation(BidirectionalIterator first, BidirectionalIterator last)
{
    BidirectionalIterator i = first;
    ++i;
    if (first == last || i == last)
        return false;
    i = last;
    --i;
    while(true)
    {
        BidirectionalIterator ii = i;
        --i;
        if (*ii < *i)
        {
            BidirectionalIterator j = last;
            while(!(*j < *i))
                --j;
            iter_swap(i, j);
            reverse(ii, last);
            return true;
        }
        if(i == first)
        {
            reverse(first, last);
            return false;
        }
    }
}

template <class BidirectionalIterator, class Compare>
bool prev_permutation(BidirectionalIterator first, BidirectionalIterator last,
                      Compare comp)
{
    BidirectionalIterator i = first;
    ++i;
    if (first == last || i == last)
        return false;
    i = last;
    --i;
    while(true)
    {
        BidirectionalIterator ii = i;
        --i;
        if (comp(*ii, *i))
        {
            BidirectionalIterator j = last;
            while(!comp(*j, *i))
                --j;
            iter_swap(i, j);
            reverse(ii, last);
            return true;
        }
        if(i == first)
        {
            reverse(first, last);
            return false;
        }
    }
}

//================================== SORT ==================================================
// Sorting Algorithm for different containers:
//    1). Associative containers (map, set) --> already sorted by container definition using RB-Tree
//    2). Sequential container
//           * Stack, Queue, Priority_Queue --> do not allow sorting
//           * List                         --> member function sort() defined in List class
//           * Vector, Deque                --> could use this sort algorithm






} // end of namespace

#endif
