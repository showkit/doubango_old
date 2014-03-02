/*
* Copyright (C) 2010-2011 Mamadou Diop.
*
* Contact: Mamadou Diop <diopmamadou(at)doubango[dot]org>
*	
* This file is part of Open Source Doubango Framework.
*
* DOUBANGO is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*	
* DOUBANGO is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*	
* You should have received a copy of the GNU General Public License
* along with DOUBANGO.
*
*/

/**@file tsk_object.c
 * @brief Base object implementation.
 *
 * @author Mamadou Diop <diopmamadou(at)doubango[dot]org>
 *

 */
#include "tsk_object.h"
#include "tsk_memory.h"
#include "tsk_debug.h"
#include "tsk_common.h"

/**@defgroup tsk_object_group Base object implementation.
* @brief Provides utility functions to ease Object Oriented Programming in C.
*/

#if defined (_DEBUG) || defined (DEBUG)
#	define TSK_DEBUG_OBJECTS	0
static int tsk_objects_count = 0;
#else
#	define TSK_DEBUG_OBJECTS	0
#endif

/**@ingroup tsk_object_group
* Creates new object. The object MUST be declared using @ref TSK_DECLARE_OBJECT macro.
* @param objdef The object meta-data (definition). For more infomation see @ref tsk_object_def_t.
* @param ... List of parameters to pass to the constructor(defined in the meta-data).
* @retval @ref tsk_object_t object with a reference counter equal to 1.
* @sa @ref tsk_object_new_2.
*/
tsk_object_t* tsk_object_new(const tsk_object_def_t *objdef, ...)
{
	// Do not check "objdef", let the application die if it's null
	tsk_object_t *newobj = tsk_calloc(1, objdef->size);
	if(newobj){
		(*(const tsk_object_def_t **) newobj) = objdef;
		TSK_OBJECT_HEADER(newobj)->refCount = 1;
		if(objdef->constructor){ 
			va_list ap;
			tsk_object_t * newobj_ = newobj;// save
			va_start(ap, objdef);
			newobj = objdef->constructor(newobj, &ap); // must return new
			va_end(ap);

			if(!newobj){ // null if constructor failed to initialized the object
				if(objdef->destructor){
					objdef->destructor(newobj_);
				}
				tsk_free(&newobj_);
			}

#if TSK_DEBUG_OBJECTS
		TSK_DEBUG_INFO("N∞ objects:%d", ++tsk_objects_count);
#endif
		}
		else{
			TSK_DEBUG_WARN("No constructor found.");
		}
	}
	else{
		TSK_DEBUG_ERROR("Failed to create new tsk_object.");
	}

	return newobj;
}

/**@ingroup tsk_object_group
* Creates new object. The object MUST be declared using @ref TSK_DECLARE_OBJECT macro.
* @param objdef The object meta-data (definition). For more infomation see @ref tsk_object_def_t.
* @param ap Variable argument list to pass to the constructor(defined in the meta-data).
* @retval @ref tsk_object_t object with a reference counter equal to 1.
* @sa @ref tsk_object_new.
*/
tsk_object_t* tsk_object_new_2(const tsk_object_def_t *objdef, va_list* ap)
{
	tsk_object_t *newobj = tsk_calloc(1, objdef->size);
	if(newobj){
		(*(const tsk_object_def_t **) newobj) = objdef;
		TSK_OBJECT_HEADER(newobj)->refCount = 1;
		if(objdef->constructor){ 
			newobj = objdef->constructor(newobj, ap);

#if TSK_DEBUG_OBJECTS
		TSK_DEBUG_INFO("N∞ objects:%d", ++tsk_objects_count);
#endif
		}
		else{
			TSK_DEBUG_WARN("No constructor found.");
		}
	}
	else{
		TSK_DEBUG_ERROR("Failed to create new tsk_object.");
	}

	return newobj;
}

/**@ingroup tsk_object_group
* Gets the size of an opaque object.
* @param self The object for which we want to get the size.
* The object MUST be declared using @ref TSK_DECLARE_OBJECT macro and created using @ref tsk_object_new or @ref tsk_object_new_2.
* @retval The size of the object.
*/
tsk_size_t tsk_object_sizeof(const tsk_object_t *self)
{
	const tsk_object_def_t **objdef = (const tsk_object_def_t **)self;
	if(objdef && *objdef){
		return (*objdef)->size;
	}
	else{
		TSK_DEBUG_ERROR("NULL object definition.");
		return 0;
	}
}

/**@ingroup tsk_object_group
* Compares two well-defined objects.
* If the meta-data (definition) of the first object (@a object1) do not include a function comparator then this method will amlways return -1.
* @param object1 The first object to compare.
* @param object2 The second object to compare.
* @retval Zero if the two object are equal.
* Positive value if @a object1 is greater than @a object2 and a negative value otherwise.
*/
int tsk_object_cmp(const tsk_object_t *object1, const tsk_object_t *object2)
{
	const tsk_object_def_t **objdef = (const tsk_object_def_t **)object1;

	if(objdef && *objdef && (*objdef)->comparator){
		return (*objdef)->comparator(object1, object2);
	}
	return ((int*)object1 - (int*)object2);
}

/**@ingroup tsk_object_group
* Increment the refrence counting of the object.<br>
* Refernce counting: http://en.wikipedia.org/wiki/Reference_counting.<br>
* The object MUST be declared using @ref TSK_DECLARE_OBJECT macro and created using @ref tsk_object_new or @ref tsk_object_new_2.
* @param self The object holding the counter to increment.
* @retval The new object (incremented).
* @sa tsk_object_unref.
*/
tsk_object_t* tsk_object_ref(tsk_object_t *self)
{
	tsk_object_header_t* objhdr = TSK_OBJECT_HEADER(self);
	if(objhdr && objhdr->refCount){
		objhdr->refCount++;
		return self;
	}
	return tsk_null;
}

/**@ingroup tsk_object_group
* Decrement the refrence counting of the object.<br>
* Refernce counting: http://en.wikipedia.org/wiki/Reference_counting.<br>
* The object MUST be declared using @ref TSK_DECLARE_OBJECT macro and created using @ref tsk_object_new or @ref tsk_object_new_2.
* @param self The object holding the counter to decrement.
* @retval If the refernce counter is equal to zero then NULL is returned otherwise a new object (decremented) is returned.
* @sa ref tsk_object_ref.
* @sa ref TSK_OBJECT_SAFE_FREE.
*/
tsk_object_t* tsk_object_unref(tsk_object_t *self)
{
	if(self){
		tsk_object_header_t* objhdr = TSK_OBJECT_HEADER(self);
		if(objhdr->refCount){ // If refCount is == 0 then, nothing should happen.
			if(!--objhdr->refCount){
				tsk_object_delete(self);
				return tsk_null;
			}
		}
		else{
			return tsk_null;
		}
	}
	return self;
}

tsk_size_t tsk_object_get_refcount(tsk_object_t *self)
{
	return self ? TSK_OBJECT_HEADER(self)->refCount : 0;
}

/**@ingroup tsk_object_group
* Delete an object. This function will delete the object even if it's reference counter is greater than 1.
* This mean that this function is not safe. You should use @ref TSK_OBJECT_SAFE_FREE to safely delete an object.
* The object MUST be declared using @ref TSK_DECLARE_OBJECT macro and created using @ref tsk_object_new or @ref tsk_object_new_2.
* @param self The object to delete.
* @sa @ref TSK_OBJECT_SAFE_FREE.
*/
void tsk_object_delete(tsk_object_t *self)
{
	const tsk_object_def_t ** objdef = self;
	if(self && *objdef){
		if((*objdef)->destructor){
			self = (*objdef)->destructor(self);
#if TSK_DEBUG_OBJECTS
		TSK_DEBUG_INFO("N∞ objects:%d", --tsk_objects_count);
#endif
		}
		else{
			TSK_DEBUG_WARN("No destructor found.");
		}
		free(self);
	}
}

