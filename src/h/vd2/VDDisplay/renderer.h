#ifndef f_VD2_VDDISPLAY_RENDERER_H
#define f_VD2_VDDISPLAY_RENDERER_H

#include <vd2/Kasumi/pixmap.h>
#include <vd2/system/refcount.h>
#include <vd2/system/unknown.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>

class VDDisplayImageView;
class VDDisplayTextRenderer;

struct VDDisplayBlt {
	sint32 mDestX;
	sint32 mDestY;
	sint32 mSrcX;
	sint32 mSrcY;
	sint32 mWidth;
	sint32 mHeight;
};

struct VDDisplayRendererCaps {
	bool mbSupportsAlphaBlending;		// non-text alpha blended primitives are supported
	bool mbSupportsColorBlt;			// Color mode blits are supported (subpixel AA with very approximate gamma correction)
	bool mbSupportsColorBlt2;			// Color2 mode blits are supported (subpixel AA with slightly better gamma correction)
	bool mbSupportsPolyLineF;			// PolyLineF() is supported for better precision line points
	bool mbSupportsPolyLineFAA;			// PolyLineF(antialiasing = true) is supported (ignored otherwise)
	bool mbSupportsHDR;					// Framebuffer is HDR and HDR drawing commands are fully supported
};

class VDDisplaySubRenderCache {
public:
	VDDisplaySubRenderCache();
	~VDDisplaySubRenderCache();

	void SetCache(IVDRefUnknown *p) { mpCache = p; }
	IVDRefUnknown *GetCache() const { return mpCache; }

	void Invalidate() { ++mUniquenessCounter; }

	uint32 GetUniquenessCounter() const { return mUniquenessCounter; }

protected:
	vdrefptr<IVDRefUnknown> mpCache;
	uint32 mUniquenessCounter;
};

struct VDDisplayBltOptions {
	enum FilterMode {
		kFilterMode_Point,
		kFilterMode_Bilinear
	};

	FilterMode mFilterMode = kFilterMode_Point;
	float mSharpnessX = 0.0f;
	float mSharpnessY = 0.0f;

	enum class AlphaBlendMode : uint8 {
		None,
		Over,
		OverPreMultiplied
	};

	AlphaBlendMode mAlphaBlendMode = AlphaBlendMode::None;
};

class IVDDisplayRenderer {
public:
	virtual const VDDisplayRendererCaps& GetCaps() = 0;

	virtual VDDisplayTextRenderer *GetTextRenderer() = 0;

	virtual void SetColorRGB(uint32 color) = 0;
	virtual void FillRect(sint32 x, sint32 y, sint32 w, sint32 h) = 0;
	virtual void MultiFillRect(const vdrect32 *rects, uint32 n) = 0;

	virtual void AlphaFillRect(sint32 x, sint32 y, sint32 w, sint32 h, uint32 alphaColor) = 0;
	virtual void AlphaTriStrip(const vdfloat2 *pts, uint32 numPts, uint32 alphaColor) = 0;

	virtual void FillTriStripHDR(const vdfloat2 *pts, const vdfloat4 *colors, uint32 numPts, bool alphaBlend) {}
	virtual void FillTriStripHDR(const vdfloat2 *pts, const vdfloat4 *colors, const vdfloat2 *uv, uint32 numPts, bool alphaBlend, bool filter, VDDisplayImageView& brush) {}

	virtual void Blt(sint32 x, sint32 y, VDDisplayImageView& imageView) = 0;
	virtual void Blt(sint32 x, sint32 y, VDDisplayImageView& imageView, sint32 sx, sint32 sy, sint32 w, sint32 h) = 0;

	virtual void StretchBlt(sint32 dx, sint32 dy, sint32 dw, sint32 dh, VDDisplayImageView& imageView, sint32 sx, sint32 sy, sint32 sw, sint32 sh, const VDDisplayBltOptions& opts) = 0;

	enum BltMode {
		kBltMode_Normal,

		// Do multiple stencil blits from a bitmap. White pixels are converted to the current color,
		// while black pixels are transparent.
		kBltMode_Stencil,

		// Do multiple alpha blits from a grayscale bitmap, using the current color. The color channel used
		// for the blit is unspecified (may be blue).
		kBltMode_Gray,

		// Do multiple color blits from a bitmap with RGB alpha and using the current color. This is used
		// for rendering subpixel text.
		kBltMode_Color,

		// Do multiple color blits from a bitmap split into halves, the left half containing color for
		// a=0 and the right half for a=1, with interpolation. The result is multiplied by vertex color
		// and then blended with vertex alpha. This is used for (very) approximate gamma correction of text.
		kBltMode_Color2
	};

	virtual void MultiBlt(const VDDisplayBlt *blts, uint32 n, VDDisplayImageView& imageView, BltMode bltMode) = 0;

	// Draw a connected line strip.
	virtual void PolyLine(const vdpoint32 *points, uint32 numLines) = 0;
	virtual void PolyLineF(const vdfloat2 *points, uint32 numLines, bool antialiased) = 0;

	[[nodiscard]]
	virtual bool PushViewport(const vdrect32& r, sint32 x, sint32 y) = 0;
	virtual void PopViewport() = 0;

	virtual IVDDisplayRenderer *BeginSubRender(const vdrect32& r, VDDisplaySubRenderCache& cache) = 0;
	virtual void EndSubRender() = 0;
};

class VDDisplayImageView {
public:
	VDDisplayImageView();
	~VDDisplayImageView();

	bool IsDynamic() const { return mbDynamic; }
	const VDPixmap& GetImage() const { return mPixmap; }
	void SetImage();
	void SetImage(const VDPixmap& px, bool dynamic);
	void SetVirtualImage(int w, int h);

	void SetCachedImage(uint32 id, IVDRefUnknown *p);
	IVDRefUnknown *GetCachedImage(uint32 id) const {
		return mCaches[0].mId == id ? mCaches[0].mpCache
			: mCaches[1].mId == id ? mCaches[1].mpCache
			: (IVDRefUnknown *)NULL;
	}

	uint32 GetUniquenessCounter() const { return mUniquenessCounter; }
	const vdrect32 *GetDirtyList() const;
	uint32 GetDirtyListSize() const;

	void Invalidate();
	void Invalidate(const vdrect32 *rects, uint32 n);

protected:
	struct CachedImageInfo {
		uint32 mId;
		vdrefptr<IVDRefUnknown> mpCache;
	};

	CachedImageInfo mCaches[2];
	uint32 mUniquenessCounter;
	VDPixmap mPixmap;
	bool mbDynamic;

	vdfastvector<vdrect32> mDirtyRects;
};

#endif
