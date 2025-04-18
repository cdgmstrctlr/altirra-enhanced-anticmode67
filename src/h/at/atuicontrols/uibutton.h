#ifndef f_AT_UIBUTTON_H
#define f_AT_UIBUTTON_H

#include <vd2/system/function.h>
#include <vd2/system/VDString.h>
#include <vd2/VDDisplay/font.h>
#include <at/atui/uiwidget.h>

class ATUIButton : public ATUIWidget {
public:
	enum {
		kActionActivate = kActionCustom
	};

	ATUIButton();
	~ATUIButton();

	bool IsHeld() const { return mbHeld; }

	void SetStockImage(sint32 id);
	void SetText(const wchar_t *s);
	void SetTextColor(uint32 color);
	void SetTextColorDisabled(uint32 color);
	void SetDepressed(bool depressed);
	void SetToggleMode(bool enabled);
	void SetFrameEnabled(bool enabled);

	vdfunction<void()>& OnPressedEvent() { return mPressedEvent; }
	vdfunction<void()>& OnActivatedEvent() { return mActivatedEvent; }

	vdfunction<void(bool)>& OnHeldEvent() { return mHeldEvent; }

public:
	void OnMouseDownL(sint32 x, sint32 y) override;
	void OnMouseUpL(sint32 x, sint32 y) override;

	void OnActionStart(uint32 id) override;
	void OnActionStop(uint32 id) override;

	void OnCreate() override;
	void OnSize() override;
	ATUIWidgetMetrics OnMeasure() override;

	void OnEnableChanged() override;

	void OnSetFocus() override;
	void OnKillFocus() override;

protected:
	void SetHeld(bool held);
	void Paint(IVDDisplayRenderer& rdr, sint32 w, sint32 h);
	void Relayout();
	void UpdateFillColor();

	sint32 mStockImageIdx;
	bool mbDepressed;
	bool mbHeld;
	bool mbToggleMode;
	bool mbToggleNextState;
	bool mbFrameEnabled;
	uint32 mTextColor;
	uint32 mTextColorDisabled;
	sint32 mTextX;
	sint32 mTextY;
	VDStringW mText;
	vdrefptr<IVDDisplayFont> mpFont;

	vdfunction<void()> mActivatedEvent;
	vdfunction<void()> mPressedEvent;
	vdfunction<void(bool)> mHeldEvent;
};

#endif
