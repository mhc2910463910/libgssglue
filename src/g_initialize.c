/*
 * #ident "@(#)g_initialize.c 1.2 96/02/06 SMI"
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
 * This function will initialize the gssapi mechglue library
 */

#include "mglueP.h"
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(_MSDOS) || defined(_WIN32)

/*
 * syslog.h
 */
#define LOG_WARNING  5
/*
 * void syslog(int priority, const char * fmt, ...);
 */
#define syslog(priority, ...)  fprintf(stderr, __VA_ARGS__)

#else

#include <unistd.h>		/* getuid, geteuid */
#include <sys/types.h>		/* ditto */
#include <syslog.h>

#endif

#ifdef USE_SOLARIS_SHARED_LIBRARIES
#include <dlfcn.h>

#if defined(_MSDOS) || defined(_WIN32)
#define MECH_CONF "C:\\Program Files\\libgssglue\\gssapi_mech.conf"
#else
#define MECH_CONF "/etc/gssapi_mech.conf"
#endif

#define MECH_SYM "gss_mech_initialize"

static void     solaris_initialize(void);
#endif				/* USE_SOLARIS_SHARED_LIBRARIES */

#if defined(__linux__) || defined(__GLIBC__)
#define USE_LINUX_SHARED_LIBRARIES
#endif

#ifdef USE_LINUX_SHARED_LIBRARIES
#include <dlfcn.h>
#define MECH_CONF "/etc/gssapi_mech.conf"
#define MECH_SYM "gss_mech_initialize"
static void     linux_initialize(void);
#endif				/* USE_LINUX_SHARED_LIBRARIES */

extern gss_mechanism krb5_gss_initialize();

static int      _gss_initialized = 0;

static struct gss_config null_mech = {
    {0, NULL}
};

gss_mechanism  *__gss_mechs_array = NULL;

/*
 * This function will add a new mechanism to the mechs_array
 */

static          OM_uint32
add_mechanism(gss_mechanism mech, int replace)
{
    gss_mechanism  *temp_array;
    gss_OID_set     mech_names;
    OM_uint32       minor_status,
                    major_status;
    unsigned int    i;

    if (mech == NULL)
	return GSS_S_COMPLETE;

    /*
     * initialize the mechs_array if it hasn't already been initialized
     */
    if (__gss_mechs_array == NULL) {
	__gss_mechs_array =
	    (gss_mechanism *) malloc(sizeof(gss_mechanism));

	if (__gss_mechs_array == NULL)
	    return ENOMEM;

	__gss_mechs_array[0] = &null_mech;
    }

    /*
     * Find the length of __gss_mechs_array, and look for an existing
     * entry for this OID
     */
    for (i = 0; __gss_mechs_array[i]->mech_type.length != 0; i++) {
	if (!g_OID_equal(&__gss_mechs_array[i]->mech_type,
			 &mech->mech_type))
	    continue;

	/*
	 * We found a match.  Replace it?
	 */
	if (!replace)
	    return GSS_S_FAILURE;

	__gss_mechs_array[i] = mech;
	return GSS_S_COMPLETE;
    }

    /*
     * we didn't find it -- add it to the end of the __gss_mechs_array
     */
    temp_array = (gss_mechanism *) realloc(__gss_mechs_array,
					   (i +
					    2) * sizeof(gss_mechanism));

    if (temp_array == NULL)
	return ENOMEM;

    temp_array[i++] = mech;
    temp_array[i] = &null_mech;

    __gss_mechs_array = temp_array;

    /*
     * OK, now let's register all of the name types this mechanism
     * knows how to deal with.
     */
    major_status =
	gss_inquire_names_for_mech(&minor_status, &mech->mech_type,
				   &mech_names);
    if (major_status != GSS_S_COMPLETE)
	return (GSS_S_COMPLETE);
    for (i = 0; i < mech_names->count; i++) {
	gss_add_mech_name_type(&minor_status, &mech_names->elements[i],
			       &mech->mech_type);
    }
#if 0
    (void) gss_release_oid_set(&minor_status, &mech_names);
#else
    /*
     * We've cheated and just copied references to the oids.
     * Don't release them!
     */
#endif

    return GSS_S_COMPLETE;
}

int
gss_initialize(void)
{
    /*
     * Make sure we've not run already
     */
    if (_gss_initialized)
	return 0;
    _gss_initialized = 1;

#ifdef USE_SOLARIS_SHARED_LIBRARIES
    solaris_initialize();

#elif defined(USE_LINUX_SHARED_LIBRARIES)
    linux_initialize();

#else
    {
	gss_mechanism   mech;

	/*
	 * Use hard-coded in mechanisms...  I need to know what mechanisms
	 * are supported...  As more mechanisms become supported, they
	 * should be added here, unless shared libraries are used.
	 */

	/*
	 * Initialize the krb5 mechanism
	 */
	mech = (gss_mechanism) krb5_gss_initialize();
	if (mech)
	    add_mechanism(mech, 1);
    }

#endif				/* USE_SOLARIS_SHARED_LIBRARIES */

    if (__gss_mechs_array == NULL) {
	syslog(LOG_WARNING, "warning: no gssapi mechanisms loaded!\n");
    }

    return 0;
}

#ifdef USE_SOLARIS_SHARED_LIBRARIES
/*
 * read the configuration file to find out what mechanisms to
 * load, load them, and then load the mechanism defitions in
 * and add the mechanisms
 */
static void
solaris_initialize()
{
    char            buffer[BUFSIZ],
                   *filename,
                   *symname,
                   *endp;
    FILE           *conffile;
    void           *dl;
    gss_mechanism(*sym) (void), mech;

    if ((getuid() != geteuid()) ||
	((filename = getenv("GSSAPI_MECH_CONF")) == NULL))
	filename = MECH_CONF;

    if ((conffile = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "warning: unable to open %s:"
		" errno %d (%s)\n", filename, errno, strerror(errno));
	return;
    }

    while (fgets(buffer, BUFSIZ, conffile) != NULL) {
	/*
	 * ignore lines beginning with #
	 */
	if (*buffer == '#')
	    continue;

	/*
	 * find the first white-space character after the filename
	 */
	for (symname = buffer; *symname && !isspace(*symname); symname++);

	/*
	 * Now find the first non-white-space character
	 */
	if (*symname) {
	    *symname = '\0';
	    symname++;
	    while (*symname && isspace(*symname))
		symname++;
	}

	if (!*symname)
	    symname = MECH_SYM;
	else {
	    /*
	     * Find the end of the symname and make sure it is
	     * NULL-terminated
	     */
	    for (endp = symname; *endp && !isspace(*endp); endp++);
	    if (*endp)
		*endp = '\0';
	}

	if ((dl = dlopen(buffer, RTLD_NOW)) == NULL) {
	    /*
	     * for debugging only
	     */
	    fprintf(stderr, "can't open %s: %s\n", buffer, dlerror());
	    continue;
	}

	if ((sym = (gss_mechanism(*)(void)) dlsym(dl, symname)) == NULL) {
	    dlclose(dl);
	    continue;
	}

	/*
	 * Call the symbol to get the mechanism table
	 */
	mech = sym();

	/*
	 * And add the mechanism (or close the shared library)
	 */
	if (mech)
	    add_mechanism(mech, 1);
	else
	    dlclose(dl);

    }				/* while */
    fclose(conffile);

    return;
}
#endif				/* USE_SOLARIS_SHARED_LIBRARIES */

#ifdef USE_LINUX_SHARED_LIBRARIES
extern gss_mechanism internal_krb5_gss_initialize(void *dl);

/*
 * read the configuration file to find out what mechanisms to
 * load, load them, and then load the mechanism defitions in
 * and add the mechanisms
 */
static void
linux_initialize(void)
{
    char            buffer[BUFSIZ],
                   *filename,
                   *symname,
                   *endp,
                   *err_string;
    FILE           *conffile;
    void           *dl;
    gss_mechanism(*sym) (void), mech;

    if ((getuid() != geteuid()) ||
	((filename = getenv("GSSAPI_MECH_CONF")) == NULL))
	filename = MECH_CONF;

    if ((conffile = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "warning: unable to open %s:"
		" errno %d (%s)\n", filename, errno, strerror(errno));
	return;
    }

    while (fgets(buffer, BUFSIZ, conffile) != NULL) {
	/*
	 * ignore lines beginning with #
	 */
	if (*buffer == '#')
	    continue;

	/*
	 * find the first white-space character after the filename
	 */
	for (symname = buffer; *symname && !isspace(*symname); symname++);

	/*
	 * Now find the first non-white-space character
	 */
	if (*symname) {
	    *symname = '\0';
	    symname++;
	    while (*symname && isspace(*symname))
		symname++;
	}

	if (!*symname)
	    symname = MECH_SYM;
	else {
	    /*
	     * Find the end of the symname and make sure it is
	     * NULL-terminated
	     */
	    for (endp = symname; *endp && !isspace(*endp); endp++);
	    if (*endp)
		*endp = '\0';
	}

	if ((dl = dlopen(buffer, RTLD_NOW | RTLD_LOCAL)) == NULL) {
	    /*
	     * for debugging only
	     */
	    fprintf(stderr, "can't open %s: %s\n", buffer, dlerror());
	    continue;
	}

	/*
	 * Special case for dealing with Kerberos 5 mechanism
	 */
	if (strcmp(symname, "mechglue_internal_krb5_init") == 0) {
	    mech = internal_krb5_gss_initialize(dl);
	} else {
	    if ((sym =
		 (gss_mechanism(*)(void)) dlsym(dl, symname)) == NULL) {
		if ((err_string = dlerror()) != NULL) {
		    fprintf(stderr,
			    "%s: searching for symbol '%s' in '%s'\n",
			    err_string, symname, buffer);
		    dlclose(dl);
		}
		continue;
	    }

	    /*
	     * Call the symbol to get the mechanism table
	     */
	    mech = sym();
	}

	/*
	 * And add the mechanism (or close the shared library)
	 */
	if (mech) {
	    add_mechanism(mech, 1);
	} else {
	    syslog(LOG_WARNING,
		   "Failed to initialize mechanism for library '%s'\n",
		   buffer);
	    dlclose(dl);
	}

    }				/* while */
    fclose(conffile);

    return;
}
#endif				/* USE_LINUX_SHARED_LIBRARIES */
