/*
 * Copyright (C) 2022 Simon Josefsson
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

#include <gssapi/gssapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
    int             errs = 0;
    gss_uint32      maj_stat,
                    min_stat;
    gss_buffer_desc bufdesc,
                    bufdesc2;
    gss_name_t      service;
    gss_OID_set     oids;
    int             n;

    /*
     * OID set tests
     */
    oids = GSS_C_NO_OID_SET;
    maj_stat = gss_create_empty_oid_set(&min_stat, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_create_empty_oid_set\n");
    else
	errs++, printf("FAIL: gss_create_empty_oid_set (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Test empty set
     */
    maj_stat = gss_test_oid_set_member(&min_stat, GSS_C_NT_USER_NAME,
				       oids, &n);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_test_oid_set_member\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member (%u,%u)\n",
		       maj_stat, min_stat);

    if (n == 0)
	printf("PASS: gss_test_oid_set_member n==0\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member n=%d!=0", n);

    /*
     * Add an OID
     */
    maj_stat =
	gss_add_oid_set_member(&min_stat, GSS_C_NT_USER_NAME, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_add_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_add_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Test set for added OID
     */
    maj_stat = gss_test_oid_set_member(&min_stat, GSS_C_NT_USER_NAME,
				       oids, &n);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("    OID present in set with the OID added to it => %d\n", n);

    if (n)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Test set for another OID
     */
    maj_stat = gss_test_oid_set_member(&min_stat, GSS_C_NT_ANONYMOUS,
				       oids, &n);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("    Another OID present in set without the OID => %d\n", n);

    if (!n)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Add another OID
     */
    maj_stat =
	gss_add_oid_set_member(&min_stat, GSS_C_NT_ANONYMOUS, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_add_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_add_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Test set for added OID
     */
    maj_stat = gss_test_oid_set_member(&min_stat, GSS_C_NT_ANONYMOUS,
				       oids, &n);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("    Another OID present in set with it added => %d\n", n);

    if (n)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Test set for first OID
     */
    maj_stat = gss_test_oid_set_member(&min_stat, GSS_C_NT_USER_NAME,
				       oids, &n);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("    First OID present in set => %d\n", n);

    if (n)
	printf("PASS: gss_test_oid_set_member() OK\n");
    else
	errs++, printf("FAIL: gss_test_oid_set_member() failed (%u,%u)\n",
		       maj_stat, min_stat);

    maj_stat = gss_release_oid_set(&min_stat, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_release_oid_set() OK\n");
    else
	errs++, printf("FAIL: gss_release_oid_set() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Check mechs
     */
    oids = GSS_C_NO_OID_SET;
    maj_stat = gss_indicate_mechs(&min_stat, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_indicate_mechs() OK\n");
    else
	errs++, printf("FAIL: gss_indicate_mechs() failed (%u,%u)\n",
		       maj_stat, min_stat);

    maj_stat = gss_release_oid_set(&min_stat, &oids);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_release_oid_set() OK\n");
    else
	errs++, printf("FAIL: gss_release_oid_set() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Check name
     */
    service = NULL;
    bufdesc.value = (char *) "imap@server.example.org@FOO";
    bufdesc.length = strlen(bufdesc.value);

    maj_stat =
	gss_import_name(&min_stat, &bufdesc, GSS_C_NT_HOSTBASED_SERVICE,
			&service);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_import_name() OK\n");
    else
	errs++, printf("FAIL: gss_import_name() failed (%u,%u)\n",
		       maj_stat, min_stat);

    maj_stat = gss_display_name(&min_stat, service, &bufdesc2, NULL);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_display_name() OK\n");
    else
	errs++, printf("FAIL: gss_display_name() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("    display_name() => %d: %.*s\n", (int) bufdesc2.length,
	   (int) bufdesc2.length, (char *) bufdesc2.value);

    maj_stat = gss_release_buffer(&min_stat, &bufdesc2);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_release_buffer() OK\n");
    else
	errs++, printf("FAIL: gss_release_buffer() failed (%u,%u)\n",
		       maj_stat, min_stat);

    /*
     * Release service allocated earlier.
     */
    maj_stat = gss_release_name(&min_stat, &service);
    if (maj_stat == GSS_S_COMPLETE)
	printf("PASS: gss_release_name() OK\n");
    else
	errs++, printf("FAIL: gss_release_name() failed (%u,%u)\n",
		       maj_stat, min_stat);

    printf("Basic self tests done with %d errors\n", errs);

    return errs == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
