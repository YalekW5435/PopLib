#include <fstream>
#include <algorithm>
#include <cstring>
#include "PopFont.hpp"
#include "graphics.hpp"
#include "MemoryImage.hpp"

#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_truetype/stb_truetype.h"



using namespace PopLib;
 

PopFont::PopFont()
	: mFontInfo(nullptr), mFontSize(0.0f), mScale(0.0f), mAscent(0), mDescent(0), mLineGap(0),
	  mRenderMode(RenderMode::Normal), mOutlineThickness(1), mOutlineColor(Color::Black)
{
}

PopFont::PopFont(const std::string &theTtfPath, float theFontSize) : PopFont()
{
	LoadTTF(theTtfPath, theFontSize);
}

PopFont::~PopFont()
{
	for (auto &pair : mGlyphCache)
	{
		delete pair.second.mImage;
	}
	mGlyphCache.clear();

	if (mFontInfo)
	{
		delete static_cast<stbtt_fontinfo *>(mFontInfo);
		mFontInfo = nullptr;
	}
}

bool PopFont::LoadTTF(const std::string &theTtfPath, float theFontSize)
{
	std::ifstream file(theTtfPath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return false;

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	mFontBuffer.resize(size);
	if (!file.read(reinterpret_cast<char *>(mFontBuffer.data()), size))
		return false;

	stbtt_fontinfo *info = new stbtt_fontinfo();
	if (!stbtt_InitFont(info, mFontBuffer.data(), 0))
	{
		delete info;
		return false;
	}

	mFontInfo = info;
	mFontSize = theFontSize;
	mScale = stbtt_ScaleForPixelHeight(info, mFontSize);

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

	mAscent = static_cast<int>(ascent * mScale);
	mDescent = static_cast<int>(descent * mScale);
	mLineGap = static_cast<int>(lineGap * mScale);

	return true;
}

Glyph *PopFont::GetOrGenerateGlyph(char32_t theCodepoint)
{
	auto it = mGlyphCache.find(theCodepoint);
	if (it != mGlyphCache.end())
		return &it->second;

	if (!mFontInfo)
		return nullptr;

	stbtt_fontinfo *info = static_cast<stbtt_fontinfo *>(mFontInfo);

	int advanceWidth, leftSideBearing;
	stbtt_GetCodepointHMetrics(info, theCodepoint, &advanceWidth, &leftSideBearing);

	int width, height, xOff, yOff;
	unsigned char *bitmap = stbtt_GetCodepointBitmap(info, 0, mScale, theCodepoint, &width, &height, &xOff, &yOff);

	Glyph glyph;
	glyph.mWidth = width;
	glyph.mHeight = height;
	glyph.mXOffset = xOff;
	glyph.mYOffset = yOff;
	glyph.mXAdvance = static_cast<int>(advanceWidth * mScale);

	if (width > 0 && height > 0 && bitmap != nullptr)
	{
		MemoryImage *img = new MemoryImage();
		img->Create(width, height);
		uint32_t *bits = img->GetBits();

		for (int i = 0; i < width * height; ++i)
		{
			uint8_t alpha;
			
			auto it = mGlyphCache.find(theCodepoint);
			if (it != mGlyphCache.end())
				return &it->second;

			if (!mFontInfo)
				return nullptr;

			stbtt_fontinfo *info = static_cast<stbtt_fontinfo *>(mFontInfo);

			int advanceWidth, leftSideBearing;
			stbtt_GetCodepointHMetrics(info, theCodepoint, &advanceWidth, &leftSideBearing);

			int width, height, xOff, yOff;
			unsigned char *bitmap =
				stbtt_GetCodepointBitmap(info, 0, mScale, theCodepoint, &width, &height, &xOff, &yOff);

			Glyph glyph;
			glyph.mWidth = width;
			glyph.mHeight = height;
			glyph.mXOffset = xOff;
			glyph.mYOffset = yOff;
			glyph.mXAdvance = static_cast<int>(advanceWidth * mScale);

			if (width > 0 && height > 0 && bitmap != nullptr)
			{
				MemoryImage *img = new MemoryImage();
				img->Create(width, height);
				img->mHasAlpha = true; // Tell framework this texture uses alpha blending

				uint32_t *bits = img->GetBits();

				for (int i = 0; i < width * height; ++i)
				{
					uint8_t a = bitmap[i]; // Coverage byte (0 to 255)

					// PopCap 32-bit ARGB format: (A << 24) | (R << 16) | (G << 8) | B
					// Premultiply RGB by Alpha (for white text: R=A, G=A, B=A)
					bits[i] = ((uint32_t)a << 24) | ((uint32_t)a << 16) | ((uint32_t)a << 8) | (uint32_t)a;
				}

				img->BitsChanged();
				glyph.mImage = img;
				stbtt_FreeBitmap(bitmap, nullptr);
			}

			mGlyphCache[theCodepoint] = glyph;
			return &mGlyphCache[theCodepoint];
		}
	}
}
MemoryImage *PopFont::CreateOutlineGlyphImage(MemoryImage *theSrcImage, int theThickness, bool theHollow)
{
	if (!theSrcImage)
		return nullptr;

	int srcW = theSrcImage->GetWidth();
	int srcH = theSrcImage->GetHeight();
	uint32_t *srcBits = theSrcImage->GetBits();

	int pad = theThickness;
	int dstW = srcW + (pad * 2);
	int dstH = srcH + (pad * 2);

	MemoryImage *dstImage = new MemoryImage();
	dstImage->Create(dstW, dstH);
	uint32_t *dstBits = dstImage->GetBits();
	std::fill(dstBits, dstBits + (dstW * dstH), 0);

	auto IsSrcSolid = [&](int x, int y) -> bool {
		if (x < 0 || x >= srcW || y < 0 || y >= srcH)
			return false;
		return ((srcBits[y * srcW + x] >> 24) & 0xFF) > 32;
	};

	for (int y = 0; y < dstH; ++y)
	{
		for (int x = 0; x < dstW; ++x)
		{
			int srcX = x - pad;
			int srcY = y - pad;

			bool isSolidCenter = IsSrcSolid(srcX, srcY);

			// Check outer ring within search radius
			bool isNearBoundary = false;
			for (int dy = -theThickness; dy <= theThickness && !isNearBoundary; ++dy)
			{
				for (int dx = -theThickness; dx <= theThickness; ++dx)
				{
					if (dx == 0 && dy == 0)
						continue;
					if (IsSrcSolid(srcX + dx, srcY + dy))
					{
						isNearBoundary = true;
						break;
					}
				}
			}

			if (theHollow)
			{
				// Pure hollow stroke: keep stroke region, zero out central body
				if (isNearBoundary && !isSolidCenter)
					dstBits[y * dstW + x] = 0xFFFFFFFF;
			}
			else
			{
				// Filled outline pass
				if (isSolidCenter || isNearBoundary)
					dstBits[y * dstW + x] = 0xFFFFFFFF;
			}
		}
	}

	dstImage->BitsChanged();
	return dstImage;
}

int PopFont::StringWidth(const std::string &theString)
{
	int totalWidth = 0;
	for (char c : theString)
	{
		Glyph *g = GetOrGenerateGlyph(static_cast<char32_t>(c));
		if (g)
			totalWidth += g->mXAdvance;
	}
	return totalWidth;
}

int PopFont::StringWidth(const std::u32string &theString)
{
	int totalWidth = 0;
	for (char32_t c : theString)
	{
		Glyph *g = GetOrGenerateGlyph(c);
		if (g)
			totalWidth += g->mXAdvance;
	}
	return totalWidth;
}

int PopFont::GetHeight()
{
	return mAscent - mDescent;
}
int PopFont::GetAscent()
{
	return mAscent;
}
int PopFont::GetDescent()
{
	return std::abs(mDescent);
}


void PopFont::DrawString(Graphics *g, int theX, int theY, const PopString &theString, const Color &theColor,
						 const Rect &theClipRect)
{
	int curX = theX;

	for (char32_t c : theString)
	{
		Glyph *glyph = GetOrGenerateGlyph(c);
		if (!glyph || !glyph->mImage)
		{
			if (glyph)
				curX += glyph->mXAdvance;
			continue;
		}

		int drawX = curX + glyph->mXOffset;
		int drawY = theY + mAscent + glyph->mYOffset;

		if (mRenderMode == RenderMode::OutlineOnly)
		{
			// Pure outline mode: render contour texture without central fill
			MemoryImage *outlineImg = CreateOutlineGlyphImage(glyph->mImage, mOutlineThickness, true);
			if (outlineImg)
			{
				g->SetColor(mOutlineColor);
				g->DrawImage(outlineImg, drawX - mOutlineThickness, drawY - mOutlineThickness);
				delete outlineImg;
			}
		}
		else if (mRenderMode == RenderMode::OutlineFill)
		{
			// Outer stroke pass
			MemoryImage *outlineImg = CreateOutlineGlyphImage(glyph->mImage, mOutlineThickness, false);
			if (outlineImg)
			{
				g->SetColor(mOutlineColor);
				g->DrawImage(outlineImg, drawX - mOutlineThickness, drawY - mOutlineThickness);
				delete outlineImg;
			}

			// Inner core pass
			g->SetColor(theColor);
			g->DrawImage(glyph->mImage, drawX, drawY);
		}
		else
		{
			// Normal filled mode
			g->SetColor(theColor);
			g->DrawImage(glyph->mImage, drawX, drawY);
		}

		curX += glyph->mXAdvance;
	}
}

Font *PopFont::Duplicate()
{
	PopFont *fontCopy = new PopFont();
	fontCopy->mFontBuffer = this->mFontBuffer;
	fontCopy->mFontSize = this->mFontSize;
	fontCopy->mScale = this->mScale;
	fontCopy->mAscent = this->mAscent;
	fontCopy->mDescent = this->mDescent;
	fontCopy->mLineGap = this->mLineGap;
	fontCopy->mRenderMode = this->mRenderMode;
	fontCopy->mOutlineThickness = this->mOutlineThickness;
	fontCopy->mOutlineColor = this->mOutlineColor;

	if (!this->mFontBuffer.empty())
	{
		stbtt_fontinfo *info = new stbtt_fontinfo();
		stbtt_InitFont(info, fontCopy->mFontBuffer.data(), 0);
		fontCopy->mFontInfo = info;
	}

	return fontCopy;
}
