/*
 * lib/gssapi/generic/oid_ops.c
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

OM_uint32
generic_gss_release_oid(OM_uint32 *minor_status, gss_OID *oid)
{
    *minor_status = 0;

    *oid = GSS_C_NO_OID;
    return (GSS_S_COMPLETE);
}

OM_uint32
mech_gss_release_oid(OM_uint32 *minor_status,
		     gss_OID *oid, gss_mechanism gss_mech)
{
    *minor_status = 0;

    if (*oid == GSS_C_NO_OID)
	return (GSS_S_COMPLETE);

    if (gss_mech == NULL) {
	return (GSS_S_COMPLETE);
    }

    if (gss_mech->gss_internal_release_oid) {
	return (gss_mech->gss_internal_release_oid(minor_status, oid));
    }
    return (GSS_S_COMPLETE);
}

OM_uint32
generic_gss_copy_oid(OM_uint32 *minor_status,
		     gss_OID oid, gss_OID *new_oid)
{
    if (oid == GSS_C_NO_OID) {
	*new_oid = GSS_C_NO_OID;
	return (GSS_S_COMPLETE);
    }

    *new_oid = oid;
    return (GSS_S_COMPLETE);
}


OM_uint32
generic_gss_create_empty_oid_set(OM_uint32 *minor_status,
				 gss_OID_set *oid_set)
{
    if ((*oid_set = (gss_OID_set) malloc(sizeof(gss_OID_set_desc)))) {
	memset(*oid_set, 0, sizeof(gss_OID_set_desc));
	*minor_status = 0;
	return (GSS_S_COMPLETE);
    } else {
	*minor_status = ENOMEM;
	return (GSS_S_FAILURE);
    }
}

OM_uint32
generic_gss_add_oid_set_member(OM_uint32 *minor_status,
			       gss_OID member_oid, gss_OID_set *oid_set)
{
    gss_OID         elist;
    gss_OID         lastel;

    elist = (*oid_set)->elements;
    /*
     * Get an enlarged copy of the array
     */
    if (((*oid_set)->elements = (gss_OID) malloc(((*oid_set)->count + 1) *
						 sizeof(gss_OID_desc)))) {
	/*
	 * Copy in the old junk
	 */
	if (elist)
	    memcpy((*oid_set)->elements,
		   elist, ((*oid_set)->count * sizeof(gss_OID_desc)));

	/*
	 * Duplicate the input element
	 */
	lastel = &(*oid_set)->elements[(*oid_set)->count];
	if ((lastel->elements =
	     (void *) malloc((size_t) member_oid->length))) {
	    /*
	     * Success - copy elements
	     */
	    memcpy(lastel->elements, member_oid->elements,
		   (size_t) member_oid->length);
	    /*
	     * Set length
	     */
	    lastel->length = member_oid->length;

	    /*
	     * Update count
	     */
	    (*oid_set)->count++;
	    if (elist)
		free(elist);
	    *minor_status = 0;
	    return (GSS_S_COMPLETE);
	} else
	    free((*oid_set)->elements);
    }
    /*
     * Failure - restore old contents of list
     */
    (*oid_set)->elements = elist;
    *minor_status = ENOMEM;
    return (GSS_S_FAILURE);
}

OM_uint32
generic_gss_test_oid_set_member(OM_uint32 *minor_status,
				gss_OID member,
				gss_OID_set set, int *present)
{
    size_t          i;
    int             result;

    result = 0;
    for (i = 0; i < set->count; i++) {
	if ((set->elements[i].length == member->length) &&
	    !memcmp(set->elements[i].elements,
		    member->elements, (size_t) member->length)) {
	    result = 1;
	    break;
	}
    }
    *present = result;
    *minor_status = 0;
    return (GSS_S_COMPLETE);
}

/*
 * OID<->string routines.  These are uuuuugly.
 */
OM_uint32
generic_gss_oid_to_str(OM_uint32 *minor_status,
		       gss_OID oid, gss_buffer_t oid_str)
{
    char            numstr[128];
    unsigned long   number;
    int             numshift;
    size_t          string_length;
    size_t          i;
    unsigned char  *cp;
    char           *bp;

    /*
     * Decoded according to krb5/gssapi_krb5.c
     */

    /*
     * First determine the size of the string
     */
    string_length = 0;
    number = 0;
    numshift = 0;
    cp = (unsigned char *) oid->elements;
    number = (unsigned long) cp[0];
    sprintf(numstr, "%ld ", number / 40);
    string_length += strlen(numstr);
    sprintf(numstr, "%ld ", number % 40);
    string_length += strlen(numstr);
    for (i = 1; i < oid->length; i++) {
	if ((size_t) (numshift + 7) < (sizeof(unsigned long) * 8)) {
	    number = (number << 7) | (cp[i] & 0x7f);
	    numshift += 7;
	} else {
	    *minor_status = EINVAL;
	    return (GSS_S_FAILURE);
	}
	if ((cp[i] & 0x80) == 0) {
	    sprintf(numstr, "%ld ", number);
	    string_length += strlen(numstr);
	    number = 0;
	    numshift = 0;
	}
    }
    /*
     * If we get here, we've calculated the length of "n n n ... n ".  Add 4
     * here for "{ " and "}\0".
     */
    string_length += 4;
    if ((bp = (char *) malloc(string_length))) {
	strcpy(bp, "{ ");
	number = (unsigned long) cp[0];
	sprintf(numstr, "%ld ", number / 40);
	strcat(bp, numstr);
	sprintf(numstr, "%ld ", number % 40);
	strcat(bp, numstr);
	number = 0;
	cp = (unsigned char *) oid->elements;
	for (i = 1; i < oid->length; i++) {
	    number = (number << 7) | (cp[i] & 0x7f);
	    if ((cp[i] & 0x80) == 0) {
		sprintf(numstr, "%ld ", number);
		strcat(bp, numstr);
		number = 0;
	    }
	}
	strcat(bp, "}");
	oid_str->length = strlen(bp) + 1;
	oid_str->value = (void *) bp;
	*minor_status = 0;
	return (GSS_S_COMPLETE);
    }
    *minor_status = ENOMEM;
    return (GSS_S_FAILURE);
}

OM_uint32
generic_gss_str_to_oid(OM_uint32 *minor_status,
		       gss_buffer_t oid_str, gss_OID *oid)
{
    char           *cp,
                   *bp,
                   *startp;
    int             brace;
    long            numbuf;
    long            onumbuf;
    OM_uint32       nbytes;
    int             index;
    unsigned char  *op;

    brace = 0;
    bp = (char *) oid_str->value;
    cp = bp;
    /*
     * Skip over leading space
     */
    while ((bp < &cp[oid_str->length]) && isspace(*bp))
	bp++;
    if (*bp == '{') {
	brace = 1;
	bp++;
    }
    while ((bp < &cp[oid_str->length]) && isspace(*bp))
	bp++;
    startp = bp;
    nbytes = 0;

    /*
     * The first two numbers are chewed up by the first octet.
     */
    if (sscanf(bp, "%ld", &numbuf) != 1) {
	*minor_status = EINVAL;
	return (GSS_S_FAILURE);
    }
    while ((bp < &cp[oid_str->length]) && isdigit(*bp))
	bp++;
    while ((bp < &cp[oid_str->length]) && isspace(*bp))
	bp++;
    if (sscanf(bp, "%ld", &numbuf) != 1) {
	*minor_status = EINVAL;
	return (GSS_S_FAILURE);
    }
    while ((bp < &cp[oid_str->length]) && isdigit(*bp))
	bp++;
    while ((bp < &cp[oid_str->length]) && isspace(*bp))
	bp++;
    nbytes++;
    while (isdigit(*bp)) {
	if (sscanf(bp, "%ld", &numbuf) != 1) {
	    *minor_status = EINVAL;
	    return (GSS_S_FAILURE);
	}
	while (numbuf) {
	    nbytes++;
	    numbuf >>= 7;
	}
	while ((bp < &cp[oid_str->length]) && isdigit(*bp))
	    bp++;
	while ((bp < &cp[oid_str->length]) && isspace(*bp))
	    bp++;
    }
    if (brace && (*bp != '}')) {
	*minor_status = EINVAL;
	return (GSS_S_FAILURE);
    }

    /*
     * Phew!  We've come this far, so the syntax is good.
     */
    if ((*oid = (gss_OID) malloc(sizeof(gss_OID_desc)))) {
	if (((*oid)->elements = (void *) malloc((size_t) nbytes))) {
	    (*oid)->length = nbytes;
	    op = (unsigned char *) (*oid)->elements;
	    bp = startp;
	    sscanf(bp, "%ld", &numbuf);
	    while (isdigit(*bp))
		bp++;
	    while (isspace(*bp))
		bp++;
	    onumbuf = 40 * numbuf;
	    sscanf(bp, "%ld", &numbuf);
	    onumbuf += numbuf;
	    *op = (unsigned char) onumbuf;
	    op++;
	    while (isdigit(*bp))
		bp++;
	    while (isspace(*bp))
		bp++;
	    while (isdigit(*bp)) {
		sscanf(bp, "%ld", &numbuf);
		nbytes = 0;
		/*
		 * Have to fill in the bytes msb-first
		 */
		onumbuf = numbuf;
		while (numbuf) {
		    nbytes++;
		    numbuf >>= 7;
		}
		numbuf = onumbuf;
		op += nbytes;
		index = -1;
		while (numbuf) {
		    op[index] = (unsigned char) numbuf & 0x7f;
		    if (index != -1)
			op[index] |= 0x80;
		    index--;
		    numbuf >>= 7;
		}
		while (isdigit(*bp))
		    bp++;
		while (isspace(*bp))
		    bp++;
	    }
	    *minor_status = 0;
	    return (GSS_S_COMPLETE);
	} else {
	    free(*oid);
	    *oid = GSS_C_NO_OID;
	}
    }
    *minor_status = ENOMEM;
    return (GSS_S_FAILURE);
}

/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
OM_uint32
generic_gss_copy_oid_set(OM_uint32 *minor_status,
			 const gss_OID_set_desc *const oidset,
			 gss_OID_set *new_oidset)
{
    gss_OID_set_desc *copy;
    OM_uint32       minor = 0;
    OM_uint32       major = GSS_S_COMPLETE;
    OM_uint32       i;

    if (minor_status != NULL)
	*minor_status = 0;

    if (new_oidset != NULL)
	*new_oidset = GSS_C_NO_OID_SET;

    if (oidset == GSS_C_NO_OID_SET)
	return (GSS_S_CALL_INACCESSIBLE_READ);

    if (new_oidset == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if ((copy = (gss_OID_set_desc *) calloc(1, sizeof(*copy))) == NULL) {
	major = GSS_S_FAILURE;
	goto done;
    }

    if ((copy->elements = (gss_OID_desc *)
	 calloc(oidset->count, sizeof(*copy->elements))) == NULL) {
	major = GSS_S_FAILURE;
	goto done;
    }
    copy->count = oidset->count;

    for (i = 0; i < copy->count; i++) {
	gss_OID_desc   *out = &copy->elements[i];
	gss_OID_desc   *in = &oidset->elements[i];

	if ((out->elements = (void *) malloc(in->length)) == NULL) {
	    major = GSS_S_FAILURE;
	    goto done;
	}
	(void) memcpy(out->elements, in->elements, in->length);
	out->length = in->length;
    }

    *new_oidset = copy;
  done:
    if (major != GSS_S_COMPLETE) {
	(void) gss_release_oid_set(&minor, &copy);
    }

    return (major);
}
