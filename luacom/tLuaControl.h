/**
 tLuaControl.h
*/

#ifndef __TLUACONTROL_H
#define __TLUACONTROL_H

#include <ole2.h>
#include <oleidl.h>
#include <ocidl.h>

#include "tLuaDispatch.h"
#include "tCOMUtil.h"

class tLuaControl:
        public tLuaDispatch,
        public IOleObject, public IOleControl, public IOleInPlaceObject,
        public IOleInPlaceActiveObject, public IViewObject2,
        public IPersistStreamInit, public IPersistStorage, public IQuickActivate
{
public:
    // Construction/destruction methods	

    tLuaControl(lua_State* L, ITypeInfo *pTypeinfo, int ref);
    ~tLuaControl();
    static tLuaControl *CreateLuaControl(lua_State* L, ITypeInfo* typeinfo, int ref);

    // IUnknown methods.

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IPersist methods.

    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID);

    // IPersistStreamInit methods

    STDMETHOD(IsDirty)(THIS);
    STDMETHOD(Load)(LPSTREAM pStm);
    STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize);
    STDMETHOD(InitNew)();

    // IPersistStorage methods

    STDMETHOD(InitNew)(IStorage  *pStg);
    STDMETHOD(Load)(IStorage  *pStg);
    STDMETHOD(Save)(IStorage  *pStgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage  *pStgNew);
    STDMETHOD(HandsOffStorage)();

    // IOleControl methods

    STDMETHOD(GetControlInfo)(LPCONTROLINFO pCI);
    STDMETHOD(OnMnemonic)(LPMSG pMsg);
    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);
    STDMETHOD(FreezeEvents)(BOOL bFreeze);

    // IOleObject methods

    STDMETHOD(SetClientSite)(IOleClientSite* pcs);
    STDMETHOD(GetClientSite)(IOleClientSite** ppcs);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(SetMoniker)(DWORD dwWitchMoniker, IMoniker* pmk);
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWitchMoniker, IMoniker** ppMoniker);
    STDMETHOD(InitFromData)(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved);
    STDMETHOD(IsUpToDate)();
    STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject** ppDataObject);
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite,
        LONG lIndex, HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB** ppEnumOleVerb);
    STDMETHOD(Update)();
    STDMETHOD(GetUserClassID)(CLSID* pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR* pszUserType);
    STDMETHOD(SetExtent)(DWORD dwAspect, SIZEL* pSizel);
    STDMETHOD(GetExtent)(DWORD dwAspect, SIZEL* pSizel);
    STDMETHOD(Advise)(IAdviseSink* pAdvSink, DWORD* pdwConnection);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA** ppEnumAdvise);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD* pdwStatus);
    STDMETHOD(SetColorScheme)(LOGPALETTE* pLogpal);

    // IOleInPlaceObject methods

    STDMETHOD(GetWindow)(HWND* phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
    STDMETHOD(InPlaceDeactivate)();
    STDMETHOD(UIDeactivate)();
    STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHOD(ReactivateAndUndo)();

    // IOleInPlaceActiveObject methods

    STDMETHOD(TranslateAccelerator)(LPMSG lpmsg);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder,
            IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow);
    STDMETHOD(EnableModeless)(BOOL fEnable);

    // IViewObject2 methods

    STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex, void  *pvAspect,
                    DVTARGETDEVICE  *ptd, HDC hdcTargetDev, HDC hdcDraw,
                    LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                    BOOL ( __stdcall  *pfnContinue )(ULONG_PTR dwContinue),
                    ULONG_PTR dwContinue);
    STDMETHOD(GetColorSet)(DWORD dwDrawAspect,LONG lindex, void  *pvAspect,
                           DVTARGETDEVICE  *ptd, HDC hicTargetDev,
                           LOGPALETTE  * *ppColorSet);
    STDMETHOD(Freeze)(DWORD dwDrawAspect, LONG lindex,
                      void  *pvAspect,DWORD  *pdwFreeze);
    STDMETHOD(Unfreeze)(DWORD dwFreeze);
    STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf, IAdviseSink  *pAdvSink);
    STDMETHOD(GetAdvise)(DWORD *pAspects, DWORD  *pAdvf, IAdviseSink  * *ppAdvSink);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE __RPC_FAR *ptd, LPSIZEL lpsizel);

    // IQuickActivate methods

    STDMETHOD(QuickActivate)(QACONTAINER *pqacontainer, QACONTROL *pqacontrol);
    STDMETHOD(SetContentExtent)(LPSIZEL);
    STDMETHOD(GetContentExtent)(LPSIZEL);
    
private:
    HRESULT InPlaceActivate(bool lVerb);
    void SetInPlaceVisible(bool fShow);
    HWND CreateInPlaceWindow(int x, int y, bool fNoRedraw);
    void DestroyWindow();	

    tCOMPtr<IOleAdviseHolder> m_pOleAdviseHolder;     // OleObject::Advise holder object
    tCOMPtr<IAdviseSink> m_pViewAdviseSink;           // sink for IViewObject2
    tCOMPtr<IOleClientSite> m_pClientSite;            // client site
    tCOMPtr<IOleControlSite> m_pControlSite;          // ptr on client site
    tCOMPtr<IOleInPlaceSite> m_pInPlaceSite;          // for managing activation
    tCOMPtr<IOleInPlaceFrame> m_pInPlaceFrame;        // ptr on client site
    tCOMPtr<IOleInPlaceUIWindow> m_pInPlaceUIWindow;  // for negotiating border space with client
    SIZEL m_Size;                   // the size of this control    
    RECT m_rcLocation;              // location
    HWND m_hwnd;                    // window handle
    HWND m_hwndParent;              // parent window handle
    HRGN m_hRgn;                    // GDI region

    bool m_fDirty:1;                       // does the control need to be resaved?
    bool m_fInPlaceActive:1;               // are we in place active or not?
    bool m_fInPlaceVisible:1;              // are we in place visible or not?
    bool m_fUIActive:1;                    // are we UI active or not?
    bool m_fViewAdvisePrimeFirst:1;        // for IViewObject2::SetAdvise
    bool m_fViewAdviseOnlyOnce:1;          // for IViewObject2::SetAdvise
};


/**
  Standard IEnumOLEVERB implementation backed by C array.
  Closely parallels CEnumOLEVERB on http://spec.winprog.org/captcom/journal3/page3.html
*/
class CEnumOLEVERB : public IEnumOLEVERB
{
public:
    // Construction

    CEnumOLEVERB(const OLEVERB* verbs, ULONG verbsSize, ULONG verbsNext=0)
         : mRefCount(0), mVerbs(verbs), mVerbsSize(verbsSize), mVerbsNext(verbsNext) {}
    ~CEnumOLEVERB() { }
    
    // IUnknown methods
    
    ULONG _stdcall AddRef() { return ++mRefCount; }
    ULONG _stdcall Release()
    {
        ULONG ret = --mRefCount;
        if(ret == 0) delete this;
        return ret;   // careful: this->_refCount may be deleted
    }
    HRESULT _stdcall QueryInterface(REFIID riid, void** ppv)
    {
        if(!ppv) return E_POINTER;
        else if(riid == IID_IUnknown || riid == IID_IEnumOLEVERB)
        { *ppv = this; AddRef(); return S_OK; }
        else { *ppv = NULL; return E_NOINTERFACE; }
    }
       
    // IEnumOLEVERB methods

    HRESULT _stdcall Clone(IEnumOLEVERB** ppenum)
    {
        *ppenum = new CEnumOLEVERB(mVerbs, mVerbsSize, mVerbsNext);
        return S_OK;
    }
    
    HRESULT _stdcall Next(ULONG celt, OLEVERB* rgelt, ULONG* pceltFetched)
    {
        if(!rgelt) return E_POINTER; // unspecified behavior (violates precondition)
        ULONG i;
        for(i = 0; i < celt && mVerbsNext + i < mVerbsSize; ++i)
        {
            int verbsi = mVerbsNext + i;
            rgelt[i] = mVerbs[verbsi];
            rgelt[i].lpszVerbName = static_cast<wchar_t*>(
                CoTaskMemAlloc(lstrlenW(mVerbs[verbsi].lpszVerbName) * 2 + 2));
            lstrcpyW(rgelt[i].lpszVerbName, mVerbs[verbsi].lpszVerbName);
        }
        mVerbsNext += i;
        if(pceltFetched) *pceltFetched = i;
        return (i == celt) ? S_OK : S_FALSE;
    }

    HRESULT _stdcall Reset()
    {
        mVerbsNext = 0;
        return S_OK;
    }

    HRESULT _stdcall Skip(ULONG celt)
    {
        ULONG n = mVerbsNext + static_cast<ULONG>(celt);
        mVerbsNext = n > mVerbsSize ? mVerbsSize : n;
        return S_OK;
    }

private:
    ULONG mRefCount;         // IUnknown reference counter
    const OLEVERB* mVerbs;  // verbs array (memory not owned)
    ULONG mVerbsSize;        // size of mVerbs array
    ULONG mVerbsNext;        // mVerbs[mVerbsNext] is returned by Next()
};

#endif // __TLUACONTROL_H
