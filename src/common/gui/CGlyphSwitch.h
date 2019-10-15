#pragma once

#include "vstcontrols.h"
#include "CScalableBitmap.h"

/*
** A switch which paints its background and draws a glyph over the front.
** When selected the value is the given value; when unselected the
** value is 0.
*/

class CGlyphSwitch : public VSTGUI::CControl
{
public:
   CGlyphSwitch( const VSTGUI::CRect &size,
                 VSTGUI::IControlListener *listener,
                 long tag,
                 int targetvalue );

   ~CGlyphSwitch() {
      if( glyph )
         glyph->forget();
   }

   // Use this later
   enum BackgroundStyle
   {
      DrawnBackground,
      SVGBackground
   };
   BackgroundStyle bgStyle = DrawnBackground;

   enum DrawState
   {
      Off, On,
      HoverOff, HoverOn,
      PressOff, PressOn,
      n_drawstates
   };
   DrawState currentState = Off;

   enum FGMode {
      None,
      Glyph,
      Text
   };
   FGMode fgMode = FGMode::None;

   enum BGMode {
      Fill,
      SVG
   };
   BGMode bgMode = BGMode::Fill;
   
   void setGlyphText(std::string t) {
      fgMode = Text;
      if( glyph )
         glyph->forget();
      glyph = nullptr;
      fgText = t;
   }

   void setGlyphTextFont( std::string fontName, int fontSize ) {
      fgFont = fontName;
      fgFontSize = fontSize;
   }
   
   void setGlyphBitmap(CScalableBitmap *b) {
      fgMode = Glyph;
      if( glyph )
         glyph->forget();
      glyph = b;
      glyph->remember();
   }

   void setBGBitmap(int which, CScalableBitmap *b) {
      bgMode = BGMode::SVG;
      if(bgBitmap[which] != nullptr)
         bgBitmap[which]->forget();
      bgBitmap[which] = b;
      bgBitmap[which]->remember();
   }

   void setFGColor(int which, VSTGUI::CColor c) {
      fgColor[which] = c;
   }
   void setBGColor(int which, VSTGUI::CColor c) {
      bgColor[which] = c;
   }
   
   virtual void setValue(float f) override;
   
   void setBackgroundColor(DrawState ds, VSTGUI::CColor c) { bgColor[ds] = c; };
   void setForegroundColor(DrawState ds, VSTGUI::CColor c) { fgColor[ds] = c; };

   virtual void draw( VSTGUI::CDrawContext *dc );

   virtual VSTGUI::CMouseEventResult onMouseDown( VSTGUI::CPoint &where,
                                                  const VSTGUI::CButtonState &buttons) override;
   virtual VSTGUI::CMouseEventResult onMouseUp( VSTGUI::CPoint &where,
                                                  const VSTGUI::CButtonState &buttons) override;
   virtual VSTGUI::CMouseEventResult onMouseEntered( VSTGUI::CPoint &where,
                                                     const VSTGUI::CButtonState &buttons) override;
   virtual VSTGUI::CMouseEventResult onMouseExited( VSTGUI::CPoint &where,
                                                     const VSTGUI::CButtonState &buttons) override;
   
private:
   CScalableBitmap *bgBitmap[n_drawstates];
   VSTGUI::CColor bgColor[n_drawstates], fgColor[n_drawstates];
   CScalableBitmap *glyph = nullptr;

   std::string fgText = "", fgFont="Lato";
   int fgFontSize=11;
   
   CLASS_METHODS( CGlyphSwitch, VSTGUI::CControl );
};
