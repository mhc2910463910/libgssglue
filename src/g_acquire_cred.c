/*
 * #pragma ident "@(#)g_acquire_cred.c 1.22 04/02/23 SMI"
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
 *  glue routine for gss_acquire_cred
 */
// Define these away
#define map_errcode(code)
#define map_error(code, mech)

#include "mglueP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static          OM_uint32
val_acq_cred_args(OM_uint32 *minor_status,
		  gss_name_t desired_name,
		  OM_uint32 time_req,
		  gss_OID_set desired_mechs,
		  int cred_usage,
		  gss_cred_id_t *output_cred_handle,
		  gss_OID_set *actual_mechs, OM_uint32 *time_rec)
{

    /*
     * Initialize outputs.
     */

    if (minor_status != NULL)
	*minor_status = 0;

    if (output_cred_handle != NULL)
	*output_cred_handle = GSS_C_NO_CREDENTIAL;

    if (actual_mechs != NULL)
	*actual_mechs = GSS_C_NULL_OID_SET;

    if (time_rec != NULL)
	*time_rec = 0;

    /*
     * Validate arguments.
     */

    if (minor_status == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (output_cred_handle == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (cred_usage != GSS_C_ACCEPT
	&& cred_usage != GSS_C_INITIATE && cred_usage != GSS_C_BOTH) {
	if (minor_status) {
	    *minor_status = EINVAL;
	    map_errcode(minor_status);
	}
	return GSS_S_FAILURE;
    }

    return (GSS_S_COMPLETE);
}


OM_uint32
gss_acquire_cred(OM_uint32 *minor_status,
		 gss_name_t desired_name,
		 OM_uint32 time_req,
		 gss_OID_set desired_mechs,
		 int cred_usage,
		 gss_cred_id_t *output_cred_handle,
		 gss_OID_set *actual_mechs, OM_uint32 *time_rec)
{
    OM_uint32       major = GSS_S_FAILURE;
    OM_uint32       initTimeOut,
                    acceptTimeOut,
                    outTime = GSS_C_INDEFINITE;
    gss_OID_set_desc default_OID_set;
    gss_OID_set     mechs;
    gss_OID_desc    default_OID;
    gss_mechanism   mech;
    unsigned int    i;
    gss_union_cred_t creds;

    major = val_acq_cred_args(minor_status,
			      desired_name,
			      time_req,
			      desired_mechs,
			      cred_usage,
			      output_cred_handle, actual_mechs, time_rec);
    if (major != GSS_S_COMPLETE)
	return (major);

    /*
     * Initial value needed below.
     */
    major = GSS_S_FAILURE;

    /*
     * if desired_mechs equals GSS_C_NULL_OID_SET, then pick an
     * appropriate default.  We use the first mechanism in the
     * mechansim list as the default. This set is created with
     * statics thus needs not be freed
     */
    if (desired_mechs == GSS_C_NULL_OID_SET) {
	mech = __gss_get_mechanism(NULL);
	if (mech == NULL)
	    return (GSS_S_BAD_MECH);

	mechs = &default_OID_set;
	default_OID_set.count = 1;
	default_OID_set.elements = &default_OID;
	default_OID.length = mech->mech_type.length;
	default_OID.elements = mech->mech_type.elements;
    } else
	mechs = desired_mechs;

    if (mechs->count == 0)
	return (GSS_S_BAD_MECH);

    /*
     * allocate the output credential structure
     */
    creds = (gss_union_cred_t) malloc(sizeof(gss_union_cred_desc));
    if (creds == NULL)
	return (GSS_S_FAILURE);

    /*
     * initialize to 0s
     */
    (void) memset(creds, 0, sizeof(gss_union_cred_desc));

    /*
     * for each requested mech attempt to obtain a credential
     */
    for (i = 0; i < mechs->count; i++) {
	major = gss_add_cred(minor_status, (gss_cred_id_t) creds,
			     desired_name,
			     &mechs->elements[i],
			     cred_usage, time_req, time_req, NULL,
			     NULL, &initTimeOut, &acceptTimeOut);
	if (major == GSS_S_COMPLETE) {
	    /*
	     * update the credential's time
	     */
	    if (cred_usage == GSS_C_ACCEPT) {
		if (outTime > acceptTimeOut)
		    outTime = acceptTimeOut;
	    } else if (cred_usage == GSS_C_INITIATE) {
		if (outTime > initTimeOut)
		    outTime = initTimeOut;
	    } else {
		/*
		 * time_rec is the lesser of the
		 * init/accept times
		 */
		if (initTimeOut > acceptTimeOut)
		    outTime = (outTime > acceptTimeOut) ?
			acceptTimeOut : outTime;
		else
		    outTime = (outTime > initTimeOut) ?
			initTimeOut : outTime;
	    }
	}
    }				/* for */

    /*
     * ensure that we have at least one credential element
     */
    if (creds->count < 1) {
	free(creds);
	return (major);
    }

    /*
     * fill in output parameters
     * setup the actual mechs output parameter
     */
    if (actual_mechs != NULL) {
	gss_OID_set_desc oids;

	oids.count = creds->count;
	oids.elements = creds->mechs_array;

	major =
	    generic_gss_copy_oid_set(minor_status, &oids, actual_mechs);
	if (GSS_ERROR(major)) {
	    (void) gss_release_cred(minor_status,
				    (gss_cred_id_t *) & creds);
	    return (major);
	}
    }

    if (time_rec)
	*time_rec = outTime;


    *output_cred_handle = (gss_cred_id_t) creds;
    return (GSS_S_COMPLETE);
}

static          OM_uint32
val_add_cred_args(OM_uint32 *minor_status,
		  gss_cred_id_t input_cred_handle,
		  gss_name_t desired_name,
		  gss_OID desired_mech,
		  gss_cred_usage_t cred_usage,
		  OM_uint32 initiator_time_req,
		  OM_uint32 acceptor_time_req,
		  gss_cred_id_t *output_cred_handle,
		  gss_OID_set *actual_mechs,
		  OM_uint32 *initiator_time_rec,
		  OM_uint32 *acceptor_time_rec)
{

    /*
     * Initialize outputs.
     */

    if (minor_status != NULL)
	*minor_status = 0;

    if (output_cred_handle != NULL)
	*output_cred_handle = GSS_C_NO_CREDENTIAL;

    if (actual_mechs != NULL)
	*actual_mechs = GSS_C_NO_OID_SET;

    if (acceptor_time_rec != NULL)
	*acceptor_time_rec = 0;

    if (initiator_time_rec != NULL)
	*initiator_time_rec = 0;

    /*
     * Validate arguments.
     */

    if (minor_status == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (input_cred_handle == GSS_C_NO_CREDENTIAL &&
	output_cred_handle == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_NO_CRED);

    if (cred_usage != GSS_C_ACCEPT
	&& cred_usage != GSS_C_INITIATE && cred_usage != GSS_C_BOTH) {
	if (minor_status) {
	    *minor_status = EINVAL;
	    map_errcode(minor_status);
	}
	return GSS_S_FAILURE;
    }

    return (GSS_S_COMPLETE);
}


/*
 * V2
 */
OM_uint32
gss_add_cred(OM_uint32 *minor_status,
	     gss_cred_id_t input_cred_handle,
	     gss_name_t desired_name,
	     gss_OID desired_mech,
	     gss_cred_usage_t cred_usage,
	     OM_uint32 initiator_time_req,
	     OM_uint32 acceptor_time_req,
	     gss_cred_id_t *output_cred_handle,
	     gss_OID_set *actual_mechs,
	     OM_uint32 *initiator_time_rec, OM_uint32 *acceptor_time_rec)
{
    OM_uint32       status,
                    temp_minor_status;
    OM_uint32       time_req,
                    time_rec;
    gss_union_name_t union_name;
    gss_union_cred_t new_union_cred,
                    union_cred;
    gss_name_t      internal_name = GSS_C_NO_NAME;
    gss_name_t      allocated_name = GSS_C_NO_NAME;
    gss_mechanism   mech;
    gss_cred_id_t   cred = NULL;
    gss_OID         new_mechs_array = NULL;
    gss_cred_id_t  *new_cred_array = NULL;

    status = val_add_cred_args(minor_status,
			       input_cred_handle,
			       desired_name,
			       desired_mech,
			       cred_usage,
			       initiator_time_req,
			       acceptor_time_req,
			       output_cred_handle,
			       actual_mechs,
			       initiator_time_rec, acceptor_time_rec);
    if (status != GSS_S_COMPLETE)
	return (status);

    mech = __gss_get_mechanism(desired_mech);
    if (!mech)
	return GSS_S_BAD_MECH;
    else if (!mech->gss_acquire_cred)
	return (GSS_S_UNAVAILABLE);

    if (input_cred_handle == GSS_C_NO_CREDENTIAL) {
	union_cred = malloc(sizeof(gss_union_cred_desc));
	if (union_cred == NULL)
	    return (GSS_S_FAILURE);

	(void) memset(union_cred, 0, sizeof(gss_union_cred_desc));

	/*
	 * for default credentials we will use GSS_C_NO_NAME
	 */
	internal_name = GSS_C_NO_NAME;
    } else {
	union_cred = (gss_union_cred_t) input_cred_handle;
	if (__gss_get_mechanism_cred(union_cred, desired_mech) !=
	    GSS_C_NO_CREDENTIAL)
	    return (GSS_S_DUPLICATE_ELEMENT);

	/*
	 * may need to create a mechanism specific name
	 */
	if (desired_name) {
	    union_name = (gss_union_name_t) desired_name;
	    if (union_name->mech_type &&
		g_OID_equal(union_name->mech_type, &mech->mech_type))
		internal_name = union_name->mech_name;
	    else {
		if (__gss_import_internal_name(minor_status,
					       &mech->mech_type,
					       union_name,
					       &allocated_name) !=
		    GSS_S_COMPLETE)
		    return (GSS_S_BAD_NAME);
		internal_name = allocated_name;
	    }
	}
    }


    if (cred_usage == GSS_C_ACCEPT)
	time_req = acceptor_time_req;
    else if (cred_usage == GSS_C_INITIATE)
	time_req = initiator_time_req;
    else if (cred_usage == GSS_C_BOTH)
	time_req = (acceptor_time_req > initiator_time_req) ?
	    acceptor_time_req : initiator_time_req;
    else
	time_req = 0;

    status = mech->gss_acquire_cred(minor_status,
				    internal_name, time_req,
				    GSS_C_NULL_OID_SET, cred_usage,
				    &cred, NULL, &time_rec);

    if (status != GSS_S_COMPLETE) {
	map_error(minor_status, mech);
	goto errout;
    }

    /*
     * may need to set credential auxinfo strucutre
     */
    if (union_cred->auxinfo.creation_time == 0) {
	union_cred->auxinfo.creation_time = time(NULL);
	union_cred->auxinfo.time_rec = time_rec;
	union_cred->auxinfo.cred_usage = cred_usage;

	/*
	 * we must set the name; if name is not supplied
	 * we must do inquire cred to get it
	 */
	if (internal_name == NULL) {
	    if (mech->gss_inquire_cred == NULL ||
		((status = mech->gss_inquire_cred(&temp_minor_status, cred,
						  &allocated_name, NULL,
						  NULL,
						  NULL)) !=
		 GSS_S_COMPLETE))
		goto errout;
	    internal_name = allocated_name;
	}

	if (internal_name != GSS_C_NO_NAME) {
	    status =
		mech->gss_display_name(&temp_minor_status, internal_name,
				       &union_cred->auxinfo.name,
				       &union_cred->auxinfo.name_type);

	    if (status != GSS_S_COMPLETE)
		goto errout;
	}
    }

    /*
     * now add the new credential elements
     */
    new_mechs_array = (gss_OID)
	malloc(sizeof(gss_OID_desc) * (union_cred->count + 1));

    new_cred_array = (gss_cred_id_t *)
	malloc(sizeof(gss_cred_id_t) * (union_cred->count + 1));

    if (!new_mechs_array || !new_cred_array) {
	status = GSS_S_FAILURE;
	goto errout;
    }

    if (acceptor_time_rec)
	if (cred_usage == GSS_C_ACCEPT || cred_usage == GSS_C_BOTH)
	    *acceptor_time_rec = time_rec;
    if (initiator_time_rec)
	if (cred_usage == GSS_C_INITIATE || cred_usage == GSS_C_BOTH)
	    *initiator_time_rec = time_rec;

    /*
     * OK, expand the mechanism array and the credential array
     */
    (void) memcpy(new_mechs_array, union_cred->mechs_array,
		  sizeof(gss_OID_desc) * union_cred->count);
    (void) memcpy(new_cred_array, union_cred->cred_array,
		  sizeof(gss_cred_id_t) * union_cred->count);

    new_cred_array[union_cred->count] = cred;
    if ((new_mechs_array[union_cred->count].elements =
	 malloc(mech->mech_type.length)) == NULL)
	goto errout;

    g_OID_copy(&new_mechs_array[union_cred->count], &mech->mech_type);

    if (actual_mechs != NULL) {
	gss_OID_set_desc oids;

	oids.count = union_cred->count + 1;
	oids.elements = new_mechs_array;

	status =
	    generic_gss_copy_oid_set(minor_status, &oids, actual_mechs);
	if (GSS_ERROR(status)) {
	    free(new_mechs_array[union_cred->count].elements);
	    goto errout;
	}
    }

    if (output_cred_handle == NULL) {
	free(union_cred->mechs_array);
	free(union_cred->cred_array);
	new_union_cred = union_cred;
    } else {
	new_union_cred = malloc(sizeof(gss_union_cred_desc));
	if (new_union_cred == NULL) {
	    free(new_mechs_array[union_cred->count].elements);
	    goto errout;
	}
	*new_union_cred = *union_cred;
	*output_cred_handle = (gss_cred_id_t) new_union_cred;
    }

    new_union_cred->mechs_array = new_mechs_array;
    new_union_cred->cred_array = new_cred_array;
    new_union_cred->count++;

    /*
     * We're done with the internal name. Free it if we allocated it.
     */

    if (allocated_name)
	(void) __gss_release_internal_name(&temp_minor_status,
					   &mech->mech_type,
					   &allocated_name);

    return (GSS_S_COMPLETE);

  errout:
    if (new_mechs_array)
	free(new_mechs_array);
    if (new_cred_array)
	free(new_cred_array);

    if (cred != NULL && mech->gss_release_cred)
	mech->gss_release_cred(&temp_minor_status, &cred);

    if (allocated_name)
	(void) __gss_release_internal_name(&temp_minor_status,
					   &mech->mech_type,
					   &allocated_name);

    if (input_cred_handle == GSS_C_NO_CREDENTIAL && union_cred) {
	if (union_cred->auxinfo.name.value)
	    free(union_cred->auxinfo.name.value);
	free(union_cred);
    }

    return (status);
}
