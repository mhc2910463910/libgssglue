lib_LTLIBRARIES = libgssglue.la
libgssglue_la_LDFLAGS = -version-info 1:0:0 -no-undefined
if HAVE_LD_VERSION_SCRIPT
libgssglue_la_LDFLAGS += -Wl,--version-script,$(srcdir)/symbol_versions
endif
libgssglue_la_SOURCES = \
	g_accept_sec_context.c g_acquire_cred.c g_compare_name.c \
	g_context_time.c g_delete_sec_context.c g_dsp_name.c g_dsp_status.c \
	g_dup_name.c gen_oids.c g_exp_name.c g_exp_sec_context.c g_glue.c \
	g_imp_name.c g_imp_sec_context.c g_indicate_mechs.c g_initialize.c \
	g_init_sec_context.c g_inq_context.c g_inq_cred.c g_inq_names.c \
	g_mechname.c g_mit_krb5_mech.c g_oid_ops.c g_process_context.c \
	g_rel_buffer.c g_rel_cred.c g_rel_name.c g_rel_oid_set.c g_seal.c \
	g_sign.c gssd_pname_to_uid.c g_unseal.c g_verify.c oid_ops.c \
	g_set_allowable_enctypes.c g_lucid_context.c g_canon_name.c \
	g_wrap.c g_unwrap.c g_get_mic.c g_verify_mic.c g_ccache_name.c \
	mglueP.h mechglue.h
libgssglue_la_LIBADD = -ldl

EXTRA_DIST = $(srcdir)/gssglue/gssapi/gssapi.h.in $(srcdir)/symbol_versions
DISTCLEANFILES = $(top_builddir)/src/gssglue/gssapi/gssapi.h
BUILT_SOURCES = $(top_builddir)/src/gssglue/gssapi/gssapi.h
nobase_include_HEADERS = gssglue/gssapi/gssapi.h

AM_CFLAGS = -Wall
