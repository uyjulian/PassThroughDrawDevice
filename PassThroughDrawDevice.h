//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//!@file "PassThrough" 描画デバイス管理
//---------------------------------------------------------------------------
#ifndef PASSTHROUGHDRAWDEVICE_H
#define PASSTHROUGHDRAWDEVICE_H

#include "tTVPDrawDevice.h"
#include "DrawDeviceTmpl.h"

// 吉里吉里2上での動作が不要であれば0にする
#ifndef USE_K2DD
#define USE_K2DD 1
#endif

class tTVPDrawer;
//---------------------------------------------------------------------------
//! @brief		「Pass Through」デバイス(もっとも基本的な描画を行うのみのデバイス)
//---------------------------------------------------------------------------
class tTVPPassThroughDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;
	typedef tTVPPassThroughDrawDevice selfclass;
	HWND TargetWindow;
	bool IsMainWindow;
	tTVPDrawer * Drawer; //!< 描画を行うもの

public:
	//! @brief	drawerのタイプ
	enum tDrawerType
	{
		dtNone, //!< drawer なし
		dtDrawDib, //!< もっとも単純なdrawer
		dtDBGDI, // GDI によるダブルバッファリングを行うdrawer
		dtDBDD, // DirectDraw によるダブルバッファリングを行うdrawer
		dtDBD3D // Direct3D によるダブルバッファリングを行うdrawer
	};

private:
	tDrawerType DrawerType; //!< drawer のタイプ
	tDrawerType PreferredDrawerType; //!< 使って欲しい drawer のタイプ

	bool DestSizeChanged; //!< DestRect のサイズに変更があったか
	bool SrcSizeChanged; //!< SrcSize に変更があったか

public:
	tTVPPassThroughDrawDevice(); //!< コンストラクタ
protected:
	~tTVPPassThroughDrawDevice(); //!< デストラクタ

private:
	virtual void OnManagerSetError(tjs_error r);

public:
	void SetToRecreateDrawer() { DestroyDrawer(); }
	void DestroyDrawer();
private:
	void CreateDrawer(tDrawerType type);
	void CreateDrawer(bool zoom_required, bool should_benchmark);

public:
	void EnsureDrawer();

	tDrawerType GetDrawerType() const { return DrawerType; }
	void SetPreferredDrawerType(tDrawerType type) { PreferredDrawerType = type; }
	tDrawerType GetPreferredDrawerType() const { return PreferredDrawerType; }

//---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);

//---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);

//---- 再描画関連
	virtual void TJS_INTF_METHOD Show();

//---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

//---- デバッグ支援
	virtual void TJS_INTF_METHOD SetShowUpdateRect(bool b);

};
//---------------------------------------------------------------------------

class tTVPPassThroughDrawDeviceWrapper : public DrawDeviceIntfTmpl<tTVPPassThroughDrawDevice> {};

//---------------------------------------------------------------------------
// tTJSNI_PassThroughDrawDevice
//---------------------------------------------------------------------------
class tTJSNI_PassThroughDrawDevice :
	public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

	tTVPPassThroughDrawDeviceWrapper * Device;

public:
	tTJSNI_PassThroughDrawDevice();
	~tTJSNI_PassThroughDrawDevice();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	tTVPPassThroughDrawDeviceWrapper * GetDevice() const { return Device; }

};
//---------------------------------------------------------------------------



#if 0
//---------------------------------------------------------------------------
// tTJSNC_PassThroughDrawDevice
//---------------------------------------------------------------------------
class tTJSNC_PassThroughDrawDevice : public tTJSNativeClass
{
public:
	tTJSNC_PassThroughDrawDevice();

	static tjs_uint32 ClassID;

private:
	iTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------
#else
class tTJSNC_PassThroughDrawDevice
{
public:
	static tTJSNativeClassForPlugin *CreateNativeClass();
	static tjs_uint32 ClassID;
	static iTJSNativeInstance *CreateNativeInstance();
};
#endif


#endif
