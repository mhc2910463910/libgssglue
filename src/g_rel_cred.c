/*
 * #ident "@(#)gss_release_cred.c 1.15 95/08/07 SMI"
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
 *  glue routine for gss_release_cred
 */

#include "mglueP.h"
#include <stdio.h>
#include <stdlib.h>

OM_uint32
gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle)
{
    OM_uint32       status,
                    temp_status;
    int             j;
    gss_union_cred_t union_cred;
    gss_mechanism   mech;

    GSS_INITIALIZE;

    if (minor_status)
	*minor_status = 0;

    /*
     * if the cred_handle is null, return a NO_CRED error
     */

    if (cred_handle == GSS_C_NO_CREDENTIAL)
	return (GSS_S_NO_CRED);

    /*
     * Loop through the union_cred struct, selecting the approprate
     * underlying mechanism routine and calling it. At the end,
     * release all of the storage taken by the union_cred struct.
     */

    union_cred = (gss_union_cred_t) * cred_handle;
    *cred_handle = NULL;

    if (union_cred == NULL)
	return GSS_S_NO_CRED;

    status = GSS_S_COMPLETE;

    for (j = 0; j < union_cred->count; j++) {

	mech = __gss_get_mechanism(&union_cred->mechs_array[j]);

	if (union_cred->mechs_array[j].elements)
	    free(union_cred->mechs_array[j].elements);
	if (mech) {
	    if (mech->gss_release_cred) {
		temp_status = mech->gss_release_cred(minor_status,
						     &union_cred->cred_array
						     [j]);

		if (temp_status != GSS_S_COMPLETE)
		    status = GSS_S_NO_CRED;

	    } else
		status = GSS_S_NO_CRED;
	} else
	    status = GSS_S_NO_CRED;
    }

    gss_release_buffer(minor_status, &union_cred->auxinfo.name);
    free(union_cred->cred_array);
    free(union_cred->mechs_array);
    free(union_cred);

    return (status);
}
