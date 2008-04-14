

/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Thu Jan 10 14:36:00 2008
 */
/* Compiler settings for .\ILspPyFilter.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
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
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "ILspPyFilter_h.h"

#define TYPE_FORMAT_STRING_SIZE   73                                
#define PROC_FORMAT_STRING_SIZE   235                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _ILspPyFilter_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } ILspPyFilter_MIDL_TYPE_FORMAT_STRING;

typedef struct _ILspPyFilter_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } ILspPyFilter_MIDL_PROC_FORMAT_STRING;

typedef struct _ILspPyFilter_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } ILspPyFilter_MIDL_EXPR_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const ILspPyFilter_MIDL_TYPE_FORMAT_STRING ILspPyFilter__MIDL_TypeFormatString;
extern const ILspPyFilter_MIDL_PROC_FORMAT_STRING ILspPyFilter__MIDL_ProcFormatString;
extern const ILspPyFilter_MIDL_EXPR_FORMAT_STRING ILspPyFilter__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ILspPyFilter_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ILspPyFilter_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const ILspPyFilter_MIDL_PROC_FORMAT_STRING ILspPyFilter__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure ConnectFilter */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x3 ),	/* 3 */
/*  8 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 10 */	NdrFcShort( 0x4e ),	/* 78 */
/* 12 */	NdrFcShort( 0x3e ),	/* 62 */
/* 14 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x6,		/* 6 */
/* 16 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x0 ),	/* 0 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter process */

/* 24 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 26 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 28 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter thread */

/* 30 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 32 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 34 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter socket */

/* 36 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 38 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 40 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter ip */

/* 42 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 44 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 46 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter port */

/* 48 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 50 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 52 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Return value */

/* 54 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 56 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 58 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure DisconnectFilter */

/* 60 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 62 */	NdrFcLong( 0x0 ),	/* 0 */
/* 66 */	NdrFcShort( 0x4 ),	/* 4 */
/* 68 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 70 */	NdrFcShort( 0x18 ),	/* 24 */
/* 72 */	NdrFcShort( 0x8 ),	/* 8 */
/* 74 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x4,		/* 4 */
/* 76 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	NdrFcShort( 0x0 ),	/* 0 */
/* 82 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter process */

/* 84 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 86 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 88 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter thread */

/* 90 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 92 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 94 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter socket */

/* 96 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 98 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 100 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 102 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 104 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 106 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RecvFilter */

/* 108 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 110 */	NdrFcLong( 0x0 ),	/* 0 */
/* 114 */	NdrFcShort( 0x5 ),	/* 5 */
/* 116 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 118 */	NdrFcShort( 0x18 ),	/* 24 */
/* 120 */	NdrFcShort( 0x24 ),	/* 36 */
/* 122 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x7,		/* 7 */
/* 124 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 126 */	NdrFcShort( 0x1 ),	/* 1 */
/* 128 */	NdrFcShort( 0x1 ),	/* 1 */
/* 130 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter process */

/* 132 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 134 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 136 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter thread */

/* 138 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 140 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter socket */

/* 144 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 146 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 148 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter recvBuffer */

/* 150 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 152 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 154 */	NdrFcShort( 0x28 ),	/* Type Offset=40 */

	/* Parameter modifiedRecvBuffer */

/* 156 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 158 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 160 */	NdrFcShort( 0x3a ),	/* Type Offset=58 */

	/* Parameter remaining */

/* 162 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 164 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 166 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 168 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 170 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 172 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure SendFilter */

/* 174 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 176 */	NdrFcLong( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0x6 ),	/* 6 */
/* 182 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 184 */	NdrFcShort( 0x18 ),	/* 24 */
/* 186 */	NdrFcShort( 0x8 ),	/* 8 */
/* 188 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x6,		/* 6 */
/* 190 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 192 */	NdrFcShort( 0x1 ),	/* 1 */
/* 194 */	NdrFcShort( 0x1 ),	/* 1 */
/* 196 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter process */

/* 198 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 200 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 202 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter thread */

/* 204 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 206 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 208 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter socket */

/* 210 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 212 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 214 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter sendBuffer */

/* 216 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 218 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 220 */	NdrFcShort( 0x28 ),	/* Type Offset=40 */

	/* Parameter modifiedSendBuffer */

/* 222 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 224 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 226 */	NdrFcShort( 0x3a ),	/* Type Offset=58 */

	/* Return value */

/* 228 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 230 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 232 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const ILspPyFilter_MIDL_TYPE_FORMAT_STRING ILspPyFilter__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  4 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/*  6 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  8 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 10 */	
			0x11, 0x0,	/* FC_RP */
/* 12 */	NdrFcShort( 0x1c ),	/* Offset= 28 (40) */
/* 14 */	
			0x12, 0x0,	/* FC_UP */
/* 16 */	NdrFcShort( 0xe ),	/* Offset= 14 (30) */
/* 18 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 20 */	NdrFcShort( 0x2 ),	/* 2 */
/* 22 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 24 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 26 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 28 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 30 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 32 */	NdrFcShort( 0x8 ),	/* 8 */
/* 34 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (18) */
/* 36 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 38 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 40 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 42 */	NdrFcShort( 0x0 ),	/* 0 */
/* 44 */	NdrFcShort( 0x4 ),	/* 4 */
/* 46 */	NdrFcShort( 0x0 ),	/* 0 */
/* 48 */	NdrFcShort( 0xffde ),	/* Offset= -34 (14) */
/* 50 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 52 */	NdrFcShort( 0x6 ),	/* Offset= 6 (58) */
/* 54 */	
			0x13, 0x0,	/* FC_OP */
/* 56 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (30) */
/* 58 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 60 */	NdrFcShort( 0x0 ),	/* 0 */
/* 62 */	NdrFcShort( 0x4 ),	/* 4 */
/* 64 */	NdrFcShort( 0x0 ),	/* 0 */
/* 66 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (54) */
/* 68 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 70 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: ILspPyFilter, ver. 0.0,
   GUID={0x0B358C0D,0x90A4,0x46d7,{0xAE,0x74,0x2C,0xD8,0xD8,0x2E,0x24,0x88}} */

#pragma code_seg(".orpc")
static const unsigned short ILspPyFilter_FormatStringOffsetTable[] =
    {
    0,
    60,
    108,
    174
    };

static const MIDL_STUBLESS_PROXY_INFO ILspPyFilter_ProxyInfo =
    {
    &Object_StubDesc,
    ILspPyFilter__MIDL_ProcFormatString.Format,
    &ILspPyFilter_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ILspPyFilter_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    ILspPyFilter__MIDL_ProcFormatString.Format,
    &ILspPyFilter_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _ILspPyFilterProxyVtbl = 
{
    &ILspPyFilter_ProxyInfo,
    &IID_ILspPyFilter,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* ILspPyFilter::ConnectFilter */ ,
    (void *) (INT_PTR) -1 /* ILspPyFilter::DisconnectFilter */ ,
    (void *) (INT_PTR) -1 /* ILspPyFilter::RecvFilter */ ,
    (void *) (INT_PTR) -1 /* ILspPyFilter::SendFilter */
};

const CInterfaceStubVtbl _ILspPyFilterStubVtbl =
{
    &IID_ILspPyFilter,
    &ILspPyFilter_ServerInfo,
    7,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    ILspPyFilter__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x70001f3, /* MIDL Version 7.0.499 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * _ILspPyFilter_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ILspPyFilterProxyVtbl,
    0
};

const CInterfaceStubVtbl * _ILspPyFilter_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ILspPyFilterStubVtbl,
    0
};

PCInterfaceName const _ILspPyFilter_InterfaceNamesList[] = 
{
    "ILspPyFilter",
    0
};


#define _ILspPyFilter_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _ILspPyFilter, pIID, n)

int __stdcall _ILspPyFilter_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_ILspPyFilter_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo ILspPyFilter_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _ILspPyFilter_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _ILspPyFilter_StubVtblList,
    (const PCInterfaceName * ) & _ILspPyFilter_InterfaceNamesList,
    0, // no delegation
    & _ILspPyFilter_IID_Lookup, 
    1,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

