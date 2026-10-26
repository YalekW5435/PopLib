#pragma once

#include "font.hpp"
#include "color.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace PopLib
{

class Graphics;
class MemoryImage;

enum class RenderMode
{
	Normal,		 // Standard filled vector text
	OutlineFill, // Filled text with an outer border
	OutlineOnly	 // Hollow text (stroke only, interior completely transparent)
};

struct Glyph
{
	int mWidth{0};
	int mHeight{0};
	int mXOffset{0};
	int mYOffset{0};
	int mXAdvance{0};
	MemoryImage *mImage{nullptr};
};

class PopFont : public Font
{
  public:
	PopFont();
	PopFont(const std::string &theTtfPath, float theFontSize);
	virtual ~PopFont();

	bool LoadTTF(const std::string &theTtfPath, float theFontSize);

	// Font interface s for PopLib / SexyAppFramework compatibility
	virtual int StringWidth(const std::string &theString) ;
	virtual int StringWidth(const std::u32string &theString) ;
	virtual int GetHeight() ;
	virtual int GetAscent() ;
	virtual int GetDescent() ;

	virtual void DrawString(Graphics *g, int theX, int theY, const PopString &theString, const Color &theColor,
							const Rect &theClipRect);

	virtual Font *Duplicate() ;

	// Outline & Render Mode Configuration
	void SetRenderMode(RenderMode theMode)
	{
		mRenderMode = theMode;
	}
	void SetOutlineThickness(int theThickness)
	{
		mOutlineThickness = theThickness;
	}
	void SetOutlineColor(const Color &theColor)
	{
		mOutlineColor = theColor;
	}

  private:
	Glyph *GetOrGenerateGlyph(char32_t theCodepoint);
	MemoryImage *CreateOutlineGlyphImage(MemoryImage *theSrcImage, int theThickness, bool theHollow);

	std::vector<uint8_t> mFontBuffer;
	void *mFontInfo; // Opaque pointer to stbtt_fontinfo
	float mFontSize;
	float mScale;
	int mAscent;
	int mDescent;
	int mLineGap;

	RenderMode mRenderMode;
	int mOutlineThickness;
	Color mOutlineColor;

	std::unordered_map<char32_t, Glyph> mGlyphCache;
};

}