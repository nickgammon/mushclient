/****************************************************************************\
Datei  : Color.h
Projekt: Farbverwaltung
Inhalt : CColor Deklaration
Datum  : 10.01.1999
Autor  : Christian Rodemeyer
Hinweis: (C) 1999 by Christian Rodemeyer
\****************************************************************************/
#pragma once

#ifndef __COLOR_H                                               
#define __COLOR_H

/****************************************************************************\
 CColor: Repräsentiert einen Farbwert
\****************************************************************************/
class CColor
{
public:

  // COLORREF Konvertierung
  // ----------------------
  CColor(COLORREF cr = 0);
  operator COLORREF() const;

  // RGB - Routinen
  // --------------
  void SetRed(int red);     // 0..255  
  void SetGreen(int green); // 0..255
  void SetBlue(int blue);   // 0..255
  void SetRGB(int red, int green, int blue);
  void SetColor(COLORREF cr = 0);

  int GetRed() const;   // 0..255
  int GetGreen() const; // 0..255
  int GetBlue() const;  // 0..255

  // HLS - Routinen
  // --------------
  void SetHue(double hue);               // 0.0 .. 360.0
  void SetLuminance(double luminance);   // 0.0 .. 1.0
  void SetSaturation(double saturation); // 0.0 .. 1.0
  void SetHLS(double hue, double luminance, double saturation);

  double GetHue() const;        // 0.0 .. 360.0
  double GetLuminance() const;  // 0.0 .. 1.0
  double GetSaturation() const; // 0.0 .. 1.0


private:

  // Konvertierung
  // -------------
  void ToRGB(); 
  void ToHLS(); 

  /*
  static unsigned char ToRGB1(double rm1, double rm2, double rh);

  */

  static double ToRGB1(double t, double p, double q);

  COLORREF      m_colorref;

  double m_hue;         // 0.0 .. 360.0  // Winkel
  double m_saturation;  // 0.0 .. 1.0    // Prozent
  double m_luminance;   // 0.0 .. 1.0    // Prozent

  // Flag for lazy evaluation
  bool m_bIsRGB; 
  bool m_bIsHLS; 

};

#endif

