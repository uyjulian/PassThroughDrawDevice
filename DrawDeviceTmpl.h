
#ifndef __DRAWDEVICETMPL_H__
#define __DRAWDEVICETMPL_H__

#include "Krkr2DrawDeviceWrapper.h"

// 破棄通知+吉里吉里2ラッパー切り替え付きテンプレート
template <class DDBASE, bool SUPPORTK2 = true>
class DrawDeviceIntfTmpl : public DDBASE {
	typedef  DDBASE inherited;
protected:
	Krkr2DrawDeviceWrapper k2dd; // 吉里吉里2向けDrawDeviceインターフェース互換ラッパー
public:
	DrawDeviceIntfTmpl() : inherited(), k2dd(this) {}
	tTVInteger GetInterface() const { return k2dd.SwitchGetInterface(); } // 吉里吉里Zか2でインターフェースを自動で切り替える
};

// 吉里吉里2をサポートしない版
template <class DDBASE>
class DrawDeviceIntfTmpl<DDBASE, false> : public DDBASE {
	typedef  DDBASE inherited;
public:
	DrawDeviceIntfTmpl() : inherited() {}
	tTVInteger GetInterface() const { return reinterpret_cast<tTVInteger>(this); }
};

#endif
