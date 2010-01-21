/****************************************************************************\
Datei  : Color.cpp
Projekt: Farbverwaltung
Inhalt : CColor Implementierung
Datum  : 10.01.1999
Autor  : Christian Rodemeyer
Hinweis: (C) 1999 by Christian Rodemeyer
         - Foley and Van Dam: "Fundamentals of Interactive Computer Graphics"  
         - MSDN: 'HLS Color Spaces'
         - MSDN: 'Converting Colors Between RGB and HLS' 
\****************************************************************************/

#include "StdAfx.h"
#include "Color.h"
#include <math.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CColor::CColor(COLORREF cr)
: m_bIsRGB(true), m_bIsHLS(false), m_colorref(cr)
{}

CColor::operator COLORREF() const
{
  const_cast<CColor*>(this)->ToRGB();
  return m_colorref;
}

void CColor::SetColor(COLORREF cr)
  {
  m_bIsRGB = true;
  m_bIsHLS = false;
  m_colorref = cr;
  }

// RGB

void CColor::SetRed(int red)
{
  ASSERT(0 <= red && red <= 255);
  ToRGB();
  m_colorref = RGB (red, GetGValue(m_colorref), GetBValue(m_colorref));
}

void CColor::SetGreen(int green)
{
  ASSERT(0 <= green && green <= 255);
  ToRGB();
  m_colorref = RGB (GetRValue(m_colorref), green, GetBValue(m_colorref));
}

void CColor::SetBlue(int blue)
{
  ASSERT(0 <= blue && blue <= 255);
  ToRGB();
  m_colorref = RGB (GetRValue(m_colorref), GetGValue(m_colorref), blue);
}

void CColor::SetRGB(int red, int green, int blue)
{
  ASSERT(0 <= red && red <= 255);
  ASSERT(0 <= green && green <= 255);
  ASSERT(0 <= blue && blue <= 255);
  m_colorref = RGB (red, green, blue);

  m_bIsHLS = false;
  m_bIsRGB = true;
}

int CColor::GetRed() const
{
  const_cast<CColor*>(this)->ToRGB();
  return GetRValue(m_colorref);
}

int CColor::GetGreen() const
{
  const_cast<CColor*>(this)->ToRGB();
  return GetGValue(m_colorref);
}

int CColor::GetBlue() const
{
  const_cast<CColor*>(this)->ToRGB();
  return GetBValue(m_colorref);
}

// HSL

void CColor::SetHue(double hue)
{
  ASSERT(hue >= 0.0 && hue <= 360.0);

  ToHLS();
  m_hue = hue;
}

void CColor::SetSaturation(double saturation)
{
  ASSERT(saturation >= 0.0 && saturation <= 1.0); // 0.0 ist undefiniert

  ToHLS();
  m_saturation = saturation;
}

void CColor::SetLuminance(double luminance)
{
  ASSERT(luminance >= 0.0 && luminance <= 1.0);

  ToHLS();
  m_luminance = luminance;
}

void CColor::SetHLS(double hue, double luminance, double saturation)
{
  ASSERT(hue >= 0.0 && hue <= 360.0);
  ASSERT(luminance >= 0.0 && luminance <= 1.0);
  ASSERT(saturation >= 0.0 && saturation <= 1.0); // 0.0 ist undefiniert

  m_hue = hue;
  m_luminance = luminance;
  m_saturation = saturation;
  m_bIsRGB = false;
  m_bIsHLS = true;
}

double CColor::GetHue() const
{
  const_cast<CColor*>(this)->ToHLS();
  return m_hue;
}

double CColor::GetSaturation() const
{
  const_cast<CColor*>(this)->ToHLS();
  return m_saturation;
}

double CColor::GetLuminance() const
{
  const_cast<CColor*>(this)->ToHLS();
  return m_luminance;
}

// see: http://en.wikipedia.org/wiki/HSL_color_space

void CColor::ToHLS() 
{
  if (m_bIsHLS)
    return;

  m_bIsHLS = true;   // well it will be soon ;)
  m_bIsRGB = false;

  //Convert rgb values to the range 0-1
  double R = ((double) GetRValue (m_colorref)) / 255.0;
  double G = ((double) GetGValue (m_colorref)) / 255.0;
  double B = ((double) GetBValue (m_colorref)) / 255.0;

  //Find min and max values of R,G,B
  double mincolor = min (R, min (G, B));
  double maxcolor = max (R, max (G, B));

  double colordiff = maxcolor - mincolor;

  //Special treatment for grey colors
  if (colordiff == 0.0)
    {
    m_hue = m_saturation = 0.0;
    m_luminance = mincolor;   // L = 1/2 (min + max) so in this case, min or max
    return;
    }

  m_luminance = (maxcolor + mincolor) / 2.0;   // L = 1/2 (min + max)

  if (m_luminance < 0.5)
    m_saturation = colordiff / (maxcolor + mincolor);
  else
    m_saturation = colordiff / (2.0 - (maxcolor + mincolor));
    
  if (R == maxcolor)
    m_hue = 60.0 * (G - B) / colordiff + 0.0;
  else if (G == maxcolor)
    m_hue = 60.0 * (B - R) / colordiff + 120.0;
  else
    m_hue = 60.0 * (R - G) / colordiff + 240.0;
      
  if (m_hue < 0.0)
    m_hue += 360.0;
  else if (m_hue > 360.0)
    m_hue -= 360.0;

  if (m_saturation > 1.0)
    m_saturation = 1.0;
  else if (m_saturation < 0.0)
    m_saturation = 0.0;
}

// see: http://en.wikipedia.org/wiki/HSL_color_space

void CColor::ToRGB() 
{

  if (m_bIsRGB)
    return;

  m_bIsRGB = true;    // well it will be soon ;)
  m_bIsHLS = false;


  if (m_saturation <= 0.0)
    {
      // round up
    m_colorref = RGB (m_luminance * 255 + 0.5, m_luminance * 255 + 0.5, m_luminance * 255 + 0.5);
    return;
    }

  double q;

  if (m_luminance < 0.5)
    q = m_luminance * (1.0 + m_saturation);
  else
    q = (m_luminance + m_saturation) - (m_luminance * m_saturation);

  double p = 2.0 * m_luminance - q;
  double hk = m_hue / 360.0;

  double tr = hk + 1.0 / 3.0;     // red
  double tg = hk;                 // green
  double tb = hk - 1.0 / 3.0;     // blue

  // ensure not < 0
  tr = (tr < 0.0) ? tr + 1.0 : tr;
  tg = (tg < 0.0) ? tg + 1.0 : tg;
  tb = (tb < 0.0) ? tb + 1.0 : tb;

  // ensure not > 1
  tr = (tr > 1.0) ? tr - 1.0 : tr;
  tg = (tg > 1.0) ? tg - 1.0 : tg;
  tb = (tb > 1.0) ? tb - 1.0 : tb;


  double r, g, b;

  r = ToRGB1 (tr, p, q) * 255;   
  g = ToRGB1 (tg, p, q) * 255;
  b = ToRGB1 (tb, p, q) * 255;

//  ASSERT (0 <= r && r <= 255);
//  ASSERT (0 <= g && g <= 255);
//  ASSERT (0 <= b && b <= 255);


  m_colorref = RGB (r + 0.5, g + 0.5, b + 0.5);

}


double CColor::ToRGB1 (double t, double p, double q)
  {
  if (t < (1.0 / 6.0))
    return p + ((q - p) * 6.0 * t);

  if (t < 0.5)
    return q;

  if (t < (2.0 / 3.0))
    return p + ((q - p) * 6.0 * (2.0 / 3.0 - t));

  return p;

  }

