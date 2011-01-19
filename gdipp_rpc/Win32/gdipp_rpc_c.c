

/* this ALWAYS GENERATED file contains the RPC client stubs */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Jan 10 01:24:26 2011
 */
/* Compiler settings for gdipp_rpc.idl, gdipp_rpc.acf:
    Oicf, W3, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

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

#define TYPE_FORMAT_STRING_SIZE   115                               
#define PROC_FORMAT_STRING_SIZE   549                               
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


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
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


GDIPP_RPC_SESSION_HANDLE gdipp_rpc_begin_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [size_is][in] */ const byte *logfont_buf,
    /* [in] */ unsigned long logfont_size,
    /* [in] */ int render_mode)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( GDIPP_RPC_SESSION_HANDLE  )_RetVal.Pointer;
    
}


unsigned long gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[52],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


unsigned long gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[104],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


unsigned long gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[168],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


unsigned long gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[208],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


unsigned long gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[260],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


GDIPP_RPC_GLYPH_RUN_HANDLE gdipp_rpc_make_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [string][in] */ wchar_t *str,
    /* [in] */ unsigned int count,
    /* [in] */ boolean is_glyph_index)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[318],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( GDIPP_RPC_GLYPH_RUN_HANDLE  )_RetVal.Pointer;
    
}


unsigned long gdipp_rpc_get_glyph_run_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[376],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( unsigned long  )_RetVal.Simple;
    
}


boolean gdipp_rpc_get_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run,
    /* [size_is][out] */ byte *glyph_run_buf,
    /* [in] */ unsigned long glyph_run_size)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[416],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( boolean  )_RetVal.Simple;
    
}


boolean gdipp_rpc_release_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE *h_glyph_run)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[468],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( boolean  )_RetVal.Simple;
    
}


boolean gdipp_rpc_end_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_SESSION_HANDLE *h_session)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&gdipp_rpc_StubDesc,
                  (PFORMAT_STRING) &gdipp_rpc__MIDL_ProcFormatString.Format[508],
                  ( unsigned char * )&h_gdipp_rpc);
    return ( boolean  )_RetVal.Simple;
    
}


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif
#if !(TARGET_IS_NT51_OR_LATER)
#error You need a Windows XP or later to run this stub because it uses these features:
#error   #error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const gdipp_rpc_MIDL_PROC_FORMAT_STRING gdipp_rpc__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure gdipp_rpc_begin_session */

			0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x10 ),	/* 16 */
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

	/* Parameter logfont_buf */

/* 34 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter logfont_size */

/* 40 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter render_mode */

/* 46 */	NdrFcShort( 0x30 ),	/* Flags:  out, return, */
/* 48 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 50 */	NdrFcShort( 0x12 ),	/* Type Offset=18 */

	/* Procedure gdipp_rpc_get_font_data_size */


	/* Return value */

/* 52 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 54 */	NdrFcLong( 0x0 ),	/* 0 */
/* 58 */	NdrFcShort( 0x1 ),	/* 1 */
/* 60 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 62 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 64 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 66 */	NdrFcShort( 0x34 ),	/* 52 */
/* 68 */	NdrFcShort( 0x8 ),	/* 8 */
/* 70 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x4,		/* 4 */
/* 72 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 74 */	NdrFcShort( 0x0 ),	/* 0 */
/* 76 */	NdrFcShort( 0x0 ),	/* 0 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 80 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 82 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 84 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

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

/* 98 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 100 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 102 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_data */


	/* Return value */

/* 104 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 106 */	NdrFcLong( 0x0 ),	/* 0 */
/* 110 */	NdrFcShort( 0x2 ),	/* 2 */
/* 112 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 114 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 116 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 118 */	NdrFcShort( 0x3c ),	/* 60 */
/* 120 */	NdrFcShort( 0x8 ),	/* 8 */
/* 122 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x6,		/* 6 */
/* 124 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 126 */	NdrFcShort( 0x1 ),	/* 1 */
/* 128 */	NdrFcShort( 0x0 ),	/* 0 */
/* 130 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 132 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 134 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 136 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 138 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 140 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter table */

/* 144 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 146 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 148 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter offset */

/* 150 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 152 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 154 */	NdrFcShort( 0x1e ),	/* Type Offset=30 */

	/* Parameter data_buf */

/* 156 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 158 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 160 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 162 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 164 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 166 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_size */


	/* Return value */

/* 168 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 170 */	NdrFcLong( 0x0 ),	/* 0 */
/* 174 */	NdrFcShort( 0x3 ),	/* 3 */
/* 176 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 178 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 180 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 182 */	NdrFcShort( 0x24 ),	/* 36 */
/* 184 */	NdrFcShort( 0x8 ),	/* 8 */
/* 186 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 188 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 192 */	NdrFcShort( 0x0 ),	/* 0 */
/* 194 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 196 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 198 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 200 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 202 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 204 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 206 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_data */


	/* Return value */

/* 208 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 210 */	NdrFcLong( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x4 ),	/* 4 */
/* 216 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 218 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 220 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 222 */	NdrFcShort( 0x2c ),	/* 44 */
/* 224 */	NdrFcShort( 0x8 ),	/* 8 */
/* 226 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x4,		/* 4 */
/* 228 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 230 */	NdrFcShort( 0x1 ),	/* 1 */
/* 232 */	NdrFcShort( 0x0 ),	/* 0 */
/* 234 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 236 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 238 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 240 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 242 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 244 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 246 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter metrics_buf */

/* 248 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 250 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 252 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 254 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 256 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 258 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_glyph_indices */


	/* Return value */

/* 260 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 262 */	NdrFcLong( 0x0 ),	/* 0 */
/* 266 */	NdrFcShort( 0x5 ),	/* 5 */
/* 268 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 270 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 272 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 274 */	NdrFcShort( 0x2c ),	/* 44 */
/* 276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 278 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 280 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 282 */	NdrFcShort( 0x1 ),	/* 1 */
/* 284 */	NdrFcShort( 0x1 ),	/* 1 */
/* 286 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 288 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 290 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 292 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 294 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 296 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 298 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter str */

/* 300 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 302 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 304 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter count */

/* 306 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 308 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 310 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */

	/* Parameter gi */

/* 312 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 314 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 316 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_make_glyph_run */


	/* Return value */

/* 318 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 320 */	NdrFcLong( 0x0 ),	/* 0 */
/* 324 */	NdrFcShort( 0x6 ),	/* 6 */
/* 326 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 328 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 330 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 332 */	NdrFcShort( 0x31 ),	/* 49 */
/* 334 */	NdrFcShort( 0x24 ),	/* 36 */
/* 336 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 338 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 340 */	NdrFcShort( 0x0 ),	/* 0 */
/* 342 */	NdrFcShort( 0x0 ),	/* 0 */
/* 344 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 346 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 348 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 350 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 352 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 354 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 356 */	NdrFcShort( 0x58 ),	/* Type Offset=88 */

	/* Parameter str */

/* 358 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 360 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 362 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter count */

/* 364 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 366 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 368 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Parameter is_glyph_index */

/* 370 */	NdrFcShort( 0x30 ),	/* Flags:  out, return, */
/* 372 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 374 */	NdrFcShort( 0x5a ),	/* Type Offset=90 */

	/* Procedure gdipp_rpc_get_glyph_run_size */


	/* Return value */

/* 376 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 378 */	NdrFcLong( 0x0 ),	/* 0 */
/* 382 */	NdrFcShort( 0x7 ),	/* 7 */
/* 384 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 386 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 388 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 390 */	NdrFcShort( 0x24 ),	/* 36 */
/* 392 */	NdrFcShort( 0x8 ),	/* 8 */
/* 394 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 396 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 398 */	NdrFcShort( 0x0 ),	/* 0 */
/* 400 */	NdrFcShort( 0x0 ),	/* 0 */
/* 402 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 404 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 406 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 408 */	NdrFcShort( 0x5e ),	/* Type Offset=94 */

	/* Parameter h_glyph_run */

/* 410 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 412 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 414 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_glyph_run */


	/* Return value */

/* 416 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 418 */	NdrFcLong( 0x0 ),	/* 0 */
/* 422 */	NdrFcShort( 0x8 ),	/* 8 */
/* 424 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 426 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 428 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 430 */	NdrFcShort( 0x2c ),	/* 44 */
/* 432 */	NdrFcShort( 0x5 ),	/* 5 */
/* 434 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x4,		/* 4 */
/* 436 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 438 */	NdrFcShort( 0x1 ),	/* 1 */
/* 440 */	NdrFcShort( 0x0 ),	/* 0 */
/* 442 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 444 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 446 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 448 */	NdrFcShort( 0x5e ),	/* Type Offset=94 */

	/* Parameter h_glyph_run */

/* 450 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 452 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 454 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter glyph_run_buf */

/* 456 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 458 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 460 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter glyph_run_size */

/* 462 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 464 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 466 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_release_glyph_run */


	/* Return value */

/* 468 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 470 */	NdrFcLong( 0x0 ),	/* 0 */
/* 474 */	NdrFcShort( 0x9 ),	/* 9 */
/* 476 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 478 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 480 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 482 */	NdrFcShort( 0x38 ),	/* 56 */
/* 484 */	NdrFcShort( 0x3d ),	/* 61 */
/* 486 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 488 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 490 */	NdrFcShort( 0x0 ),	/* 0 */
/* 492 */	NdrFcShort( 0x0 ),	/* 0 */
/* 494 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 496 */	NdrFcShort( 0x118 ),	/* Flags:  in, out, simple ref, */
/* 498 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 500 */	NdrFcShort( 0x66 ),	/* Type Offset=102 */

	/* Parameter h_glyph_run */

/* 502 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 504 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 506 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_end_session */


	/* Return value */

/* 508 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 510 */	NdrFcLong( 0x0 ),	/* 0 */
/* 514 */	NdrFcShort( 0xa ),	/* 10 */
/* 516 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 518 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 520 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 522 */	NdrFcShort( 0x38 ),	/* 56 */
/* 524 */	NdrFcShort( 0x3d ),	/* 61 */
/* 526 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 528 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 530 */	NdrFcShort( 0x0 ),	/* 0 */
/* 532 */	NdrFcShort( 0x0 ),	/* 0 */
/* 534 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 536 */	NdrFcShort( 0x118 ),	/* Flags:  in, out, simple ref, */
/* 538 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 540 */	NdrFcShort( 0x6e ),	/* Type Offset=110 */

	/* Parameter h_session */

/* 542 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 544 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 546 */	0x3,		/* FC_SMALL */
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
/* 18 */	0x30,		/* FC_BIND_CONTEXT */
			0x30,		/* Ctxt flags:  out, ret, */
/* 20 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 22 */	0x30,		/* FC_BIND_CONTEXT */
			0x45,		/* Ctxt flags:  in, no serialize, can't be null */
/* 24 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 26 */	
			0x11, 0x0,	/* FC_RP */
/* 28 */	NdrFcShort( 0x2 ),	/* Offset= 2 (30) */
/* 30 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 32 */	NdrFcShort( 0x1 ),	/* 1 */
/* 34 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 36 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 38 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 40 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 42 */	
			0x11, 0x0,	/* FC_RP */
/* 44 */	NdrFcShort( 0x2 ),	/* Offset= 2 (46) */
/* 46 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 48 */	NdrFcShort( 0x1 ),	/* 1 */
/* 50 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 52 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 54 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 56 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 58 */	
			0x11, 0x0,	/* FC_RP */
/* 60 */	NdrFcShort( 0x2 ),	/* Offset= 2 (62) */
/* 62 */	
			0x25,		/* FC_C_WSTRING */
			0x44,		/* FC_STRING_SIZED */
/* 64 */	0x28,		/* Corr desc:  parameter, FC_LONG */
			0x56,		/* FC_MULT_2 */
/* 66 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 68 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 70 */	
			0x11, 0x0,	/* FC_RP */
/* 72 */	NdrFcShort( 0x2 ),	/* Offset= 2 (74) */
/* 74 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 76 */	NdrFcShort( 0x2 ),	/* 2 */
/* 78 */	0x28,		/* Corr desc:  parameter, FC_LONG */
			0x56,		/* FC_MULT_2 */
/* 80 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 82 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 84 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 86 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 88 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 90 */	0x30,		/* FC_BIND_CONTEXT */
			0x30,		/* Ctxt flags:  out, ret, */
/* 92 */	0x1,		/* 1 */
			0x1,		/* 1 */
/* 94 */	0x30,		/* FC_BIND_CONTEXT */
			0x45,		/* Ctxt flags:  in, no serialize, can't be null */
/* 96 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 98 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 100 */	NdrFcShort( 0x2 ),	/* Offset= 2 (102) */
/* 102 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* Ctxt flags:  via ptr, in, out, */
/* 104 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 106 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 108 */	NdrFcShort( 0x2 ),	/* Offset= 2 (110) */
/* 110 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* Ctxt flags:  via ptr, in, out, */
/* 112 */	0x0,		/* 0 */
			0x0,		/* 0 */

			0x0
        }
    };

static const unsigned short gdipp_rpc_FormatStringOffsetTable[] =
    {
    0,
    52,
    104,
    168,
    208,
    260,
    318,
    376,
    416,
    468,
    508
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
    0x60000, /* Ndr library version */
    0,
    0x70001f4, /* MIDL Version 7.0.500 */
    0,
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

