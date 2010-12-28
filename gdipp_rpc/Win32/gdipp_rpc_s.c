

/* this ALWAYS GENERATED file contains the RPC server stubs */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Dec 28 02:29:26 2010
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
#define PROC_FORMAT_STRING_SIZE   543                               
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

/* Standard interface: gdipp_rpc, ver. 0.9,
   GUID={0xDA2AAF70,0x7FD7,0x4D0D,{0x97,0x9F,0x20,0x56,0x46,0x3C,0x31,0x0A}} */


extern const MIDL_SERVER_INFO gdipp_rpc_ServerInfo;

extern RPC_DISPATCH_TABLE gdipp_rpc_v0_9_DispatchTable;

static const RPC_SERVER_INTERFACE gdipp_rpc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xDA2AAF70,0x7FD7,0x4D0D,{0x97,0x9F,0x20,0x56,0x46,0x3C,0x31,0x0A}},{0,9}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &gdipp_rpc_v0_9_DispatchTable,
    0,
    0,
    0,
    &gdipp_rpc_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE gdipp_rpc_v0_9_s_ifspec = (RPC_IF_HANDLE)& gdipp_rpc___RpcServerInterface;

extern const MIDL_STUB_DESC gdipp_rpc_StubDesc;

extern const NDR_RUNDOWN RundownRoutines[];

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
/*  8 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x8 ),	/* 8 */
/* 16 */	NdrFcShort( 0x24 ),	/* 36 */
/* 18 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x3,		/* 3 */
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

/* 40 */	NdrFcShort( 0x30 ),	/* Flags:  out, return, */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	NdrFcShort( 0x12 ),	/* Type Offset=18 */

	/* Procedure gdipp_rpc_get_font_data_size */


	/* Return value */

/* 46 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 48 */	NdrFcLong( 0x0 ),	/* 0 */
/* 52 */	NdrFcShort( 0x1 ),	/* 1 */
/* 54 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 56 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 58 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 60 */	NdrFcShort( 0x34 ),	/* 52 */
/* 62 */	NdrFcShort( 0x8 ),	/* 8 */
/* 64 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x4,		/* 4 */
/* 66 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 68 */	NdrFcShort( 0x0 ),	/* 0 */
/* 70 */	NdrFcShort( 0x0 ),	/* 0 */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 74 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 76 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 78 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 80 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 82 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 84 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter table */

/* 86 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 88 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 90 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter offset */

/* 92 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 94 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 96 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_data */


	/* Return value */

/* 98 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 100 */	NdrFcLong( 0x0 ),	/* 0 */
/* 104 */	NdrFcShort( 0x2 ),	/* 2 */
/* 106 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 108 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 110 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 112 */	NdrFcShort( 0x3c ),	/* 60 */
/* 114 */	NdrFcShort( 0x8 ),	/* 8 */
/* 116 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x6,		/* 6 */
/* 118 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 120 */	NdrFcShort( 0x1 ),	/* 1 */
/* 122 */	NdrFcShort( 0x0 ),	/* 0 */
/* 124 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 126 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 128 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 130 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 132 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 134 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 136 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter table */

/* 138 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 140 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter offset */

/* 144 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 146 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 148 */	NdrFcShort( 0x1e ),	/* Type Offset=30 */

	/* Parameter data_buf */

/* 150 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 152 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 154 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 156 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 158 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 160 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_size */


	/* Return value */

/* 162 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 164 */	NdrFcLong( 0x0 ),	/* 0 */
/* 168 */	NdrFcShort( 0x3 ),	/* 3 */
/* 170 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 172 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 174 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 176 */	NdrFcShort( 0x24 ),	/* 36 */
/* 178 */	NdrFcShort( 0x8 ),	/* 8 */
/* 180 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 182 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 188 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 190 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 192 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 194 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 196 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 198 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 200 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_font_metrics_data */


	/* Return value */

/* 202 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 204 */	NdrFcLong( 0x0 ),	/* 0 */
/* 208 */	NdrFcShort( 0x4 ),	/* 4 */
/* 210 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 212 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 214 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 216 */	NdrFcShort( 0x2c ),	/* 44 */
/* 218 */	NdrFcShort( 0x8 ),	/* 8 */
/* 220 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x4,		/* 4 */
/* 222 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 224 */	NdrFcShort( 0x1 ),	/* 1 */
/* 226 */	NdrFcShort( 0x0 ),	/* 0 */
/* 228 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 230 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 232 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 234 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 236 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 238 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 240 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter metrics_buf */

/* 242 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 244 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 246 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter buf_size */

/* 248 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 250 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 252 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_glyph_indices */


	/* Return value */

/* 254 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 256 */	NdrFcLong( 0x0 ),	/* 0 */
/* 260 */	NdrFcShort( 0x5 ),	/* 5 */
/* 262 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 264 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 266 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 268 */	NdrFcShort( 0x2c ),	/* 44 */
/* 270 */	NdrFcShort( 0x8 ),	/* 8 */
/* 272 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 274 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 276 */	NdrFcShort( 0x1 ),	/* 1 */
/* 278 */	NdrFcShort( 0x1 ),	/* 1 */
/* 280 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 282 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 284 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 286 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 288 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 290 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 292 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter str */

/* 294 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 296 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 298 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter count */

/* 300 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 302 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 304 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */

	/* Parameter gi */

/* 306 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 308 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 310 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_make_glyph_run */


	/* Return value */

/* 312 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 314 */	NdrFcLong( 0x0 ),	/* 0 */
/* 318 */	NdrFcShort( 0x6 ),	/* 6 */
/* 320 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 322 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 324 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 326 */	NdrFcShort( 0x31 ),	/* 49 */
/* 328 */	NdrFcShort( 0x24 ),	/* 36 */
/* 330 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 332 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 334 */	NdrFcShort( 0x0 ),	/* 0 */
/* 336 */	NdrFcShort( 0x0 ),	/* 0 */
/* 338 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 340 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 342 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 344 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Parameter h_session */

/* 346 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 348 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 350 */	NdrFcShort( 0x58 ),	/* Type Offset=88 */

	/* Parameter str */

/* 352 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 354 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 356 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter count */

/* 358 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 360 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 362 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Parameter is_glyph_index */

/* 364 */	NdrFcShort( 0x30 ),	/* Flags:  out, return, */
/* 366 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 368 */	NdrFcShort( 0x5a ),	/* Type Offset=90 */

	/* Procedure gdipp_rpc_get_glyph_run_size */


	/* Return value */

/* 370 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 372 */	NdrFcLong( 0x0 ),	/* 0 */
/* 376 */	NdrFcShort( 0x7 ),	/* 7 */
/* 378 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 380 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 382 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 384 */	NdrFcShort( 0x24 ),	/* 36 */
/* 386 */	NdrFcShort( 0x8 ),	/* 8 */
/* 388 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 390 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 392 */	NdrFcShort( 0x0 ),	/* 0 */
/* 394 */	NdrFcShort( 0x0 ),	/* 0 */
/* 396 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 398 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 400 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 402 */	NdrFcShort( 0x5e ),	/* Type Offset=94 */

	/* Parameter h_glyph_run */

/* 404 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 406 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 408 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_get_glyph_run */


	/* Return value */

/* 410 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 412 */	NdrFcLong( 0x0 ),	/* 0 */
/* 416 */	NdrFcShort( 0x8 ),	/* 8 */
/* 418 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 420 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 422 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 424 */	NdrFcShort( 0x2c ),	/* 44 */
/* 426 */	NdrFcShort( 0x5 ),	/* 5 */
/* 428 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x4,		/* 4 */
/* 430 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 432 */	NdrFcShort( 0x1 ),	/* 1 */
/* 434 */	NdrFcShort( 0x0 ),	/* 0 */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 438 */	NdrFcShort( 0x8 ),	/* Flags:  in, */
/* 440 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 442 */	NdrFcShort( 0x5e ),	/* Type Offset=94 */

	/* Parameter h_glyph_run */

/* 444 */	NdrFcShort( 0x113 ),	/* Flags:  must size, must free, out, simple ref, */
/* 446 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 448 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter glyph_run_buf */

/* 450 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 452 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 454 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter glyph_run_size */

/* 456 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 458 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 460 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_release_glyph_run */


	/* Return value */

/* 462 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 464 */	NdrFcLong( 0x0 ),	/* 0 */
/* 468 */	NdrFcShort( 0x9 ),	/* 9 */
/* 470 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 472 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 474 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 476 */	NdrFcShort( 0x38 ),	/* 56 */
/* 478 */	NdrFcShort( 0x3d ),	/* 61 */
/* 480 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 482 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 484 */	NdrFcShort( 0x0 ),	/* 0 */
/* 486 */	NdrFcShort( 0x0 ),	/* 0 */
/* 488 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 490 */	NdrFcShort( 0x118 ),	/* Flags:  in, out, simple ref, */
/* 492 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 494 */	NdrFcShort( 0x66 ),	/* Type Offset=102 */

	/* Parameter h_glyph_run */

/* 496 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 498 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 500 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Procedure gdipp_rpc_end_session */


	/* Return value */

/* 502 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 504 */	NdrFcLong( 0x0 ),	/* 0 */
/* 508 */	NdrFcShort( 0xa ),	/* 10 */
/* 510 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 512 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 514 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 516 */	NdrFcShort( 0x38 ),	/* 56 */
/* 518 */	NdrFcShort( 0x3d ),	/* 61 */
/* 520 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 522 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 524 */	NdrFcShort( 0x0 ),	/* 0 */
/* 526 */	NdrFcShort( 0x0 ),	/* 0 */
/* 528 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter h_gdipp_rpc */

/* 530 */	NdrFcShort( 0x118 ),	/* Flags:  in, out, simple ref, */
/* 532 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 534 */	NdrFcShort( 0x6e ),	/* Type Offset=110 */

	/* Parameter h_session */

/* 536 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 538 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 540 */	0x3,		/* FC_SMALL */
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

static const NDR_RUNDOWN RundownRoutines[] = 
    {
    GDIPP_RPC_SESSION_HANDLE_rundown
    ,GDIPP_RPC_GLYPH_RUN_HANDLE_rundown
    };


static const unsigned short gdipp_rpc_FormatStringOffsetTable[] =
    {
    0,
    46,
    98,
    162,
    202,
    254,
    312,
    370,
    410,
    462,
    502
    };


static const MIDL_STUB_DESC gdipp_rpc_StubDesc = 
    {
    (void *)& gdipp_rpc___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    RundownRoutines,
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

static RPC_DISPATCH_FUNCTION gdipp_rpc_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE gdipp_rpc_v0_9_DispatchTable = 
    {
    11,
    gdipp_rpc_table
    };

static const SERVER_ROUTINE gdipp_rpc_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)gdipp_rpc_begin_session,
    (SERVER_ROUTINE)gdipp_rpc_get_font_data_size,
    (SERVER_ROUTINE)gdipp_rpc_get_font_data,
    (SERVER_ROUTINE)gdipp_rpc_get_font_metrics_size,
    (SERVER_ROUTINE)gdipp_rpc_get_font_metrics_data,
    (SERVER_ROUTINE)gdipp_rpc_get_glyph_indices,
    (SERVER_ROUTINE)gdipp_rpc_make_glyph_run,
    (SERVER_ROUTINE)gdipp_rpc_get_glyph_run_size,
    (SERVER_ROUTINE)gdipp_rpc_get_glyph_run,
    (SERVER_ROUTINE)gdipp_rpc_release_glyph_run,
    (SERVER_ROUTINE)gdipp_rpc_end_session
    };

static const MIDL_SERVER_INFO gdipp_rpc_ServerInfo = 
    {
    &gdipp_rpc_StubDesc,
    gdipp_rpc_ServerRoutineTable,
    gdipp_rpc__MIDL_ProcFormatString.Format,
    gdipp_rpc_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

