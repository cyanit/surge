#include <iostream>
#include "CGlyphSwitch.h"


CGlyphSwitch::CGlyphSwitch( const VSTGUI::CRect &size,
                            VSTGUI::IControlListener *listener,
                            long tag,
                            int targetvalue )
   : VSTGUI::CControl( size, listener, tag )
{
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
   auto fgColor = VSTGUI::kRedCColor;
   
   switch( currentState )
   {
   case On:
      dc->setFillColor(VSTGUI::kGreenCColor);
      break;
   case Off:
      dc->setFillColor(VSTGUI::kRedCColor);
      fgColor = VSTGUI::kWhiteCColor;
      break;
   case HoverOn:
      dc->setFillColor(VSTGUI::kYellowCColor);
      fgColor = VSTGUI::kBlueCColor;
      break;
   case HoverOff:
      dc->setFillColor(VSTGUI::kGreyCColor);
      fgColor = VSTGUI::kWhiteCColor;
      break;
   case PressOn:
      dc->setFillColor(VSTGUI::kBlueCColor);
      fgColor = VSTGUI::CColor(255,100,100);
      break;
   case PressOff:
      dc->setFillColor(VSTGUI::kWhiteCColor);
      fgColor = VSTGUI::kGreenCColor;
      break;
   }

   dc->drawRect(size, VSTGUI::kDrawFilled);
   switch( fgMode )
   {
   case Glyph:
   {
      if( glyph != nullptr )
      {
         glyph->updateWithGlyphColor(fgColor);
         VSTGUI::CPoint where(0,0);
         glyph->draw(dc, size, where, 0xff );
      }
   }
   break;
   case Text:
   {
      auto stringR = getViewSize(); 
      dc->setFontColor(fgColor);
      
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
