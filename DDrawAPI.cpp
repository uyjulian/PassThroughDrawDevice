
#include <windows.h>
#include "tp_stub.h"

#include "DDrawAPI.h"

#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>

#define TVPCannotInitDirectDraw TJS_W("DirectDraw を初期化できません : %1")

//---------------------------------------------------------------------------
// Color Format Detection
//---------------------------------------------------------------------------
tjs_int TVPDisplayColorFormat = 0;
static tjs_int TVPGetDisplayColorFormat()
{
	// detect current 16bpp display color format
	// return value:
	// 555 : 16bit 555 mode
	// 565 : 16bit 565 mode
	// 0   : other modes

	// create temporary bitmap and device contexts
	HDC desktopdc = GetDC(0);
	HDC bitmapdc = CreateCompatibleDC(desktopdc);
	HBITMAP bmp = CreateCompatibleBitmap(desktopdc, 1, 1);
	HBITMAP oldbmp = SelectObject(bitmapdc, bmp);

	int count;
	int r, g, b;
	COLORREF lastcolor;

	// red
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(i, 0, 0));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	r = count;

	// green
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(0, i, 0));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	g = count;

	// blue
	count = 0;
	lastcolor = 0xffffff;
	for(int i = 0; i < 256; i++)
	{
		SetPixel(bitmapdc, 0, 0, RGB(0, 0, i));
		COLORREF rgb = GetPixel(bitmapdc, 0, 0);
		if(rgb != lastcolor) count ++;
		lastcolor = rgb;
	}
	b = count;

	// free bitmap and device contexts
	SelectObject(bitmapdc, oldbmp);
	DeleteObject(bmp);
	DeleteDC(bitmapdc);
	ReleaseDC(0, desktopdc);

	// determine type
	if(r == 32 && g == 64 && b == 32)
	{
		TVPDisplayColorFormat = 565;
		return 565;
	}
	else if(r == 32 && g == 32 && b == 32)
	{
		TVPDisplayColorFormat = 555;
		return 555;
	}
	else
	{
		TVPDisplayColorFormat = 0;
		return 0;
	}
}
//---------------------------------------------------------------------------

static IDirectDraw *TVPDirectDraw=NULL;
static IDirectDraw2 *TVPDirectDraw2=NULL;
static IDirectDraw7 *TVPDirectDraw7=NULL;
static HRESULT WINAPI (*TVPDirectDrawCreate)
	( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter ) = NULL;
static HRESULT WINAPI (*TVPDirectDrawCreateEx)
	( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter ) = NULL;

static HRESULT WINAPI (*TVPDirectDrawEnumerateA)
	( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext ) = NULL;
static HRESULT WINAPI (*TVPDirectDrawEnumerateExA)
	( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags) = NULL;

static HMODULE TVPDirectDrawDLLHandle=NULL;

static IDirectDrawSurface * TVPDDPrimarySurface = NULL;
bool TVPDDPrimarySurfaceFailed = false;

static BOOL WINAPI DDEnumCallbackEx( GUID *pGUID, LPSTR pDescription, LPSTR strName,
							  LPVOID pContext, HMONITOR hm )
{
	ttstr log(TJS_W("(info) DirectDraw Driver/Device found : "));
	if(pDescription)
		log += ttstr(pDescription);
	if(strName)
		log += TJS_W(" [") + ttstr(strName) + TJS_W("]");
	char tmp[60];
	sprintf(tmp, "0x%p", hm);
	log += TJS_W(" (monitor: ") + ttstr(tmp) + TJS_W(")");
	TVPAddImportantLog(log);

	return  DDENUMRET_OK;
}
//---------------------------------------------------------------------------
static BOOL WINAPI DDEnumCallback( GUID *pGUID, LPSTR pDescription,
							LPSTR strName, LPVOID pContext )
{
	return ( DDEnumCallbackEx( pGUID, pDescription, strName, pContext, NULL ) );
}
//---------------------------------------------------------------------------
#if 0
void TVPDumpDirectDrawDriverInformation()
{
	if(TVPDirectDraw7)
	{
		IDirectDraw7 *dd7 = TVPDirectDraw7;
		static bool dumped = false;
		if(dumped) return;
		dumped = true;

		TVPAddImportantLog(TJS_W("(info) DirectDraw7 or higher detected. Retrieving current DirectDraw driver information..."));

		try
		{
			// dump directdraw information
			DDDEVICEIDENTIFIER2 DDID = {0};
			if(SUCCEEDED(dd7->GetDeviceIdentifier(&DDID, 0)))
			{
				ttstr infostart(TJS_W("(info)  "));
				ttstr log;

				// driver string
				log = infostart + ttstr(DDID.szDescription) + TJS_W(" [") + ttstr(DDID.szDriver) + TJS_W("]");
				TVPAddImportantLog(log);

				// driver version(reported)
				log = infostart + TJS_W("Driver version (reported) : ");
				char tmp[256];
				wsprintfA( tmp, "%d.%02d.%02d.%04d ",
						  HIWORD( DDID.liDriverVersion.u.HighPart ),
						  LOWORD( DDID.liDriverVersion.u.HighPart ),
						  HIWORD( DDID.liDriverVersion.u.LowPart  ),
						  LOWORD( DDID.liDriverVersion.u.LowPart  ) );
				log += tmp;
				TVPAddImportantLog(log);

				// driver version(actual)
				char driverpath[1024];
				char *driverpath_filename = NULL;
				bool success = SearchPath(NULL, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);

				if(!success)
				{
					char syspath[1024];
					GetSystemDirectory(syspath, 1023);
					strcat(syspath, "\\drivers"); // SystemDir\drivers
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(!success)
				{
					char syspath[1024];
					GetWindowsDirectory(syspath, 1023);
					strcat(syspath, "\\system32"); // WinDir\system32
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(!success)
				{
					char syspath[1024];
					GetWindowsDirectory(syspath, 1023);
					strcat(syspath, "\\system32\\drivers"); // WinDir\system32\drivers
					success = SearchPath(syspath, DDID.szDriver, NULL, 1023, driverpath, &driverpath_filename);
				}

				if(success)
				{
					log = infostart + TJS_W("Driver version (") + ttstr(driverpath) + TJS_W(") : ");
					tjs_int major, minor, release, build;
					if(TVPGetFileVersionOf(driverpath, major, minor, release, build))
					{
						wsprintfA(tmp, "%d.%d.%d.%d", (int)major, (int)minor, (int)release, (int)build);
						log += tmp;
					}
					else
					{
						log += TJS_W("unknown");
					}
				}
				else
				{
					log = infostart + TJS_W("Driver ") + ttstr(DDID.szDriver) +
						TJS_W(" is not found in search path.");
				}
				TVPAddImportantLog(log);

				// device id
				wsprintfA(tmp, "VendorId:%08X  DeviceId:%08X  SubSysId:%08X  Revision:%08X",
					DDID.dwVendorId, DDID.dwDeviceId, DDID.dwSubSysId, DDID.dwRevision);
				log = infostart + TJS_W("Device ids : ") + tmp;
				TVPAddImportantLog(log);

				// Device GUID
				GUID *pguid = &DDID.guidDeviceIdentifier;
				wsprintfA( tmp, "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
						  pguid->Data1,
						  pguid->Data2,
						  pguid->Data3,
						  pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3],
						  pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] );
				log = infostart + TJS_W("Unique driver/device id : ") + tmp;
				TVPAddImportantLog(log);

				// WHQL level
				wsprintfA(tmp, "%08x", DDID.dwWHQLLevel);
				log = infostart + TJS_W("WHQL level : ")  + tmp;
				TVPAddImportantLog(log);
			}
			else
			{
				TVPAddImportantLog(TJS_W("(info) Failed."));
			}
		}
		catch(...)
		{
		}
	}

}
#endif
//---------------------------------------------------------------------------
static void TVPUnloadDirectDraw();
static void TVPInitDirectDraw()
{
	if(!TVPDirectDrawDLLHandle)
	{
		// load ddraw.dll
		TVPAddLog(TJS_W("(info) Loading DirectDraw ..."));
		TVPDirectDrawDLLHandle = LoadLibraryA("ddraw.dll");
		if(!TVPDirectDrawDLLHandle)
			TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
				TJS_W("Cannot load ddraw.dll"));

		// Enumerate display drivers, for debugging information
		try
		{
			TVPDirectDrawEnumerateExA = (HRESULT WINAPI (*)
				( LPDDENUMCALLBACKEXA , LPVOID , DWORD )	)
					GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawEnumerateExA");
			if(TVPDirectDrawEnumerateExA)
			{
				TVPDirectDrawEnumerateExA( DDEnumCallbackEx, NULL,
										  DDENUM_ATTACHEDSECONDARYDEVICES |
										  DDENUM_DETACHEDSECONDARYDEVICES |
										  DDENUM_NONDISPLAYDEVICES );
			}
			else
			{
				TVPDirectDrawEnumerateA = (HRESULT WINAPI (*)
					( LPDDENUMCALLBACKA , LPVOID  ))
					GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawEnumerateA");
				if(TVPDirectDrawEnumerateA)
				{
			        TVPDirectDrawEnumerateA( DDEnumCallback, NULL );
				}
			}
		}
		catch(...)
		{
			// Ignore errors
		}
	}

	if(!TVPDirectDraw2)
	{
		try
		{
			// get DirectDrawCreaet function
			TVPDirectDrawCreate = (HRESULT(WINAPI*)(_GUID*,IDirectDraw**,IUnknown*))
				GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawCreate");
			if(!TVPDirectDrawCreate)
				TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
					TJS_W("Missing DirectDrawCreate in ddraw.dll"));

			TVPDirectDrawCreateEx = (HRESULT(WINAPI*)( GUID FAR *, LPVOID  *, REFIID,IUnknown FAR *))
				GetProcAddress(TVPDirectDrawDLLHandle, "DirectDrawCreateEx");

			// create IDirectDraw object
			if(TVPDirectDrawCreateEx)
			{
				HRESULT hr;
				hr = TVPDirectDrawCreateEx(NULL, (void**)&TVPDirectDraw7, IID_IDirectDraw7, NULL);
 				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("DirectDrawCreateEx failed./HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				// retrieve IDirecDraw2 interface
				hr = TVPDirectDraw7->QueryInterface(IID_IDirectDraw2,
					(void **)&TVPDirectDraw2);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("Querying of IID_IDirectDraw2 failed."
							"/HR="))+
							TJSInt32ToHex((tjs_uint32)hr));
			}
			else
			{
				HRESULT hr;

				hr = TVPDirectDrawCreate(NULL, &TVPDirectDraw, NULL);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("DirectDrawCreate failed./HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				// retrieve IDirecDraw2 interface
				hr = TVPDirectDraw->QueryInterface(IID_IDirectDraw2,
					(void **)&TVPDirectDraw2);
				if(FAILED(hr))
					TVPThrowExceptionMessage(TVPCannotInitDirectDraw,
						ttstr(TJS_W("Querying of IID_IDirectDraw2 failed."
							" (DirectX on this system may be too old)/HR="))+
							TJSInt32ToHex((tjs_uint32)hr));

				TVPDirectDraw->Release(), TVPDirectDraw = NULL;

				// retrieve IDirectDraw7 interface
				hr = TVPDirectDraw2->QueryInterface(IID_IDirectDraw7, (void**)&TVPDirectDraw7);
				if(FAILED(hr)) TVPDirectDraw7 = NULL;
			}


#if 0
			if(TVPLoggingToFile)
			{
				TVPDumpDirectDrawDriverInformation();
			}
#endif

			// set cooperative level
			if(TVPDirectDraw7)
				TVPDirectDraw7->SetCooperativeLevel(NULL, DDSCL_NORMAL);
			else
				TVPDirectDraw2->SetCooperativeLevel(NULL, DDSCL_NORMAL);
		}
		catch(...)
		{
			TVPUnloadDirectDraw();
			throw;
		}
	}

	TVPGetDisplayColorFormat();
}
//---------------------------------------------------------------------------
static void TVPUninitDirectDraw()
{
	// release DirectDraw object ( DLL will not be released )
}
//---------------------------------------------------------------------------
static void TVPUnloadDirectDraw()
{
	// release DirectDraw object and /*release it's DLL */
	TVPUninitDirectDraw();
	if(TVPDDPrimarySurface) TVPDDPrimarySurface->Release(), TVPDDPrimarySurface = NULL;
	if(TVPDirectDraw7) TVPDirectDraw7->Release(), TVPDirectDraw7 = NULL;
	if(TVPDirectDraw2) TVPDirectDraw2->Release(), TVPDirectDraw2 = NULL;
	if(TVPDirectDraw) TVPDirectDraw -> Release(), TVPDirectDraw = NULL;
//	if(TVPDirectDrawDLLHandle)
//		FreeLibrary(TVPDirectDrawDLLHandle), TVPDirectDrawDLLHandle = NULL;

	TVPGetDisplayColorFormat();
}
//---------------------------------------------------------------------------
void TVPEnsureDirectDrawObject()
{
	try
	{
		TVPInitDirectDraw();
	}
	catch(...)
	{
	}
}
//---------------------------------------------------------------------------
IDirectDraw2 * TVPGetDirectDrawObjectNoAddRef()
{
	// retrieves DirectDraw2 interface
	return TVPDirectDraw2;
}
//---------------------------------------------------------------------------
IDirectDraw7 * TVPGetDirectDraw7ObjectNoAddRef()
{
	// retrieves DirectDraw7 interface
	return TVPDirectDraw7;
}
//---------------------------------------------------------------------------
IDirectDrawSurface * TVPGetDDPrimarySurfaceNoAddRef()
{
	if(TVPDDPrimarySurfaceFailed) return NULL;
	if(TVPDDPrimarySurface) return TVPDDPrimarySurface;

	TVPEnsureDirectDrawObject();

	if(!TVPDirectDraw2)
	{
		// DirectDraw not available
		TVPDDPrimarySurfaceFailed = true;
		return NULL;
	}

	DDSURFACEDESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	HRESULT hr;
	hr = TVPDirectDraw2->CreateSurface(&ddsd, &TVPDDPrimarySurface, NULL);
	if(hr != DD_OK)
	{
		// failed to create DirectDraw primary surface
		TVPDDPrimarySurface = NULL;
		TVPDDPrimarySurfaceFailed = true;
		return NULL;
	}

	return TVPDDPrimarySurface;
}
//---------------------------------------------------------------------------
void TVPSetDDPrimaryClipper(IDirectDrawClipper *clipper)
{
	// set clipper object

	IDirectDrawSurface * pri = TVPGetDDPrimarySurfaceNoAddRef();

	// set current clipper object
	if(pri) pri->SetClipper(clipper);
}
//---------------------------------------------------------------------------
void TVPReleaseDDPrimarySurface()
{
	if(TVPDDPrimarySurface) TVPDDPrimarySurface->Release(), TVPDDPrimarySurface = NULL;
}
//---------------------------------------------------------------------------


#if 0
//---------------------------------------------------------------------------
static tTVPAtExit
	TVPUnloadDirectDrawAtExit(TVP_ATEXIT_PRI_RELEASE, TVPUnloadDirectDraw);
//---------------------------------------------------------------------------
#endif
