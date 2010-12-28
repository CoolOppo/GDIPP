

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Dec 28 02:29:26 2010
 */
/* Compiler settings for gdipp_rpc.idl, gdipp_rpc.acf:
    Oicf, W3, Zp8, env=Win64 (32b run)
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __gdipp_rpc_h__
#define __gdipp_rpc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __gdipp_rpc_INTERFACE_DEFINED__
#define __gdipp_rpc_INTERFACE_DEFINED__

/* interface gdipp_rpc */
/* [explicit_handle][version][uuid] */ 

typedef /* [context_handle] */ void *GDIPP_RPC_SESSION_HANDLE;

typedef /* [context_handle] */ void *GDIPP_RPC_GLYPH_RUN_HANDLE;

GDIPP_RPC_SESSION_HANDLE gdipp_rpc_begin_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [size_is][in] */ const byte *logfont_buf,
    /* [in] */ unsigned long logfont_size);

unsigned long gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset);

unsigned long gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size);

unsigned long gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session);

unsigned long gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size);

unsigned long gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi);

GDIPP_RPC_GLYPH_RUN_HANDLE gdipp_rpc_make_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [string][in] */ wchar_t *str,
    /* [in] */ unsigned int count,
    /* [in] */ boolean is_glyph_index);

unsigned long gdipp_rpc_get_glyph_run_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run);

boolean gdipp_rpc_get_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run,
    /* [size_is][out] */ byte *glyph_run_buf,
    /* [in] */ unsigned long glyph_run_size);

boolean gdipp_rpc_release_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE *h_glyph_run);

boolean gdipp_rpc_end_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_SESSION_HANDLE *h_session);



extern RPC_IF_HANDLE gdipp_rpc_v0_9_c_ifspec;
extern RPC_IF_HANDLE gdipp_rpc_v0_9_s_ifspec;
#endif /* __gdipp_rpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

void __RPC_USER GDIPP_RPC_SESSION_HANDLE_rundown( GDIPP_RPC_SESSION_HANDLE );
void __RPC_USER GDIPP_RPC_GLYPH_RUN_HANDLE_rundown( GDIPP_RPC_GLYPH_RUN_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


