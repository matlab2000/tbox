/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2012, ruki All rights reserved.
 *
 * @author		ruki
 * @file		iterator.c
 * @ingroup 	container
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "iterator.h"
#include "../libc/libc.h"

/* ///////////////////////////////////////////////////////////////////////
 * integer
 */
static tb_size_t tb_iterator_int_head(tb_iterator_t* iterator)
{
	return 0;
}
static tb_size_t tb_iterator_int_tail(tb_iterator_t* iterator)
{
	return iterator->size;
}
static tb_size_t tb_iterator_int_next(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(itor < iterator->size, iterator->size);
	return itor + 1;
}
static tb_size_t tb_iterator_int_prev(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(itor, 0);
	return itor - 1;
}
static tb_pointer_t tb_iterator_int_item(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(itor < iterator->size, TB_NULL);
	return (tb_pointer_t)((tb_long_t*)iterator->data)[itor];
}
static tb_void_t tb_iterator_int_move(tb_iterator_t* iterator, tb_size_t itor, tb_cpointer_t item)
{
	tb_assert_return(itor < iterator->size);
	((tb_long_t*)iterator->data)[itor] = (tb_long_t)item;
}
static tb_long_t tb_iterator_int_comp(tb_iterator_t* iterator, tb_cpointer_t ltem, tb_cpointer_t rtem)
{
	return (tb_long_t)ltem - (tb_long_t)rtem;
}
/* ///////////////////////////////////////////////////////////////////////
 * string
 */
static tb_long_t tb_iterator_str_comp(tb_iterator_t* iterator, tb_cpointer_t ltem, tb_cpointer_t rtem)
{
	tb_assert_return_val(ltem && rtem, 0);
	return iterator->priv? tb_strcmp((tb_char_t const*)ltem, (tb_char_t const*)rtem) : tb_stricmp((tb_char_t const*)ltem, (tb_char_t const*)rtem);
}
/* ///////////////////////////////////////////////////////////////////////
 * pointer
 */
static tb_pointer_t tb_iterator_ptr_item(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(itor < iterator->size, TB_NULL);
	return ((tb_pointer_t*)iterator->data)[itor];
}
static tb_void_t tb_iterator_ptr_move(tb_iterator_t* iterator, tb_size_t itor, tb_cpointer_t item)
{
	tb_assert_return(itor < iterator->size);
	((tb_pointer_t*)iterator->data)[itor] = item;
}
static tb_long_t tb_iterator_ptr_comp(tb_iterator_t* iterator, tb_cpointer_t ltem, tb_cpointer_t rtem)
{
	return ltem - rtem;
}
/* ///////////////////////////////////////////////////////////////////////
 * memory
 */
static tb_pointer_t tb_iterator_mem_item(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(itor < iterator->size, TB_NULL);
	return (tb_pointer_t)((tb_byte_t*)iterator->data + itor * iterator->step);
}
static tb_pointer_t tb_iterator_mem_dupl(tb_iterator_t* iterator, tb_cpointer_t item)
{
	return item? tb_memdup(item, iterator->step) : tb_malloc(iterator->step);
}
static tb_void_t tb_iterator_mem_free(tb_iterator_t* iterator, tb_cpointer_t item)
{
	if (item) tb_free(item);
}
static tb_void_t tb_iterator_mem_move(tb_iterator_t* iterator, tb_size_t itor, tb_cpointer_t item)
{
	tb_assert_return(itor < iterator->size && item);
	tb_memcpy((tb_byte_t*)iterator->data + itor * iterator->step, item, iterator->step);
}
static tb_long_t tb_iterator_mem_comp(tb_iterator_t* iterator, tb_cpointer_t ltem, tb_cpointer_t rtem)
{
	tb_assert_return_val(ltem && rtem, 0);
	return tb_memcmp(ltem, rtem, iterator->step);
}
/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_iterator_t tb_iterator_int(tb_long_t* data, tb_size_t size)
{
	// check
	tb_assert(data && size);

	// init
	tb_iterator_t itor;
	itor.mode = TB_ITERATOR_MODE_FORWARD | TB_ITERATOR_MODE_REVERSE | TB_ITERATOR_MODE_RACCESS;
	itor.data = (tb_pointer_t)data;
	itor.size = size;
	itor.priv = TB_NULL;
	itor.step = sizeof(tb_long_t);
	itor.head = tb_iterator_int_head;
	itor.tail = tb_iterator_int_tail;
	itor.prev = tb_iterator_int_prev;
	itor.next = tb_iterator_int_next;
	itor.item = tb_iterator_int_item;
	itor.move = tb_iterator_int_move;
	itor.comp = tb_iterator_int_comp;

	// ok
	return itor;
}
tb_iterator_t tb_iterator_str(tb_char_t** data, tb_size_t size, tb_bool_t bcase)
{	
	// check
	tb_assert(data && size);

	// init
	tb_iterator_t itor;
	itor.mode = TB_ITERATOR_MODE_FORWARD | TB_ITERATOR_MODE_REVERSE | TB_ITERATOR_MODE_RACCESS;
	itor.data = (tb_pointer_t)data;
	itor.size = size;
	itor.priv = (tb_pointer_t)bcase;
	itor.step = sizeof(tb_char_t*);
	itor.head = tb_iterator_int_head;
	itor.tail = tb_iterator_int_tail;
	itor.prev = tb_iterator_int_prev;
	itor.next = tb_iterator_int_next;
	itor.item = tb_iterator_ptr_item;
	itor.move = tb_iterator_ptr_move;
	itor.comp = tb_iterator_str_comp;

	// ok
	return itor;
}
tb_iterator_t tb_iterator_ptr(tb_pointer_t* data, tb_size_t size)
{
	// check
	tb_assert(data && size);

	// init
	tb_iterator_t itor;
	itor.mode = TB_ITERATOR_MODE_FORWARD | TB_ITERATOR_MODE_REVERSE | TB_ITERATOR_MODE_RACCESS;
	itor.data = (tb_pointer_t)data;
	itor.size = size;
	itor.priv = TB_NULL;
	itor.step = sizeof(tb_pointer_t);
	itor.head = tb_iterator_int_head;
	itor.tail = tb_iterator_int_tail;
	itor.prev = tb_iterator_int_prev;
	itor.next = tb_iterator_int_next;
	itor.item = tb_iterator_ptr_item;
	itor.move = tb_iterator_ptr_move;
	itor.comp = tb_iterator_ptr_comp;

	// ok
	return itor;
}
tb_iterator_t tb_iterator_mem(tb_pointer_t data, tb_size_t size, tb_size_t step, tb_pointer_t temp)
{
	// check
	tb_assert(data && size && temp);

	// init
	tb_iterator_t itor;
	itor.mode = TB_ITERATOR_MODE_FORWARD | TB_ITERATOR_MODE_REVERSE | TB_ITERATOR_MODE_RACCESS;
	itor.data = (tb_pointer_t)data;
	itor.size = size;
	itor.priv = TB_NULL;
	itor.step = step;
	itor.head = tb_iterator_int_head;
	itor.tail = tb_iterator_int_tail;
	itor.prev = tb_iterator_int_prev;
	itor.next = tb_iterator_int_next;
	itor.item = tb_iterator_mem_item;
	itor.move = tb_iterator_mem_move;
	itor.comp = tb_iterator_mem_comp;

	// ok
	return itor;
}
tb_size_t tb_iterator_step(tb_iterator_t* iterator)
{
	tb_assert_and_check_return_val(iterator, 0);
	return iterator->step;
}
tb_size_t tb_iterator_head(tb_iterator_t* iterator)
{
	tb_assert_and_check_return_val(iterator && iterator->head, 0);
	return iterator->head(iterator);
}
tb_size_t tb_iterator_last(tb_iterator_t* iterator)
{
	return tb_iterator_prev(iterator, tb_iterator_tail(iterator));
}
tb_size_t tb_iterator_tail(tb_iterator_t* iterator)
{
	tb_assert_and_check_return_val(iterator && iterator->tail, 0);
	return iterator->tail(iterator);
}
tb_size_t tb_iterator_prev(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(iterator && iterator->prev, 0);
	return iterator->prev(iterator, itor);
}
tb_size_t tb_iterator_next(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(iterator && iterator->next, 0);
	return iterator->next(iterator, itor);
}
tb_pointer_t tb_iterator_item(tb_iterator_t* iterator, tb_size_t itor)
{
	tb_assert_return_val(iterator && iterator->item, TB_NULL);
	return iterator->item(iterator, itor);
}
tb_void_t tb_iterator_move(tb_iterator_t* iterator, tb_size_t itor, tb_cpointer_t item)
{
	tb_assert_return(iterator && iterator->move);
	return iterator->move(iterator, itor, item);
}
tb_long_t tb_iterator_comp(tb_iterator_t* iterator, tb_cpointer_t ltem, tb_cpointer_t rtem)
{
	tb_assert_return_val(iterator && iterator->comp, 0);
	return iterator->comp(iterator, ltem, rtem);
}

