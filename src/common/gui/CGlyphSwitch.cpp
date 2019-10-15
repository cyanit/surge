#include <iostream>
#include "CGlyphSwitch.h"


CGlyphSwitch::CGlyphSwitch( const VSTGUI::CRect &size,
                            VSTGUI::IControlListener *listener,
                            long tag,
                            int targetvalue )
   : VSTGUI::CControl( size, listener, tag )
{
   for( auto i=0; i<n_drawstates; ++i )
      bgBitmap[i] = nullptr;

   fgColor[On] = fgColor[HoverOn] = fgColor[PressOn] = VSTGUI::kWhiteCColor;
   bgColor[On] = bgColor[HoverOn] = bgColor[PressOn] = VSTGUI::kBlackCColor;

   fgColor[Off] = fgColor[HoverOff] = fgColor[PressOff] = VSTGUI::kBlackCColor;
   bgColor[Off] = bgColor[HoverOff] = bgColor[PressOff] = VSTGUI::kWhiteCColor;
}

void CGlyphSwitch::setValue(float f)
{
   value = f;
   if( value > 0 )
      currentState = On;
   else
      currentState = Off;
   invalid();
}

void CGlyphSwitch::draw(VSTGUI::CDrawContext *dc)
{
   auto size = getViewSize();

   switch( bgMode )
   {
   case Fill:
   {
      dc->setFillColor(bgColor[currentState]);
      dc->drawRect(size, VSTGUI::kDrawFilled);
   }
   break;
   case SVG:
   {
      // FIXME that painful waterfall for nulls. For now assuem they are there
      auto b = bgBitmap[currentState];
      if( b != nullptr )
      {
         VSTGUI::CPoint where(0,0);
         b->draw(dc, size, where, 0xff );
      }
      else
      {
         dc->setFillColor(VSTGUI::kRedCColor);
         dc->drawRect(size, VSTGUI::kDrawFilled);
      }
   }
   }

   switch( fgMode )
   {
   case Glyph:
   {
      if( glyph != nullptr )
      {
         glyph->updateWithGlyphColor(fgColor[currentState]);
         VSTGUI::CPoint where(0,0);
         glyph->draw(dc, size, where, 0xff );
      }
   }
   break;
   case Text:
   {
      auto stringR = getViewSize(); 
      // dc->setFontColor(fgColor);
      dc->setFontColor(fgColor[currentState]);
      
      VSTGUI::SharedPointer<VSTGUI::CFontDesc> labelFont = new VSTGUI::CFontDesc(fgFont.c_str(), fgFontSize);
      dc->setFont(labelFont);
      
      dc->drawString(fgText.c_str(), stringR, VSTGUI::kCenterText, true);
   }
   break;
   case None:
   {
      // Obviously do nothing
   }
   break;
   };

}

VSTGUI::CMouseEventResult CGlyphSwitch::onMouseDown (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   if( value > 0.5 )
      currentState = PressOn;
   else
      currentState = PressOff;
   invalid();
   return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult CGlyphSwitch::onMouseUp (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   if( value > 0.5 )
   {
      value = 0;
      currentState = HoverOff;
   }
   else
   {
      value = 1;
      currentState = HoverOn;
   }
   if( listener )
      listener->valueChanged(this);
   
   invalid();
   return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult CGlyphSwitch::onMouseEntered (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   if( value > 0 )
      currentState = HoverOn;
   else
      currentState = HoverOff;
   invalid();
   return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult CGlyphSwitch::onMouseExited (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   if( value > 0 )
      currentState = On;
   else
      currentState = Off;
   
   invalid();
   return VSTGUI::kMouseEventHandled;
}
