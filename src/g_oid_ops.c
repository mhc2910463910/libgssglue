/*
 * lib/gssapi/mechglue/g_oid_ops.c
 *
 * Copyright 1995 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 */

/*
 * oid_ops.c - GSS-API V2 interfaces to manipulate OIDs
 */

#include "mglueP.h"
#include <stdio.h>
/*
 * should include to get protos #include "../generic/gssapiP_generic.h"
 */

extern gss_mechanism *__gss_mechs_array;

/*
 * Remove gss_release_oid() which was added in GSS-API V2 and
 * then removed in GSS-API V2 Revision 1. KWC 03/02/2005
 */

OM_uint32
gss_create_empty_oid_set(OM_uint32 *minor_status, gss_OID_set *oid_set)
{
    return generic_gss_create_empty_oid_set(minor_status, oid_set);
}

OM_uint32
gss_add_oid_set_member(OM_uint32 *minor_status,
		       gss_OID member_oid, gss_OID_set *oid_set)
{
    return generic_gss_add_oid_set_member(minor_status, member_oid,
					  oid_set);
}

OM_uint32
gss_test_oid_set_member(OM_uint32 *minor_status,
			gss_OID member, gss_OID_set set, int *present)
{
    return generic_gss_test_oid_set_member(minor_status, member, set,
					   present);
}

OM_uint32
gss_oid_to_str(OM_uint32 *minor_status, gss_OID oid, gss_buffer_t oid_str)
{
    return generic_gss_oid_to_str(minor_status, oid, oid_str);
}

OM_uint32
gss_str_to_oid(OM_uint32 *minor_status, gss_buffer_t oid_str, gss_OID *oid)
{
    return generic_gss_str_to_oid(minor_status, oid_str, oid);
}
