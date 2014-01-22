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
 * @file		astream.c
 * @ingroup 	stream
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * trace
 */
//#define TB_TRACE_IMPL_TAG 				"astream"

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "astream.h"
#include "../network/network.h"
#include "../platform/platform.h"

/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */
static tb_bool_t tb_astream_oread_func(tb_astream_t* astream, tb_size_t state, tb_pointer_t priv)
{
	// check
	tb_astream_oread_t* oread = (tb_astream_oread_t*)priv;
	tb_assert_and_check_return_val(astream && astream->read && oread && oread->func, tb_false);

	// done
	do
	{
		// ok? 
		tb_check_break(state == TB_ASTREAM_STATE_OK);

		// reset state
		state = TB_ASTREAM_STATE_UNKNOWN_ERROR;
		
		// stoped?
		if (tb_atomic_get(&astream->stoped))
		{
			state = TB_ASTREAM_STATE_KILLED;
			break;
		}

		// read it
		if (!astream->read(astream, oread->func, oread->priv)) break;

		// ok
		state = TB_ASTREAM_STATE_OK;

	} while (0);

	// failed? done func
	if (state != TB_ASTREAM_STATE_OK) oread->func(astream, state, tb_null, 0, oread->priv);

	// ok
	return tb_true;
}
static tb_bool_t tb_astream_owrit_func(tb_astream_t* astream, tb_size_t state, tb_pointer_t priv)
{
	// check
	tb_astream_owrit_t* owrit = (tb_astream_owrit_t*)priv;
	tb_assert_and_check_return_val(astream && astream->writ && owrit && owrit->func, tb_false);

	// done
	do
	{
		// ok? 
		tb_check_break(state == TB_ASTREAM_STATE_OK);

		// reset state
		state = TB_ASTREAM_STATE_UNKNOWN_ERROR;
		
		// stoped?
		if (tb_atomic_get(&astream->stoped))
		{
			state = TB_ASTREAM_STATE_KILLED;
			break;
		}

		// check
		tb_assert_and_check_break(owrit->data && owrit->size);

		// writ it
		if (!astream->writ(astream, owrit->data, owrit->size, owrit->func, owrit->priv)) break;

		// ok
		state = TB_ASTREAM_STATE_OK;

	} while (0);

	// failed? done func
	if (state != TB_ASTREAM_STATE_OK) owrit->func(astream, state, 0, owrit->size, owrit->priv);

	// ok
	return tb_true;
}
static tb_bool_t tb_astream_oseek_func(tb_astream_t* astream, tb_size_t state, tb_pointer_t priv)
{
	// check
	tb_astream_oseek_t* oseek = (tb_astream_oseek_t*)priv;
	tb_assert_and_check_return_val(astream && astream->seek && oseek && oseek->func, tb_false);

	// done
	do
	{
		// ok? 
		tb_check_break(state == TB_ASTREAM_STATE_OK);

		// reset state
		state = TB_ASTREAM_STATE_UNKNOWN_ERROR;
		
		// stoped?
		if (tb_atomic_get(&astream->stoped))
		{
			state = TB_ASTREAM_STATE_KILLED;
			break;
		}

		// seek it
		if (!astream->seek(astream, oseek->offset, oseek->func, oseek->priv)) break;

		// ok
		state = TB_ASTREAM_STATE_OK;

	} while (0);

	// failed? done func
	if (state != TB_ASTREAM_STATE_OK) oseek->func(astream, state, 0, oseek->priv);

	// ok
	return tb_true;
}

/* ///////////////////////////////////////////////////////////////////////
 * interfaces
 */
tb_astream_t* tb_astream_init_from_url(tb_aicp_t* aicp, tb_char_t const* url)
{
	// check
	tb_assert_and_check_return_val(aicp && url, tb_null);

	// the init
	static tb_astream_t* (*g_init[])() = 
	{
		tb_null
	,	tb_astream_init_file
	,	tb_astream_init_sock
	,	tb_astream_init_http
	};

	// init
	tb_char_t const* 	p = url;
	tb_astream_t* 		astream = tb_null;
	tb_size_t 			type = TB_ASTREAM_TYPE_NONE;
	if (!tb_strnicmp(p, "http://", 7)) 			type = TB_ASTREAM_TYPE_HTTP;
	else if (!tb_strnicmp(p, "sock://", 7)) 	type = TB_ASTREAM_TYPE_SOCK;
	else if (!tb_strnicmp(p, "https://", 8)) 	type = TB_ASTREAM_TYPE_HTTP;
	else if (!tb_strnicmp(p, "socks://", 8)) 	type = TB_ASTREAM_TYPE_SOCK;
	else if (!tb_strstr(p, "://")) 				type = TB_ASTREAM_TYPE_FILE;
	else 
	{
		tb_trace("[astream]: unknown prefix for url: %s", url);
		return tb_null;
	}
	tb_assert_and_check_goto(type && type < tb_arrayn(g_init) && g_init[type], fail);

	// init stream
	astream = g_init[type](aicp);
	tb_assert_and_check_goto(astream, fail);

	// set url
	if (!tb_astream_ctrl(astream, TB_ASTREAM_CTRL_SET_URL, url)) goto fail;

	// ok
	return astream;

fail:
	
	// exit stream
	if (astream) tb_astream_exit(astream);
	return tb_null;
}
tb_void_t tb_astream_exit(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return(astream);

	// stop it first if not stoped
	if (!tb_atomic_get(&astream->stoped))
		tb_astream_kill(astream);

	// exit it
	if (astream->exit) astream->exit(astream);

	// exit url
	tb_url_exit(&astream->url);

	// free it
	tb_free(astream);
}
tb_void_t tb_astream_kill(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return(astream);

	// opened?
	tb_check_return(tb_atomic_get(&astream->opened));

	// stop it
	tb_check_return(!tb_atomic_fetch_and_set(&astream->stoped, 1));

	// kill it
	if (astream->kill) astream->kill(astream);
}
tb_bool_t tb_astream_pending(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return_val(astream, tb_false);

	// pending?
	tb_bool_t pending = tb_false;
	return tb_astream_ctrl((tb_astream_t*)astream, TB_ASTREAM_CTRL_IS_PENDING, &pending)? pending : tb_false;
}
tb_bool_t tb_astream_open_impl(tb_astream_t* astream, tb_astream_open_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->open && func, tb_false);
	
	// check state
	tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened) && tb_atomic_get(&astream->stoped), tb_false);

	// save debug info
#ifdef __tb_debug__
	astream->func = func_;
	astream->file = file_;
	astream->line = line_;
#endif

	// init state
	tb_atomic_set0(&astream->stoped);

	// open it
	tb_bool_t ok = astream->open(astream, func, priv);

	// post failed?
	if (!ok) tb_atomic_set(&astream->stoped, 1);

	// ok?
	return ok;
}
tb_bool_t tb_astream_read_impl(tb_astream_t* astream, tb_astream_read_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->read && func, tb_false);
	
	// check state
	tb_check_return_val(!tb_atomic_get(&astream->stoped), tb_false);
	tb_assert_and_check_return_val(tb_atomic_get(&astream->opened), tb_false);

	// save debug info
#ifdef __tb_debug__
	astream->func = func_;
	astream->file = file_;
	astream->line = line_;
#endif

	// read it
	return astream->read(astream, func, priv);
}
tb_bool_t tb_astream_writ_impl(tb_astream_t* astream, tb_byte_t const* data, tb_size_t size, tb_astream_writ_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->writ && data && size && func, tb_false);
	
	// check state
	tb_check_return_val(!tb_atomic_get(&astream->stoped), tb_false);
	tb_assert_and_check_return_val(tb_atomic_get(&astream->opened), tb_false);

	// save debug info
#ifdef __tb_debug__
	astream->func = func_;
	astream->file = file_;
	astream->line = line_;
#endif

	// writ it
	return astream->writ(astream, data, size, func, priv);
}
tb_bool_t tb_astream_seek_impl(tb_astream_t* astream, tb_hize_t offset, tb_astream_seek_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->seek && func, tb_false);
	
	// check state
	tb_check_return_val(!tb_atomic_get(&astream->stoped), tb_false);
	tb_assert_and_check_return_val(tb_atomic_get(&astream->opened), tb_false);

	// save debug info
#ifdef __tb_debug__
	astream->func = func_;
	astream->file = file_;
	astream->line = line_;
#endif

	// seek it
	return astream->seek(astream, offset, func, priv);
}
tb_bool_t tb_astream_sync_impl(tb_astream_t* astream, tb_astream_sync_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->sync && func, tb_false);
	
	// check state
	tb_check_return_val(!tb_atomic_get(&astream->stoped), tb_false);
	tb_assert_and_check_return_val(tb_atomic_get(&astream->opened), tb_false);

	// save debug info
#ifdef __tb_debug__
	astream->func = func_;
	astream->file = file_;
	astream->line = line_;
#endif

	// sync it
	return astream->sync(astream, func, priv);
}
tb_bool_t tb_astream_oread_impl(tb_astream_t* astream, tb_astream_read_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->open && astream->read && func, tb_false);

	// no opened? open it first
	if (!tb_atomic_get(&astream->opened))
	{
		// init open and read
		astream->open_and.read.func = func;
		astream->open_and.read.priv = priv;
		return tb_astream_open_impl(astream, tb_astream_oread_func, &astream->open_and.read __tb_debug_args__);
	}

	// read it
	return tb_astream_read_impl(astream, func, priv __tb_debug_args__);
}
tb_bool_t tb_astream_owrit_impl(tb_astream_t* astream, tb_byte_t const* data, tb_size_t size, tb_astream_writ_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->open && astream->writ && data && size && func, tb_false);

	// no opened? open it first
	if (!tb_atomic_get(&astream->opened))
	{
		// init open and writ
		astream->open_and.writ.func = func;
		astream->open_and.writ.priv = priv;
		astream->open_and.writ.data = data;
		astream->open_and.writ.size = size;
		return tb_astream_open_impl(astream, tb_astream_owrit_func, &astream->open_and.writ __tb_debug_args__);
	}

	// writ it
	return tb_astream_writ_impl(astream, data, size, func, priv __tb_debug_args__);
}
tb_bool_t tb_astream_oseek_impl(tb_astream_t* astream, tb_hize_t offset, tb_astream_seek_func_t func, tb_pointer_t priv __tb_debug_decl__)
{
	// check
	tb_assert_and_check_return_val(astream && astream->open && astream->seek && func, tb_false);

	// no opened? open it first
	if (!tb_atomic_get(&astream->opened))
	{
		// init open and seek
		astream->open_and.seek.func = func;
		astream->open_and.seek.priv = priv;
		astream->open_and.seek.offset = offset;
		return tb_astream_open_impl(astream, tb_astream_oseek_func, &astream->open_and.seek __tb_debug_args__);
	}

	// seek it
	return tb_astream_seek_impl(astream, offset, func, priv __tb_debug_args__);
}
tb_aicp_t* tb_astream_aicp(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return_val(astream, tb_null);

	// the aicp
	return astream->aicp;
}
tb_size_t tb_astream_type(tb_astream_t const* astream)
{
	// check
	tb_assert_and_check_return_val(astream, TB_ASTREAM_TYPE_NONE);

	// the type
	return astream->type;
}
tb_hize_t tb_astream_size(tb_astream_t const* astream)
{
	// check
	tb_assert_and_check_return_val(astream, 0);

	// get the size
	tb_hize_t size = 0;
	return tb_astream_ctrl((tb_astream_t*)astream, TB_ASTREAM_CTRL_GET_SIZE, &size)? size : 0;
}
tb_hong_t tb_astream_left(tb_astream_t const* astream)
{
	// check
	tb_assert_and_check_return_val(astream, -1);
	
	// the offset
	tb_hong_t offset = tb_astream_offset(astream);
	tb_check_return_val(offset >= 0, -1);

	// the size
	tb_hize_t size = tb_astream_size(astream);

	// the left
	return ((size && size >= offset)? (size - offset) : -1);
}
tb_hong_t tb_astream_offset(tb_astream_t const* astream)
{
	// check
	tb_assert_and_check_return_val(astream, -1);

	// get the offset
	tb_hize_t offset = 0;
	return tb_astream_ctrl((tb_astream_t*)astream, TB_ASTREAM_CTRL_GET_OFFSET, &offset)? offset : -1;
}
tb_size_t tb_astream_timeout(tb_astream_t const* astream)
{
	// check
	tb_assert_and_check_return_val(astream, 0);

	// get the timeout
	tb_size_t timeout = 0;
	return tb_astream_ctrl((tb_astream_t*)astream, TB_ASTREAM_CTRL_GET_TIMEOUT, &timeout)? timeout : 0;
}
tb_char_t const* tb_astream_state_cstr(tb_size_t state)
{
	// done
	switch (state)
	{
	case TB_ASTREAM_STATE_OK: 					return "ok";
	case TB_ASTREAM_STATE_CLOSED: 				return "closed";
	case TB_ASTREAM_STATE_KILLED: 				return "killed";
	case TB_ASTREAM_STATE_NOT_SUPPORTED: 		return "not supported";
	case TB_ASTREAM_STATE_UNKNOWN_ERROR: 		return "unknown error";
	case TB_ASTREAM_SOCK_STATE_DNS_FAILED: 		return "sock: dns: failed";
	case TB_ASTREAM_SOCK_STATE_CONNECT_FAILED: 	return "sock: connect: failed";
	case TB_ASTREAM_SOCK_STATE_CONNECT_TIMEOUT: return "sock: connect: timeout";
	case TB_ASTREAM_SOCK_STATE_RECV_TIMEOUT: 	return "sock: recv: timeout";
	case TB_ASTREAM_SOCK_STATE_SEND_TIMEOUT: 	return "sock: send: timeout";
	default: 									return "unknown";
	}

	return tb_null;
}
tb_bool_t tb_astream_ctrl(tb_astream_t* astream, tb_size_t ctrl, ...)
{
	// check
	tb_assert_and_check_return_val(astream && astream->ctrl, tb_false);

	// init args
	tb_va_list_t args;
    tb_va_start(args, ctrl);

	// ctrl
	tb_bool_t ret = tb_false;
	switch (ctrl)
	{
	case TB_ASTREAM_CTRL_SET_URL:
		{
			// check
			tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened), tb_false);

			// set url
			tb_char_t const* url = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			if (url && tb_url_set(&astream->url, url)) ret = tb_true;
		}
		break;
	case TB_ASTREAM_CTRL_GET_URL:
		{
			// get url
			tb_char_t const** purl = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			if (purl)
			{
				tb_char_t const* url = tb_url_get(&astream->url);
				if (url)
				{
					*purl = url;
					ret = tb_true;
				}
			}
		}
		break;
	case TB_ASTREAM_CTRL_SET_HOST:
		{
			// check
			tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened), tb_false);

			// set host
			tb_char_t const* host = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			if (host)
			{
				tb_url_host_set(&astream->url, host);
				ret = tb_true;
			}
		}
		break;
	case TB_ASTREAM_CTRL_GET_HOST:
		{
			// get host
			tb_char_t const** phost = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			if (phost)
			{
				tb_char_t const* host = tb_url_host_get(&astream->url);
				if (host)
				{
					*phost = host;
					ret = tb_true;
				}
			}
		}
		break;
	case TB_ASTREAM_CTRL_SET_PORT:
		{
			// check
			tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened), tb_false);

			// set port
			tb_size_t port = (tb_size_t)tb_va_arg(args, tb_size_t);
			if (port)
			{
				tb_url_port_set(&astream->url, port);
				ret = tb_true;
			}
		}
		break;
	case TB_ASTREAM_CTRL_GET_PORT:
		{
			// get port
			tb_size_t* pport = (tb_size_t*)tb_va_arg(args, tb_size_t*);
			if (pport)
			{
				*pport = tb_url_port_get(&astream->url);
				ret = tb_true;
			}
		}
		break;
	case TB_ASTREAM_CTRL_SET_PATH:
		{
			// check
			tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened), tb_false);

			// set path
			tb_char_t const* path = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			if (path)
			{
				tb_url_path_set(&astream->url, path);
				ret = tb_true;
			}
		}
		break;
	case TB_ASTREAM_CTRL_GET_PATH:
		{
			// get path
			tb_char_t const** ppath = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			if (ppath)
			{
				tb_char_t const* path = tb_url_path_get(&astream->url);
				if (path)
				{
					*ppath = path;
					ret = tb_true;
				}
			}
		}
		break;
	case TB_ASTREAM_CTRL_SET_TIMEOUT:
		{
			// check
			tb_assert_and_check_return_val(!tb_atomic_get(&astream->opened), tb_false);

			// set timeout
			tb_long_t timeout = (tb_long_t)tb_va_arg(args, tb_long_t);
			astream->timeout = timeout;
			ret = tb_true;
		}
		break;
	case TB_ASTREAM_CTRL_GET_TIMEOUT:
		{
			// get timeout
			tb_long_t* ptimeout = (tb_long_t*)tb_va_arg(args, tb_long_t*);
			if (ptimeout)
			{
				*ptimeout = astream->timeout;
				ret = tb_true;
			}
		}
		break;
	case TB_ASTREAM_CTRL_IS_OPENED:
		{
			tb_bool_t* popened = (tb_bool_t*)tb_va_arg(args, tb_bool_t*);
			if (popened)
			{
				*popened = tb_atomic_get(&astream->opened)? tb_true : tb_false;
				ret = tb_true;
			}
		}
		break;
	default:
		break;
	}

	// reset args
	tb_va_end(args);
    tb_va_start(args, ctrl);

	// ctrl for native stream
	ret = (astream->ctrl(astream, ctrl, args) || ret)? tb_true : tb_false;

	// exit args
	tb_va_end(args);

	// ok?
	return ret;
}
#ifdef __tb_debug__
tb_char_t const* tb_astream_func(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return_val(astream, tb_null);

	// the func
	return astream->func;
}
tb_char_t const* tb_astream_file(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return_val(astream, tb_null);

	// the file
	return astream->file;
}
tb_size_t tb_astream_line(tb_astream_t* astream)
{
	// check
	tb_assert_and_check_return_val(astream, 0);

	// the line
	return astream->line;
}
#endif