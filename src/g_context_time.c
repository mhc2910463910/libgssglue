/*
 * #ident "@(#)gss_context_time.c 1.8 95/08/07 SMI"
 */

/*
 * Copyright 1996, 2023 by Sun Microsystems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Sun Microsystems not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. Sun Microsystems makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 *  glue routines for gss_context_time
 */

#include "mglueP.h"

OM_uint32
gss_context_time(OM_uint32 *minor_status,
		 gss_ctx_id_t context_handle, OM_uint32 *time_rec)
{
    OM_uint32       status;
    gss_union_ctx_id_t ctx;
    gss_mechanism   mech;

    GSS_INITIALIZE;

    if (context_handle == GSS_C_NO_CONTEXT)
	return GSS_S_NO_CONTEXT;

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = __gss_get_mechanism(ctx->mech_type);

    if (mech) {

	if (mech->gss_context_time)
	    status = mech->gss_context_time(minor_status,
					    ctx->internal_ctx_id,
					    time_rec);
	else
	    status = GSS_S_BAD_BINDINGS;

	return (status);
    }

    return (GSS_S_NO_CONTEXT);
}
