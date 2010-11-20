

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Nov 19 21:05:21 2010
 */
/* Compiler settings for ..\gdipp_rpc.idl, ..\gdipp_rpc.acf:
    Oicf, W3, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_register_font( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [size_is][in] */ const byte *attr_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *font_id);

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [out] */ unsigned long *data_size);

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *data_size);

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [out] */ unsigned long *metrics_size);

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *metrics_size);

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi,
    /* [out] */ unsigned long *converted);



extern RPC_IF_HANDLE gdipp_rpc_v0_9_c_ifspec;
extern RPC_IF_HANDLE gdipp_rpc_v0_9_s_ifspec;
#endif /* __gdipp_rpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


