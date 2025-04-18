#ifndef f_VD2_VDDISPLAY_INTERNAL_RENDERERGDI_H
#define f_VD2_VDDISPLAY_INTERNAL_RENDERERGDI_H

#include <vd2/VDDisplay/renderer.h>
#include <vd2/VDDisplay/renderersoft.h>
#include <vd2/VDDisplay/renderergdi.h>

class VDDisplayCachedImageGDI;

class VDDisplayRendererGDI final : public IVDDisplayRendererGDI {
public:
	VDDisplayRendererGDI();
	~VDDisplayRendererGDI();

	void Init();
	void Shutdown();

	bool Begin(HDC hdc, sint32 w, sint32 h);
	void End();

	void FillTri(const vdpoint32 *pts) override;

	void SetTextFont(VDZHFONT hfont) override;
	void SetTextColorRGB(uint32 c) override;
	void SetTextBkTransp() override;
	void SetTextBkColorRGB(uint32 c) override;
	void SetTextAlignment(TextAlign align = TextAlign::Left, TextVAlign valign = TextVAlign::Top) override;
	void DrawTextSpan(sint32 x, sint32 y, const wchar_t *text, uint32 numChars) override;

	const VDDisplayRendererCaps& GetCaps();
	VDDisplayTextRenderer *GetTextRenderer() { return NULL; }

	void SetColorRGB(uint32 color);
	void FillRect(sint32 x, sint32 y, sint32 w, sint32 h);
	void MultiFillRect(const vdrect32 *rects, uint32 n);

	void AlphaFillRect(sint32 x, sint32 y, sint32 w, sint32 h, uint32 alphaColor);
	void AlphaTriStrip(const vdfloat2 *pts, uint32 numPts, uint32 alphaColor) {}

	void Blt(sint32 x, sint32 y, VDDisplayImageView& imageView);
	void Blt(sint32 x, sint32 y, VDDisplayImageView& imageView, sint32 sx, sint32 sy, sint32 w, sint32 h);
	void StretchBlt(sint32 dx, sint32 dy, sint32 dw, sint32 dh, VDDisplayImageView& imageView, sint32 sx, sint32 sy, sint32 sw, sint32 sh, const VDDisplayBltOptions& opts);
	void MultiBlt(const VDDisplayBlt *blts, uint32 n, VDDisplayImageView& imageView, BltMode bltMode);

	void PolyLine(const vdpoint32 *points, uint32 numLines);
	void PolyLineF(const vdfloat2 *points, uint32 numLines, bool antialiased) override {}

	virtual bool PushViewport(const vdrect32& r, sint32 x, sint32 y);
	virtual void PopViewport();

	IVDDisplayRenderer *BeginSubRender(const vdrect32& r, VDDisplaySubRenderCache& cache);
	void EndSubRender();

protected:
	void UpdateViewport();
	void UpdatePen();
	void UpdateBrush();

	VDDisplayCachedImageGDI *GetCachedImage(VDDisplayImageView& imageView);

	HDC		mhdc;
	HDC		mhdc1x1;
	HBITMAP	mhbm1x1;
	HGDIOBJ	mhbm1x1Old;
	int		mSavedDC;
	uint32	mColor;
	uint32	mPenColor;
	HPEN	mhPen;
	uint32	mBrushColor;
	HBRUSH	mhBrush;
	sint32	mWidth;
	sint32	mHeight;
	sint32	mOffsetX;
	sint32	mOffsetY;
	vdrect32	mClipRect;

	static constexpr uint32 kInvalidTextColor = 0x01000000;
	uint32	mLastTextColor = kInvalidTextColor;
	uint32	mLastTextBkColor = kInvalidTextColor;

	VDDisplaySubRenderCache *mpCurrentSubRender;
	sint32	mSubRenderX;
	sint32	mSubRenderY;

	vdlist<VDDisplayCachedImageGDI> mCachedImages;

	struct Viewport {
		vdrect32 mClipRect;
		sint32 mOffsetX;
		sint32 mOffsetY;
	};

	vdfastvector<Viewport> mViewportStack;

	VDDisplayRendererSoft mFallback;
};

#endif
