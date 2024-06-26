/*
 * #ident "@(#)gss_release_oid_set.c 1.12 95/08/23 SMI"
 */

/*
 * Copyright 1996 by Sun Microsystems, Inc.
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
 *  glue routine for gss_release_oid_set
 */

#include "mglueP.h"
#include <stdio.h>
#include <stdlib.h>

OM_uint32
gss_release_oid_set(OM_uint32 *minor_status, gss_OID_set *set)
{
    size_t          index;
    gss_OID         oid;
    if (minor_status)
	*minor_status = 0;

    if (set == NULL)
	return GSS_S_COMPLETE;

    if (*set == GSS_C_NULL_OID_SET)
	return (GSS_S_COMPLETE);

    for (index = 0; index < (*set)->count; index++) {
	oid = &(*set)->elements[index];
	free(oid->elements);
    }
    free((*set)->elements);
    free(*set);

    *set = GSS_C_NULL_OID_SET;

    return (GSS_S_COMPLETE);
}
