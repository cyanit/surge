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
   std::string tx;

   switch( currentState )
   {
   case On:
      dc->setFillColor(VSTGUI::kGreenCColor);
      tx = "On";
      break;
   case Off:
      dc->setFillColor(VSTGUI::kRedCColor);
      tx = "Off";
      break;
   case HoverOn:
      dc->setFillColor(VSTGUI::kYellowCColor);
      tx = "HOn";
      break;
   case HoverOff:
      dc->setFillColor(VSTGUI::kGreyCColor);
      tx = "Hoff";
      break;
   case PressOn:
      dc->setFillColor(VSTGUI::kBlueCColor);
      tx = "Pon";
      break;
   case PressOff:
      dc->setFillColor(VSTGUI::kWhiteCColor);
      tx = "Poff";
      break;
   }

   dc->drawRect(size, VSTGUI::kDrawFilled);

   VSTGUI::SharedPointer<VSTGUI::CFontDesc> labelFont = new VSTGUI::CFontDesc("Lato", 10 );;
   dc->setFont(labelFont);
   dc->setFontColor(VSTGUI::kBlackCColor);
   dc->drawString(tx.c_str(), getViewSize(), VSTGUI::kCenterText, true);

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
