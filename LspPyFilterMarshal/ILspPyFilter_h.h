

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ILspPyFilter_h_h__
#define __ILspPyFilter_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ILspPyFilter_FWD_DEFINED__
#define __ILspPyFilter_FWD_DEFINED__
typedef interface ILspPyFilter ILspPyFilter;
#endif 	/* __ILspPyFilter_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ILspPyFilter_INTERFACE_DEFINED__
#define __ILspPyFilter_INTERFACE_DEFINED__

/* interface ILspPyFilter */
/* [oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ILspPyFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B358C0D-90A4-46d7-AE74-2CD8D82E2488")
    ILspPyFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [out][in] */ unsigned int *ip,
            /* [out][in] */ unsigned short *port) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecvFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *recvBuffer,
            /* [out] */ BSTR *modifiedRecvBuffer,
            /* [out] */ unsigned int *remaining) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *sendBuffer,
            /* [out] */ BSTR *modifiedSendBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILspPyFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILspPyFilter * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILspPyFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILspPyFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectFilter )( 
            ILspPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [out][in] */ unsigned int *ip,
            /* [out][in] */ unsigned short *port);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectFilter )( 
            ILspPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket);
        
        HRESULT ( STDMETHODCALLTYPE *RecvFilter )( 
            ILspPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *recvBuffer,
            /* [out] */ BSTR *modifiedRecvBuffer,
            /* [out] */ unsigned int *remaining);
        
        HRESULT ( STDMETHODCALLTYPE *SendFilter )( 
            ILspPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *sendBuffer,
            /* [out] */ BSTR *modifiedSendBuffer);
        
        END_INTERFACE
    } ILspPyFilterVtbl;

    interface ILspPyFilter
    {
        CONST_VTBL struct ILspPyFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILspPyFilter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILspPyFilter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILspPyFilter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILspPyFilter_ConnectFilter(This,process,thread,socket,ip,port)	\
    ( (This)->lpVtbl -> ConnectFilter(This,process,thread,socket,ip,port) ) 

#define ILspPyFilter_DisconnectFilter(This,process,thread,socket)	\
    ( (This)->lpVtbl -> DisconnectFilter(This,process,thread,socket) ) 

#define ILspPyFilter_RecvFilter(This,process,thread,socket,recvBuffer,modifiedRecvBuffer,remaining)	\
    ( (This)->lpVtbl -> RecvFilter(This,process,thread,socket,recvBuffer,modifiedRecvBuffer,remaining) ) 

#define ILspPyFilter_SendFilter(This,process,thread,socket,sendBuffer,modifiedSendBuffer)	\
    ( (This)->lpVtbl -> SendFilter(This,process,thread,socket,sendBuffer,modifiedSendBuffer) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILspPyFilter_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


