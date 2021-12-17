// Created on: 2010-07-18
// Created by: Kirill GAVRILOV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <Image_PixMap.hxx>

#include <NCollection_AlignedAllocator.hxx>
#include <Standard_ProgramError.hxx>

#include <algorithm>

namespace
{
  //! Structure defining image pixel format description.
  struct Image_FormatInfo
  {
    const char*  Name;         //!< string representation
    int          Format;       //!< enumeration name
    unsigned int NbComponents; //!< number of components
    unsigned int PixelSize;    //!< bytes per pixel

    Image_FormatInfo (Image_Format theFormat, const char* theName, unsigned int theNbComponents, Standard_Size thePixelSize)
    : Name (theName), Format (theFormat), NbComponents (theNbComponents), PixelSize ((unsigned int )thePixelSize) {}

    Image_FormatInfo (Image_CompressedFormat theFormat, const char* theName, unsigned int theNbComponents, Standard_Size thePixelSize)
    : Name (theName), Format (theFormat), NbComponents (theNbComponents), PixelSize ((unsigned int )thePixelSize) {}
  };

  #define ImageFormatInfo(theName, theNbComponents, thePixelSize) \
    Image_FormatInfo(Image_Format_##theName, #theName, theNbComponents, thePixelSize)

  #define CompressedImageFormatInfo(theName, theNbComponents, thePixelSize) \
    Image_FormatInfo(Image_CompressedFormat_##theName, #theName, theNbComponents, thePixelSize)

  //! Table of image pixel formats.
  static const Image_FormatInfo Image_Table_ImageFormats[Image_CompressedFormat_NB] =
  {
    ImageFormatInfo(UNKNOWN, 0, 1),
    ImageFormatInfo(Gray,    1, 1),
    ImageFormatInfo(Alpha,   1, 1),
    ImageFormatInfo(RGB,     3, 3),
    ImageFormatInfo(BGR,     3, 3),
    ImageFormatInfo(RGB32,   3, 4),
    ImageFormatInfo(BGR32,   3, 4),
    ImageFormatInfo(RGBA,    4, 4),
    ImageFormatInfo(BGRA,    4, 4),
    ImageFormatInfo(GrayF,   1, sizeof(float)),
    ImageFormatInfo(AlphaF,  1, sizeof(float)),
    ImageFormatInfo(RGF,     2, sizeof(float) * 2),
    ImageFormatInfo(RGBF,    3, sizeof(float) * 3),
    ImageFormatInfo(BGRF,    3, sizeof(float) * 3),
    ImageFormatInfo(RGBAF,   4, sizeof(float) * 4),
    ImageFormatInfo(BGRAF,   4, sizeof(float) * 4),
    ImageFormatInfo(RGF_half,   2, sizeof(uint16_t) * 2),
    ImageFormatInfo(RGBAF_half, 4, sizeof(uint16_t) * 4),
    CompressedImageFormatInfo(RGB_S3TC_DXT1,  3, 1), // DXT1 uses circa half a byte per pixel (64 bits per 4x4 block)
    CompressedImageFormatInfo(RGBA_S3TC_DXT1, 4, 1),
    CompressedImageFormatInfo(RGBA_S3TC_DXT3, 4, 1), // DXT3/5 uses circa 1 byte per pixel (128 bits per 4x4 block)
    CompressedImageFormatInfo(RGBA_S3TC_DXT5, 4, 1)
  };
}

IMPLEMENT_STANDARD_RTTIEXT(Image_PixMap,Standard_Transient)

// =======================================================================
// function : DefaultAllocator
// purpose  :
// =======================================================================
const Handle(NCollection_BaseAllocator)& Image_PixMap::DefaultAllocator()
{
  static const Handle(NCollection_BaseAllocator) THE_ALLOC = new NCollection_AlignedAllocator (16);
  return THE_ALLOC;
}

// =======================================================================
// function : ImageFormatToString
// purpose  :
// =======================================================================
Standard_CString Image_PixMap::ImageFormatToString (Image_Format theFormat)
{
  return Image_Table_ImageFormats[theFormat].Name;
}

// =======================================================================
// function : ImageFormatToString
// purpose  :
// =======================================================================
Standard_CString Image_PixMap::ImageFormatToString (Image_CompressedFormat theFormat)
{
  return Image_Table_ImageFormats[theFormat].Name;
}

// =======================================================================
// function : Image_PixMap
// purpose  :
// =======================================================================
Image_PixMap::Image_PixMap()
: myImgFormat (Image_Format_Gray)
{
  //
}

// =======================================================================
// function : ~Image_PixMap
// purpose  :
// =======================================================================
Image_PixMap::~Image_PixMap()
{
  Clear();
}

// =======================================================================
// function : SizePixelBytes
// purpose  :
// =======================================================================
Standard_Size Image_PixMap::SizePixelBytes (const Image_Format thePixelFormat)
{
  return Image_Table_ImageFormats[thePixelFormat].PixelSize;
}

// =======================================================================
// function : SetFormat
// purpose  :
// =======================================================================
void Image_PixMap::SetFormat (Image_Format thePixelFormat)
{
  if (myImgFormat == thePixelFormat)
  {
    return;
  }

  if (!IsEmpty()
    && SizePixelBytes (myImgFormat) != SizePixelBytes (thePixelFormat))
  {
    throw Standard_ProgramError("Image_PixMap::SetFormat() - incompatible pixel format");
    return;
  }

  myImgFormat = thePixelFormat;
}

// =======================================================================
// function : InitWrapper
// purpose  :
// =======================================================================
bool Image_PixMap::InitWrapper (Image_Format        thePixelFormat,
                                Standard_Byte*      theDataPtr,
                                const Standard_Size theSizeX,
                                const Standard_Size theSizeY,
                                const Standard_Size theSizeRowBytes)
{
  Clear();
  myImgFormat = thePixelFormat;
  if ((theSizeX == 0) || (theSizeY == 0) || (theDataPtr == NULL))
  {
    return false;
  }

  Handle(NCollection_BaseAllocator) anEmptyAlloc;
  myData.Init (anEmptyAlloc, Image_PixMap::SizePixelBytes (thePixelFormat),
               theSizeX, theSizeY, theSizeRowBytes, theDataPtr);
  return true;
}

// =======================================================================
// function : InitTrash
// purpose  :
// =======================================================================
bool Image_PixMap::InitTrash (Image_Format        thePixelFormat,
                              const Standard_Size theSizeX,
                              const Standard_Size theSizeY,
                              const Standard_Size theSizeRowBytes)
{
  Clear();
  myImgFormat = thePixelFormat;
  if ((theSizeX == 0) || (theSizeY == 0))
  {
    return false;
  }

  // use argument only if it greater
  const Standard_Size aSizeRowBytes = std::max (theSizeRowBytes, theSizeX * SizePixelBytes (thePixelFormat));
  myData.Init (DefaultAllocator(), Image_PixMap::SizePixelBytes (thePixelFormat),
               theSizeX, theSizeY, aSizeRowBytes, NULL);
  return !myData.IsEmpty();
}

// =======================================================================
// function : InitZero
// purpose  :
// =======================================================================
bool Image_PixMap::InitZero (Image_Format        thePixelFormat,
                             const Standard_Size theSizeX,
                             const Standard_Size theSizeY,
                             const Standard_Size theSizeRowBytes,
                             const Standard_Byte theValue)
{
  if (!InitTrash (thePixelFormat, theSizeX, theSizeY, theSizeRowBytes))
  {
    return false;
  }
  memset (myData.ChangeData(), (int )theValue, SizeBytes());
  return true;
}

// =======================================================================
// function : InitCopy
// purpose  :
// =======================================================================
bool Image_PixMap::InitCopy (const Image_PixMap& theCopy)
{
  if (&theCopy == this)
  {
    // self-copying disallowed
    return false;
  }
  if (InitTrash (theCopy.myImgFormat, theCopy.SizeX(), theCopy.SizeY(), theCopy.SizeRowBytes()))
  {
    memcpy (myData.ChangeData(), theCopy.myData.Data(), theCopy.SizeBytes());
    return true;
  }
  return false;
}

// =======================================================================
// function : Clear
// purpose  :
// =======================================================================
void Image_PixMap::Clear()
{
  Handle(NCollection_BaseAllocator) anEmptyAlloc;
  myData.Init (anEmptyAlloc, Image_PixMap::SizePixelBytes (myImgFormat),
               0, 0, 0, NULL);
}

// =======================================================================
// function : PixelColor
// purpose  :
// =======================================================================
Quantity_ColorRGBA Image_PixMap::PixelColor (const Standard_Integer theX,
                                             const Standard_Integer theY,
                                             const Standard_Boolean theToLinearize) const
{
  if (IsEmpty()
   || theX < 0 || (Standard_Size )theX >= SizeX()
   || theY < 0 || (Standard_Size )theY >= SizeY())
  {
    return Quantity_ColorRGBA (0.0f, 0.0f, 0.0f, 0.0f); // transparent
  }

  switch (myImgFormat)
  {
    case Image_Format_GrayF:
    {
      const Standard_ShortReal& aPixel = Value<Standard_ShortReal> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel, aPixel, aPixel, 1.0f)); // opaque
    }
    case Image_Format_AlphaF:
    {
      const Standard_ShortReal& aPixel = Value<Standard_ShortReal> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (1.0f, 1.0f, 1.0f, aPixel));
    }
    case Image_Format_RGF:
    {
      const Image_ColorRGF& aPixel = Value<Image_ColorRGF> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel.r(), aPixel.g(), 0.0f, 1.0f));
    }
    case Image_Format_RGBAF:
    {
      const Image_ColorRGBAF& aPixel = Value<Image_ColorRGBAF> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel.r(), aPixel.g(), aPixel.b(), aPixel.a()));
    }
    case Image_Format_BGRAF:
    {    
      const Image_ColorBGRAF& aPixel = Value<Image_ColorBGRAF> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel.r(), aPixel.g(), aPixel.b(), aPixel.a()));
    }
    case Image_Format_RGBF:
    {
      const Image_ColorRGBF& aPixel = Value<Image_ColorRGBF> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel.r(), aPixel.g(), aPixel.b(), 1.0f)); // opaque
    }
    case Image_Format_BGRF:
    {
      const Image_ColorBGRF& aPixel = Value<Image_ColorBGRF> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (aPixel.r(), aPixel.g(), aPixel.b(), 1.0f)); // opaque
    }
    case Image_Format_RGF_half:
    {
      const NCollection_Vec2<uint16_t>& aPixel = Value<NCollection_Vec2<uint16_t>> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (ConvertFromHalfFloat (aPixel.x()), ConvertFromHalfFloat (aPixel.y()), 0.0f, 1.0f));
    }
    case Image_Format_RGBAF_half:
    {
      const NCollection_Vec4<uint16_t>& aPixel = Value<NCollection_Vec4<uint16_t>> (theY, theX);
      return Quantity_ColorRGBA (NCollection_Vec4<float> (ConvertFromHalfFloat (aPixel.r()), ConvertFromHalfFloat (aPixel.g()),
                                                          ConvertFromHalfFloat (aPixel.b()), ConvertFromHalfFloat (aPixel.a())));
    }
    case Image_Format_RGBA:
    {
      const Image_ColorRGBA& aPixel = Value<Image_ColorRGBA> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f),
                                 float(aPixel.a()) / 255.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, float(aPixel.a()) / 255.0f);
    }
    case Image_Format_BGRA:
    {
      const Image_ColorBGRA& aPixel = Value<Image_ColorBGRA> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f),
                                 float(aPixel.a()) / 255.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, float(aPixel.a()) / 255.0f);
    }
    case Image_Format_RGB32:
    {
      const Image_ColorRGB32& aPixel = Value<Image_ColorRGB32> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f), 1.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, 1.0f);
    }
    case Image_Format_BGR32:
    {
      const Image_ColorBGR32& aPixel = Value<Image_ColorBGR32> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f), 1.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, 1.0f);
    }
    case Image_Format_RGB:
    {
      const Image_ColorRGB& aPixel = Value<Image_ColorRGB> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f), 1.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, 1.0f);
    }
    case Image_Format_BGR:
    {
      const Image_ColorBGR& aPixel = Value<Image_ColorBGR> (theY, theX);
      return theToLinearize
           ? Quantity_ColorRGBA (Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.r()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.g()) / 255.0f),
                                 Quantity_Color::Convert_sRGB_To_LinearRGB (float(aPixel.b()) / 255.0f), 1.0f)
           : Quantity_ColorRGBA (float(aPixel.r()) / 255.0f, float(aPixel.g()) / 255.0f, float(aPixel.b()) / 255.0f, 1.0f);
    }
    case Image_Format_Gray:
    {
      const Standard_Byte& aPixel = Value<Standard_Byte> (theY, theX);
      return Quantity_ColorRGBA (float(aPixel) / 255.0f, float(aPixel) / 255.0f, float(aPixel) / 255.0f, 1.0f); // opaque
    }
    case Image_Format_Alpha:
    {
      const Standard_Byte& aPixel = Value<Standard_Byte> (theY, theX);
      return Quantity_ColorRGBA (1.0f, 1.0f, 1.0f, float(aPixel) / 255.0f);
    }
    case Image_Format_UNKNOWN:
    {
      break;
    }
  }

  // unsupported image type
  return Quantity_ColorRGBA (0.0f, 0.0f, 0.0f, 0.0f); // transparent
}

// =======================================================================
// function : SetPixelColor
// purpose  :
// =======================================================================
void Image_PixMap::SetPixelColor (const Standard_Integer theX,
                                  const Standard_Integer theY,
                                  const Quantity_ColorRGBA& theColor,
                                  const Standard_Boolean theToDeLinearize)
{
  if (IsEmpty()
   || theX < 0 || Standard_Size(theX) >= SizeX()
   || theY < 0 || Standard_Size(theY) >= SizeY())
  {
    return;
  }

  const NCollection_Vec4<float>& aColor = theColor;
  switch (myImgFormat)
  {
    case Image_Format_GrayF:
    {
      ChangeValue<Standard_ShortReal> (theY, theX) = aColor.r();
      return;
    }
    case Image_Format_AlphaF:
    {
      ChangeValue<Standard_ShortReal> (theY, theX) = aColor.a();
      return;
    }
    case Image_Format_RGF:
    {
      Image_ColorRGF& aPixel = ChangeValue<Image_ColorRGF> (theY, theX);
      aPixel.r() = aColor.r();
      aPixel.g() = aColor.g();
      return;
    }
    case Image_Format_RGBAF:
    {
      Image_ColorRGBAF& aPixel = ChangeValue<Image_ColorRGBAF> (theY, theX);
      aPixel.r() = aColor.r();
      aPixel.g() = aColor.g();
      aPixel.b() = aColor.b();
      aPixel.a() = aColor.a();
      return;
    }
    case Image_Format_BGRAF:
    {
      Image_ColorBGRAF& aPixel = ChangeValue<Image_ColorBGRAF> (theY, theX);
      aPixel.r() = aColor.r();
      aPixel.g() = aColor.g();
      aPixel.b() = aColor.b();
      aPixel.a() = aColor.a();
      return;
    }
    case Image_Format_RGBF:
    {
      Image_ColorRGBF& aPixel = ChangeValue<Image_ColorRGBF> (theY, theX);
      aPixel.r() = aColor.r();
      aPixel.g() = aColor.g();
      aPixel.b() = aColor.b();
      return;
    }
    case Image_Format_BGRF:
    {
      Image_ColorBGRF& aPixel = ChangeValue<Image_ColorBGRF> (theY, theX);
      aPixel.r() = aColor.r();
      aPixel.g() = aColor.g();
      aPixel.b() = aColor.b();
      return;
    }
    case Image_Format_RGF_half:
    {
      NCollection_Vec2<uint16_t>& aPixel = ChangeValue<NCollection_Vec2<uint16_t>> (theY, theX);
      aPixel.x() = ConvertToHalfFloat (aColor.r());
      aPixel.y() = ConvertToHalfFloat (aColor.g());
      return;
    }
    case Image_Format_RGBAF_half:
    {
      NCollection_Vec4<uint16_t>& aPixel = ChangeValue<NCollection_Vec4<uint16_t>> (theY, theX);
      aPixel.r() = ConvertToHalfFloat (aColor.r());
      aPixel.g() = ConvertToHalfFloat (aColor.g());
      aPixel.b() = ConvertToHalfFloat (aColor.b());
      aPixel.a() = ConvertToHalfFloat (aColor.a());
      return;
    }
    case Image_Format_RGBA:
    {
      Image_ColorRGBA& aPixel = ChangeValue<Image_ColorRGBA> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r() = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g() = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b() = Standard_Byte(aColor.b() * 255.0f);
      }
      aPixel.a() = Standard_Byte(aColor.a() * 255.0f);
      return;
    }
    case Image_Format_BGRA:
    {
      Image_ColorBGRA& aPixel = ChangeValue<Image_ColorBGRA> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r() = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g() = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b() = Standard_Byte(aColor.b() * 255.0f);
      }
      aPixel.a() = Standard_Byte(aColor.a() * 255.0f);
      return;
    }
    case Image_Format_RGB32:
    {
      Image_ColorRGB32& aPixel = ChangeValue<Image_ColorRGB32> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r()  = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g()  = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b()  = Standard_Byte(aColor.b() * 255.0f);
      }
      aPixel.a_() = 255;
      return;
    }
    case Image_Format_BGR32:
    {
      Image_ColorBGR32& aPixel = ChangeValue<Image_ColorBGR32> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b()  = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r()  = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g()  = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b()  = Standard_Byte(aColor.b() * 255.0f);
      }
      aPixel.a_() = 255;
      return;
    }
    case Image_Format_RGB:
    {
      Image_ColorRGB& aPixel = ChangeValue<Image_ColorRGB> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r() = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g() = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b() = Standard_Byte(aColor.b() * 255.0f);
      }
      return;
    }
    case Image_Format_BGR:
    {
      Image_ColorBGR& aPixel = ChangeValue<Image_ColorBGR> (theY, theX);
      if (theToDeLinearize)
      {
        aPixel.r() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.r()) * 255.0f);
        aPixel.g() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.g()) * 255.0f);
        aPixel.b() = Standard_Byte(Quantity_Color::Convert_LinearRGB_To_sRGB (aColor.b()) * 255.0f);
      }
      else
      {
        aPixel.r() = Standard_Byte(aColor.r() * 255.0f);
        aPixel.g() = Standard_Byte(aColor.g() * 255.0f);
        aPixel.b() = Standard_Byte(aColor.b() * 255.0f);
      }
      return;
    }
    case Image_Format_Gray:
    {
      ChangeValue<Standard_Byte> (theY, theX) = Standard_Byte(aColor.r() * 255.0f);
      return;
    }
    case Image_Format_Alpha:
    {
      ChangeValue<Standard_Byte> (theY, theX) = Standard_Byte(aColor.a() * 255.0f);
      return;
    }
    case Image_Format_UNKNOWN:
    {
      return;
    }
  }
}

// =======================================================================
// function : SwapRgbaBgra
// purpose  :
// =======================================================================
bool Image_PixMap::SwapRgbaBgra (Image_PixMap& theImage)
{
  switch (theImage.Format())
  {
    case Image_Format_BGR32:
    case Image_Format_RGB32:
    case Image_Format_BGRA:
    case Image_Format_RGBA:
    {
      const bool toResetAlpha = theImage.Format() == Image_Format_BGR32
                             || theImage.Format() == Image_Format_RGB32;
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          Image_ColorRGBA& aPixel     = theImage.ChangeValue<Image_ColorRGBA> (aRow, aCol);
          Image_ColorBGRA  aPixelCopy = theImage.Value      <Image_ColorBGRA> (aRow, aCol);
          aPixel.r() = aPixelCopy.r();
          aPixel.g() = aPixelCopy.g();
          aPixel.b() = aPixelCopy.b();
          if (toResetAlpha)
          {
            aPixel.a() = 255;
          }
        }
      }
      return true;
    }
    case Image_Format_BGR:
    case Image_Format_RGB:
    {
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          Image_ColorRGB& aPixel     = theImage.ChangeValue<Image_ColorRGB> (aRow, aCol);
          Image_ColorBGR  aPixelCopy = theImage.Value      <Image_ColorBGR> (aRow, aCol);
          aPixel.r() = aPixelCopy.r();
          aPixel.g() = aPixelCopy.g();
          aPixel.b() = aPixelCopy.b();
        }
      }
      return true;
    }
    case Image_Format_BGRF:
    case Image_Format_RGBF:
    case Image_Format_BGRAF:
    case Image_Format_RGBAF:
    {
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          Image_ColorRGBF& aPixel     = theImage.ChangeValue<Image_ColorRGBF> (aRow, aCol);
          Image_ColorBGRF  aPixelCopy = theImage.Value      <Image_ColorBGRF> (aRow, aCol);
          aPixel.r() = aPixelCopy.r();
          aPixel.g() = aPixelCopy.g();
          aPixel.b() = aPixelCopy.b();
        }
      }
      return true;
    }
    default: return false;
  }
}

// =======================================================================
// function : ToBlackWhite
// purpose  :
// =======================================================================
void Image_PixMap::ToBlackWhite (Image_PixMap& theImage)
{
  switch (theImage.Format())
  {
    case Image_Format_Gray:
    case Image_Format_Alpha:
    {
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          unsigned char& aPixel = theImage.ChangeValue<unsigned char> (aRow, aCol);
          if (aPixel != 0)
          {
            aPixel = 255;
          }
        }
      }
      break;
    }
    case Image_Format_RGB:
    case Image_Format_BGR:
    case Image_Format_RGB32:
    case Image_Format_BGR32:
    case Image_Format_RGBA:
    case Image_Format_BGRA:
    {
      const NCollection_Vec3<unsigned char> aWhite24 (255, 255, 255);
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          NCollection_Vec3<unsigned char>& aPixel = theImage.ChangeValue< NCollection_Vec3<unsigned char> > (aRow, aCol);
          if (aPixel[0] != 0
           || aPixel[1] != 0
           || aPixel[2] != 0)
          {
            aPixel = aWhite24;
          }
        }
      }
      break;
    }
    default:
    {
      const Quantity_ColorRGBA aWhiteRgba (1.0f, 1.0f, 1.0f, 1.0f);
      for (Standard_Size aRow = 0; aRow < theImage.SizeY(); ++aRow)
      {
        for (Standard_Size aCol = 0; aCol < theImage.SizeX(); ++aCol)
        {
          const Quantity_ColorRGBA       aPixelRgba = theImage.PixelColor (Standard_Integer(aCol), Standard_Integer(aRow));
          const NCollection_Vec4<float>& aPixel     = aPixelRgba;
          if (aPixel[0] != 0.0f
           || aPixel[1] != 0.0f
           || aPixel[2] != 0.0f)
          {
            theImage.SetPixelColor (int(aCol), int(aRow), aWhiteRgba);
          }
        }
      }
      break;
    }
  }
}

// =======================================================================
// function : InitCopy
// purpose  :
// =======================================================================
bool Image_PixMap::FlipY (Image_PixMap& theImage)
{
  if (theImage.IsEmpty()
   || theImage.SizeX() == 0
   || theImage.SizeY() == 0)
  {
    return false;
  }

  NCollection_Buffer aTmp (NCollection_BaseAllocator::CommonBaseAllocator());
  const size_t aRowSize = theImage.SizeRowBytes();
  if (!aTmp.Allocate (aRowSize))
  {
    return false;
  }

  // for odd height middle row should be left as is
  Standard_Size aNbRowsHalf = theImage.SizeY() / 2;
  for (Standard_Size aRowT = 0, aRowB = theImage.SizeY() - 1; aRowT < aNbRowsHalf; ++aRowT, --aRowB)
  {
    Standard_Byte* aTop = theImage.ChangeRow (aRowT);
    Standard_Byte* aBot = theImage.ChangeRow (aRowB);
    memcpy (aTmp.ChangeData(), aTop, aRowSize);
    memcpy (aTop, aBot, aRowSize);
    memcpy (aBot, aTmp.Data(), aRowSize);
  }
  return true;
}
