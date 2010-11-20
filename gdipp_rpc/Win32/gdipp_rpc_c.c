

/* this ALWAYS GENERATED file contains the RPC client stubs */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#include <string.h>

#include "gdipp_rpc.h"

#define TYPE_FORMAT_STRING_SIZE   83                                
#define PROC_FORMAT_STRING_SIZE   349                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _gdipp_rpc_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } gdipp_rpc_MIDL_TYPE_FORMAT_STRING;

typedef struct _gdipp_rpc_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } gdipp_rpc_MIDL_PROC_FORMAT_STRING;

typedef struct _gdipp_rpc_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } gdipp_rpc_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const gdipp_rpc_MIDL_TYPE_FORMAT_STRING gdipp_rpc__MIDL_TypeFormatString;
extern const gdipp_rpc_MIDL_PROC_FORMAT_STRING gdipp_rpc__MIDL_ProcFormatString;
extern const gdipp_rpc_MIDL_EXPR_FORMAT_STRING gdipp_rpc__MIDL_ExprFormatString;

#define GENERIC_BINDING_TABLE_SIZE   0            


/* Standard interface: gdipp_rpc, ver. 0.9,
   GUID={0xDA2AAF70,0x7FD7,0x4D0D,{0x97,0x9F,0x20,0x56,0x46,0x3C,0x31,0x0A}} */



static const RPC_CLIENT_INTERFACE gdipp_rpc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xDA2AAF70,0x7FD7,0x4D0D,{0x97,0x9F,0x20,0x56,0x46,0x3C,0x31,0x0A}},{0,9}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE gdipp_rpc_v0_9_c_ifspec = (RPC_IF_HANDLE)& gdipp_rpc___RpcClientInterface;

extern const MIDL_STUB_DESC gdipp_rpc_StubDesc;

static RPC_BINDING_HANDLE gdipp_rpc__MIDL_AutoBindHandle;


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_register_font( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [size_is][in] */ const byte *attr_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *font_id)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [out] */ unsigned long *data_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[52],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *data_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[110],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [out] */ unsigned long *metrics_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[180],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size,
    /* [out] */ unsigned long *metrics_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[226],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [in] */ unsigned long font_id,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi,
    /* [out] */ unsigned long *converted)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[284],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( error_status_t  )_RetVal.Simple;
    
}


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const gdipp_rpc_MIDL_PROC_FORMAT_STRING gdipp_rpc__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure gdipp_rpc_register_font */

			0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x8 ),	/* 8 */
/* 16 */	NdrFcShort( 0x24 ),	/* 36 */
/* 18 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x4,		/* 4 */
/* 20 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x1 ),	/* 1 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 28 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter attr_buf */

/* 34 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 40 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 46 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 48 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 50 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_data_size */


	/* Return value */

/* 52 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 54 */	NdrFcLong( 0x0 ),	/* 0 */
/* 58 */	NdrFcShort( 0x1 ),	/* 1 */
/* 60 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 62 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 64 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 66 */	NdrFcShort( 0x18 ),	/* 24 */
/* 68 */	NdrFcShort( 0x24 ),	/* 36 */
/* 70 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x5,		/* 5 */
/* 72 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 74 */	NdrFcShort( 0x0 ),	/* 0 */
/* 76 */	NdrFcShort( 0x0 ),	/* 0 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 80 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 82 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 84 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 86 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 88 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 90 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter table */

/* 92 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 94 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 96 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter offset */

/* 98 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 100 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 102 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter data_size */

/* 104 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 106 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 108 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_data */


	/* Return value */

/* 110 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 112 */	NdrFcLong( 0x0 ),	/* 0 */
/* 116 */	NdrFcShort( 0x2 ),	/* 2 */
/* 118 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 120 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 122 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 124 */	NdrFcShort( 0x20 ),	/* 32 */
/* 126 */	NdrFcShort( 0x24 ),	/* 36 */
/* 128 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x7,		/* 7 */
/* 130 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 132 */	NdrFcShort( 0x1 ),	/* 1 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 138 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 140 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 144 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 146 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 148 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter table */

/* 150 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 152 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 154 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter offset */

/* 156 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 158 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 160 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter data_buf */

/* 162 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 164 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 166 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 168 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 170 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 172 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter data_size */

/* 174 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 176 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 178 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_size */


	/* Return value */

/* 180 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 182 */	NdrFcLong( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x3 ),	/* 3 */
/* 188 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 190 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 192 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 194 */	NdrFcShort( 0x8 ),	/* 8 */
/* 196 */	NdrFcShort( 0x24 ),	/* 36 */
/* 198 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x3,		/* 3 */
/* 200 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 202 */	NdrFcShort( 0x0 ),	/* 0 */
/* 204 */	NdrFcShort( 0x0 ),	/* 0 */
/* 206 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 208 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 210 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 212 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 214 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 216 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 218 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter metrics_size */

/* 220 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 222 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 224 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_data */


	/* Return value */

/* 226 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 228 */	NdrFcLong( 0x0 ),	/* 0 */
/* 232 */	NdrFcShort( 0x4 ),	/* 4 */
/* 234 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 236 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 238 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 240 */	NdrFcShort( 0x10 ),	/* 16 */
/* 242 */	NdrFcShort( 0x24 ),	/* 36 */
/* 244 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x5,		/* 5 */
/* 246 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 248 */	NdrFcShort( 0x1 ),	/* 1 */
/* 250 */	NdrFcShort( 0x0 ),	/* 0 */
/* 252 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 254 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 256 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 258 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 260 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 262 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 264 */	NdrFcShort( 0x2a ),	/* Type Offset=42 */

	/* Parameter metrics_buf */

/* 266 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 268 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 270 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 272 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 274 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 276 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter metrics_size */

/* 278 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 280 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 282 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_glyph_indices */


	/* Return value */

/* 284 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 286 */	NdrFcLong( 0x0 ),	/* 0 */
/* 290 */	NdrFcShort( 0x5 ),	/* 5 */
/* 292 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 294 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 296 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 298 */	NdrFcShort( 0x10 ),	/* 16 */
/* 300 */	NdrFcShort( 0x24 ),	/* 36 */
/* 302 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x6,		/* 6 */
/* 304 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 306 */	NdrFcShort( 0x1 ),	/* 1 */
/* 308 */	NdrFcShort( 0x1 ),	/* 1 */
/* 310 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 312 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 314 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 316 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter font_id */

/* 318 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 320 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 322 */	NdrFcShort( 0x3a ),	/* Type Offset=58 */

	/* Parameter str */

/* 324 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 326 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 328 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter count */

/* 330 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 332 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 334 */	NdrFcShort( 0x46 ),	/* Type Offset=70 */

	/* Parameter gi */

/* 336 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 338 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 340 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter converted */

/* 342 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 344 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 346 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

			0x0
        }
    };

static const gdipp_rpc_MIDL_TYPE_FORMAT_STRING gdipp_rpc__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x0,	/* FC_RP */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/*  8 */	NdrFcShort( 0x1 ),	/* 1 */
/* 10 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 14 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 16 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 18 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 20 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 22 */	
			0x11, 0x0,	/* FC_RP */
/* 24 */	NdrFcShort( 0x2 ),	/* Offset= 2 (26) */
/* 26 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 28 */	NdrFcShort( 0x1 ),	/* 1 */
/* 30 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 32 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 34 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 36 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 38 */	
			0x11, 0x0,	/* FC_RP */
/* 40 */	NdrFcShort( 0x2 ),	/* Offset= 2 (42) */
/* 42 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 44 */	NdrFcShort( 0x1 ),	/* 1 */
/* 46 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 48 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 50 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 52 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 54 */	
			0x11, 0x0,	/* FC_RP */
/* 56 */	NdrFcShort( 0x2 ),	/* Offset= 2 (58) */
/* 58 */	
			0x25,		/* FC_C_WSTRING */
			0x44,		/* FC_STRING_SIZED */
/* 60 */	0x28,		/* Corr desc:  parameter, FC_LONG */
			0x56,		/* FC_MULT_2 */
/* 62 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 64 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 66 */	
			0x11, 0x0,	/* FC_RP */
/* 68 */	NdrFcShort( 0x2 ),	/* Offset= 2 (70) */
/* 70 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 72 */	NdrFcShort( 0x2 ),	/* 2 */
/* 74 */	0x28,		/* Corr desc:  parameter, FC_LONG */
			0x56,		/* FC_MULT_2 */
/* 76 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 78 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 80 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */

			0x0
        }
    };

static const unsigned short gdipp_rpc_FormatStringOffsetTable[] =
    {
    0,
    52,
    110,
    180,
    226,
    284
    };


static const COMM_FAULT_OFFSETS gdipp_rpc_CommFaultOffsets[] = 
{
	{ -1, -1 },	/* x86 Offsets for gdipp_rpc_register_font */
	{ -1, -1 },	/* x86 Offsets for gdipp_rpc_get_font_data_size */
	{ -1, -1 },	/* x86 Offsets for gdipp_rpc_get_font_data */
	{ -1, -1 },	/* x86 Offsets for gdipp_rpc_get_font_metrics_size */
	{ -1, -1 },	/* x86 Offsets for gdipp_rpc_get_font_metrics_data */
	{ -1, -1 } 	/* x86 Offsets for gdipp_rpc_get_glyph_indices */
};


static const MIDL_STUB_DESC gdipp_rpc_StubDesc = 
    {
    (void *)& gdipp_rpc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &gdipp_rpc__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    gdipp_rpc__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    gdipp_rpc_CommFaultOffsets,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

