#ifndef _MY_ALLOC_
#define _MY_ALLOC_

#include <stdlib.h> //for malloc, free, realloc, exit
#include <iostream> //for cerr, endl
#include "my_iterator.h"
#include "my_uninitialized.h"
#include "my_construct.h"
#include "my_algo.h"


namespace fyj
{

/*
 *  Standard interface for all the containers.
 *  Actual implementations are called Alloc::allocate and Alloc::deallocate.
 */
template<class T, class Alloc>
class simple_alloc
{
public:
	static T *allocate(size_t n)
		{return n == 0 ? 0 : (T*)Alloc::allocate(n * sizeof(T));}
	static T *allocate(void)
		{return (T*)Alloc::allocate(sizeof(T));}
	static void deallocate(T *p, size_t n)
		{if (n != 0) Alloc::deallocate(p, n * sizeof(T));}
	static void deallocate(T *p)
		{Alloc::deallocate(p, sizeof(T));}
};


/*=============1st-level allocator: __malloc_alloc_template=================
/* 
 * Use C-type malloc, realloc and free for allocation
 */

#if 0
#	include <new>
#	define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)
#	include <iostream>
#	define __THROW_BAD_ALLOC std::cerr<<"out of memory"<<std::endl;exit(1)
#endif

template <int inst> //inst is actually not used
class __malloc_alloc_template
{
private:
	// functions to handler out-of-memory (oom) issue
	static void *oom_malloc(size_t);
	static void *oom_realloc(void*, size_t);
	static void (*__malloc_alloc_oom_handler)();

public:
	static void *allocate(size_t n)
	{
		void *result = malloc(n);
		if (result == 0)
			result = oom_malloc(n);
		return result;
	}

	static void deallocate(void *p, size_t /*n*/) // n not really necessary
	{
		free(p);
	}

	static void *reallocate(void *p, size_t new_sz)
	{
		void *result = realloc(p, new_sz);
		if (result == 0)
			result = oom_realloc(p, new_sz);
		return result;
	}

	//Handle out-of-memory like set_new_handler() in C++
	//We could not use C++ new handler because we do not use ::operator new
	static void (*set_malloc_handler(void (*f)()))()
	{
		void (*old)() = __malloc_alloc_oom_handler;
		__malloc_alloc_oom_handler = f;
		return (old);
	}
};

template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void *__malloc_alloc_template<inst>::oom_malloc(size_t n)
{
	void (*my_malloc_handler)();
	void *result;

	//repeatly try to free, malloc, free, malloc...
	for(;;)
	{
		my_malloc_handler = __malloc_alloc_oom_handler;
		if (0 == my_malloc_handler)
			__THROW_BAD_ALLOC;
		(*my_malloc_handler)(); // try handle / free
		result = malloc(n);      // try malloc
		if (result)
			return result;	
	}
}

template <int inst>
void *__malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
	void (*my_malloc_handler)();
	void *result;

	//repeatly try to free, realloc, free, realloc...
	for(;;)
	{
		my_malloc_handler = __malloc_alloc_oom_handler;
		if (0 == my_malloc_handler)
			__THROW_BAD_ALLOC;
		(*my_malloc_handler)(); // try handle / free
		result = realloc(p, n);  // try realloc
		if (result)
			return result;	
	}
}

typedef __malloc_alloc_template<0> malloc_alloc;


/*======================2nd-level allocator: __default_alloc_template===================================*/
/* 
 * Objective: avoid lots of small memory blocks (memory fragment) and overhead
 * If the memory block:
 * 		> 128 bytes   --> use 1st-level allocator
 *      < 128 bytes   --> by memory pool / free-lists 
 * A free-list is a linked list of memory block
 */

const int __ALIGN = 8;
const int __MAX_BYTES = 128;
const int __NFREELISTS = __MAX_BYTES / __ALIGN; //Node of free-list = 16
                                                //16 free-list maintains the block of size 8, 16, 24,...,120, 128
//__default_alloc_template
//first params threads = true only when multithread
//second params inst not really used
template <bool threads, int inst>
class __default_alloc_template
{

private:
	//Round-up the byte to be a number 8x, e.g if byte = 14 then return 16 
	static size_t ROUND_UP(size_t byte)
	{
		return ( (byte) + __ALIGN - 1) & ~(__ALIGN - 1); //bit manipulation
	}

private:
	//structure of free-list node
	union obj
	{
		union obj *free_list_link;
		char client_data[1];
	};

private:
	//16 free-list nodes
	static obj *volatile free_list[__NFREELISTS];

	//determine which n_th node in the free-list to be used, acc. to size
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes) + __ALIGN-1)/__ALIGN - 1);
	}	

	//refill
	static void *refill(size_t n);

	//allocate a big block of memory for nb blocks with size of sz
	static char *chunk_alloc(size_t sz, int &nb);

	//chunk allocation state
	static char *start_free;  //start of memory pool
	static char *end_free;    //end of memory pool
	static size_t heap_size;

public:
	static void *allocate(size_t n)
	{
		obj *volatile *my_free_list;
		obj *result;

		//if n > 128 then use 1st-level allocator
		if (n > (size_t)__MAX_BYTES)
			return (malloc_alloc::allocate(n));
		//else find the proper node in the free-list
		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list;
		if (result == 0)
		{
			void *r = refill(ROUND_UP(n));
			return r;
		}
		//adjust free-list
		*my_free_list = result->free_list_link;
		return (result);
	}
	static void deallocate(void *p, size_t n)
	{
		obj *q = (obj*)p;
		obj *volatile *my_free_list;
		//if n > 128 then use 1st-level allocator
		if (n > (size_t)__MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
			return;
		}
		//else find the proper node in the free-list
		my_free_list = free_list + FREELIST_INDEX(n);
		//adjust free-list, take back of the block
		q->free_list_link = *my_free_list;
		*my_free_list = q;
	}
	static void *reallocate(void *p, size_t old_sz, size_t new_sz);

};

//static data member initialization
template <bool threads, int inst>
char *__default_alloc_template<threads, inst> :: start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst> :: end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst> :: heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj * volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] = 
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};


//refill
template <bool threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n)
{
	//default number of new block
	int nobjs = 20;
	char *chunk = chunk_alloc(n, nobjs);
	obj *volatile *my_free_list;
	obj *result;
	obj *current_obj, *next_obj;
	int i;

	if (1 == nobjs)
		return chunk;

	my_free_list = free_list + FREELIST_INDEX(n);

	result = (obj*)chunk;

	*my_free_list = next_obj = (obj*)(chunk+n);

	for(i = 1; ; i++)
	{
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i)
		{
			current_obj->free_list_link = 0;
			break;
		}else{
			current_obj->free_list_link = next_obj;
		}
	}
	return result;
}



template <bool threads, int inst>
char *__default_alloc_template<threads,inst>::
		chunk_alloc(size_t size, int& nobjs)
{
	char *result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;

	if(bytes_left >= total_bytes)
	{
		result = start_free;
		start_free += total_bytes;
		return(result);
	} else if (bytes_left >= size) {
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	} else {
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		if (bytes_left > 0)
		{
			obj *volatile *my_free_list = 
					free_list + FREELIST_INDEX(bytes_left);
		    ((obj*)start_free)->free_list_link = *my_free_list;
		    *my_free_list = (obj*)start_free;
		}
		start_free = (char*)malloc(bytes_to_get);
		if (0 == start_free)
		{
			int i;
			obj *volatile *my_free_list, *p;
			for (i = size; i <= __MAX_BYTES; i += __ALIGN)
			{
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (0 != p)
				{
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					return (chunk_alloc(size, nobjs));
				}
			}
			end_free = 0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return (chunk_alloc(size, nobjs));
	}
}

// Set the default alloc = second level allocator
#ifdef __USE_MALLOC
typedef __malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;
#else
typedef __default_alloc_template<0, 0> alloc;
#endif

} //end of namespace

#endif