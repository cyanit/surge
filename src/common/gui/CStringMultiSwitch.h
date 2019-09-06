#pragma once

#include "SurgeStorage.h"
#include "vstcontrols.h"
#include <vector>
#include <string>


class CStringMultiSwitch : public VSTGUI::CControl
{
public:
   CStringMultiSwitch(VSTGUI::CRect &size, VSTGUI::IControlListener *listener, int32_t tag)
      : VSTGUI::CControl(size,
                         listener,
                         tag)
   {
      rows = 1; cols = 1;
      depressed = -1;
      borderstyle = Rectangle;
   }

   virtual void setRows(int r) { rows = r; }
   virtual void setCols(int c) { cols = c; }
   virtual void setChoiceLabels(std::vector<std::string> &l) {
      choices = l;
   }

   typedef enum BorderStyle {
      Rectangle,
      RoundedRect,
   } BorderStyle;
   virtual void setBorderStyle(BorderStyle t) { borderstyle=t; }
   // Support all the colors
   
   virtual void draw(VSTGUI::CDrawContext *pContext) override;

   virtual VSTGUI::CMouseEventResult onMouseDown (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
	virtual VSTGUI::CMouseEventResult onMouseUp (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;
	virtual VSTGUI::CMouseEventResult onMouseMoved (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override;


   VSTGUI::CColor bgcolor, fontcolor, selectcolor, selectfontcolor, depresscolor, hovercolor, bordercolor;
   int fontsize;
   
	/** called when the mouse enters this view */
	// virtual VSTGUI::CMouseEventResult onMouseEntered (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {return kMouseEventNotImplemented;}
	/** called when the mouse leaves this view */
	// virtual VSTGUI::CMouseEventResult onMouseExited (VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) {return kMouseEventNotImplemented;}

   CLASS_METHODS(CStringMultiSwitch, CControl)

private:
   int rows, cols;
   std::vector<std::string> choices;

   int depressed;
   BorderStyle borderstyle;
   
};
