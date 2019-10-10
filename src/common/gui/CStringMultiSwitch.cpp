#include "CStringMultiSwitch.h"

void CStringMultiSwitch::draw(VSTGUI::CDrawContext *dc)
{
   auto size = getViewSize();
   dc->setFillColor(bgcolor);
   dc->drawRect(size, VSTGUI::kDrawFilled);

   int sel = std::round(getValue() * ( rows * cols - 1 ) );
   int curr = 0;

   float delr = size.getWidth() * 1.0 / rows;
   float delc = size.getHeight() * 1.0 / cols;
   
   for( int r=0; r<rows; ++r )
   {
      int sx = r * delr + size.left;
      for( int c=0; c<cols; ++c )
      {
         int sy = c * delc + size.top;

         bool selected = false;
         if( curr == sel )
         {
            selected = true;
         }
         
         VSTGUI::CRect r( sx, sy, sx+delr + 1, sy+delc + 1 );

         if( curr == depressed )
         {
            dc->setFillColor(depresscolor);
            dc->drawRect(r, VSTGUI::kDrawFilled);
         }
         else if(selected)
         {
            dc->setFillColor(selectcolor);
            auto q = r;
            q.inset(1,1);
            dc->drawRect(q, VSTGUI::kDrawFilled);
         }
         
         VSTGUI::SharedPointer<VSTGUI::CFontDesc> labelFont = new VSTGUI::CFontDesc("Lato", fontsize);
         dc->setFont(labelFont);
         if( selected )
            dc->setFontColor(selectfontcolor);
         else
            dc->setFontColor(fontcolor);

         auto stringR = r;
         stringR.top = sy;
         stringR.bottom = sy + delc;
         dc->drawString(choices[curr].c_str(), stringR, VSTGUI::kCenterText, true);

         dc->setFrameColor(bordercolor);
         dc->drawRect(r, VSTGUI::kDrawStroked);

         curr++;
      }
   }
   
}

VSTGUI::CMouseEventResult CStringMultiSwitch::onMouseDown (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   auto size = getViewSize();
   float delr = size.getWidth() * 1.0 / rows;
   float delc = size.getHeight() * 1.0 / cols;

   float wx = where.x - size.left;
   float wy = where.y - size.top;

   int wr = (int)(wx / delr);
   int wc = (int)(wy / delc);

   int sr = wr * cols + wc;

   depressed = sr;
   invalid();
   return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult CStringMultiSwitch::onMouseUp (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   float nv = depressed * 1.0 / ( rows * cols - 1 );
   if( nv != getValue() )
   {
      setValue(nv);
      if( listener )
         listener->valueChanged(this);
   }
   invalid();
   depressed = -1;
   return VSTGUI::kMouseEventHandled;
}

VSTGUI::CMouseEventResult CStringMultiSwitch::onMouseMoved (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons)
{
   return VSTGUI::kMouseEventHandled;
}
