
#ifndef __DDRAW_API_H__
#define __DDRAW_API_H__

#if 0
TJS_EXP_FUNC_DEF(void, TVPEnsureDirectDrawObject, ());
#else
extern void TVPEnsureDirectDrawObject();
#endif

extern void TVPDumpDirectDrawDriverInformation();
/*[*/
//---------------------------------------------------------------------------
// DirectDraw former declaration
//---------------------------------------------------------------------------
#ifndef __DDRAW_INCLUDED__
struct IDirectDraw2;
struct IDirectDraw7;
struct IDirectDrawSurface;
struct IDirectDrawClipper;
#endif

/*]*/
#if 0
TJS_EXP_FUNC_DEF(IDirectDraw2 *,  TVPGetDirectDrawObjectNoAddRef, ());
TJS_EXP_FUNC_DEF(IDirectDraw7 *,  TVPGetDirectDraw7ObjectNoAddRef, ());
TJS_EXP_FUNC_DEF(IDirectDrawSurface *, TVPGetDDPrimarySurfaceNoAddRef, ());
TJS_EXP_FUNC_DEF(void, TVPSetDDPrimaryClipper, (IDirectDrawClipper * clipper));
TJS_EXP_FUNC_DEF(void, TVPReleaseDDPrimarySurface, ());
#else
extern IDirectDraw2 *TVPGetDirectDrawObjectNoAddRef();
extern IDirectDraw7 *TVPGetDirectDraw7ObjectNoAddRef();
extern IDirectDrawSurface *TVPGetDDPrimarySurfaceNoAddRef();
extern void TVPSetDDPrimaryClipper(IDirectDrawClipper *clipper);
extern void TVPReleaseDDPrimarySurface();
#endif

#endif
