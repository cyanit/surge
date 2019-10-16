#include "LayoutEngine.h"
#include "SurgeGUIEditor.h"
#include "CScalableBitmap.h"
#include "CHSwitch2.h"
#include "COscillatorDisplay.h"
#include "CGlyphSwitch.h"
#include "CScalableBitmap.h"
#include "CSurgeSlider.h"
#include "CSurgeKnob.h"
#include "CSwitchControl.h"
#include "CStringMultiSwitch.h"
#include "CModulationSourceButton.h"

namespace Surge
{
// Some tiny utilities
std::unordered_map<std::string, std::string> mergeProperties(std::unordered_map<std::string, std::string> &base,
                                                             std::unordered_map<std::string, std::string> &overrides)
{
   std::unordered_map<std::string, std::string> res = base;
   for( auto p : overrides )
   {
      res[p.first] = p.second;
   }
   return res;
}

/*
** The "setupControlFactory" links to the definitions of all the controls, so keep it
** out of the main layoutengine just for code linkage clarity
*/

void LayoutEngine::setupControlFactory()
{
   controlFactory["CHSwitch2"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                        long tag, SurgeGUIEditor* unused, LayoutElement* p) {
      auto comp = components[p->component];
      auto pprops = p->properties;
      auto props = Surge::mergeProperties(comp->properties, pprops);
      
      point_t nopoint(0, 0);
      rect_t rect(0, 0, p->width, p->height);

      rect.offset(p->xoff, p->yoff);

      auto sbp = atoi(props["subPixmaps"].c_str());
      auto h1i = atoi(props["heightOfOneImage"].c_str());

      auto rows = atoi(props["rows"].c_str());
      auto cols = atoi(props["cols"].c_str());

      auto svg = props["svg"];
      if( ! this->loadSVGToBitmapStore(svg) )
      {
         return (CHSwitch2*)nullptr;
      }

      auto res = new CHSwitch2(rect, listener, tag, sbp, h1i, rows, cols,
                               bitmapStore->getLayoutBitmap(this->layoutId, svg), nopoint);
      return res;
   };

   controlFactory["CSwitchControl"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                             long tag, SurgeGUIEditor* unused, LayoutElement* p) {
      auto comp = components[p->component];
      auto pprops = p->properties;
      auto props = Surge::mergeProperties(comp->properties, pprops);

      rect_t rect(0, 0, p->width, p->height);
      rect.offset(p->xoff, p->yoff);

      if( props["svgPerState"] == "true" )
      {
         auto res = new CSwitchControl(rect, listener, tag, nullptr );
         auto pm = std::max(2, std::atoi(props["subPixmaps"].c_str()));
         for( int i=0; i<pm; ++i )
         {
            std::ostringstream oss;
            oss << "svgstate" << i;
            auto svg = props[oss.str()];
            if( ! this->loadSVGToBitmapStore(svg, p->width, p->height) )
               return (CSwitchControl*)nullptr;
            res->setBitmapForState(i, bitmapStore->getLayoutBitmap(this->layoutId, svg));
         }
         return res;
      }
      else
      {
         auto svg = props["svg"];
         if( ! this->loadSVGToBitmapStore(svg) )
         {
            return (CSwitchControl*)nullptr;
         }
         
         auto res = new CSwitchControl(rect, listener, tag, 
                                       bitmapStore->getLayoutBitmap(this->layoutId, svg));
         return res;
      }
      return (CSwitchControl *)nullptr;
   };

   controlFactory["CSurgeKnob"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                         long tag, SurgeGUIEditor* editor, LayoutElement* p) {
      auto comp = components[p->component];
      point_t nopoint(0, 0);
      rect_t rect(0, 0, p->width, p->height);

      rect.offset(p->xoff, p->yoff);
      auto res = new CSurgeKnob(rect, listener, tag);
      return res;
   };

   controlFactory["CModulationSourceButton"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                                      long tag, SurgeGUIEditor* editor, LayoutElement* p) {
      auto pprops = p->properties;
      auto comp = components[p->component];
      auto props = Surge::mergeProperties(comp->properties, pprops);

      point_t nopoint(0, 0);
      rect_t rect(0, 0, p->width, p->height);

      rect.offset(p->xoff, p->yoff);

      auto res = new CModulationSourceButton(rect, listener, tag, 0, 0, bitmapStore);
      return res;
   };

   controlFactory["CGlyphSwitch"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                           long tag, SurgeGUIEditor* editor, LayoutElement* p) {
      auto pprops = p->properties;
      auto comp = components[p->component];
      auto props = Surge::mergeProperties(comp->properties, pprops);

      point_t nopoint(0, 0);
      rect_t rect(0, 0, p->width, p->height);

      rect.offset(p->xoff, p->yoff);

      auto res = new CGlyphSwitch(rect, listener, tag, 7);

      auto glyph = props["glyph"];
      if( glyph != "" )
      {
         auto glyphtag = glyph + "." + guiid;
         if( ! this->loadSVGToBitmapStore(glyph, glyphtag, p->width, p->height) )
            return (CGlyphSwitch*)nullptr;
      
         bitmapStore->getLayoutBitmap(this->layoutId, glyphtag)->setGlyphMode(true);
         res->setGlyphBitmap(bitmapStore->getLayoutBitmap(this->layoutId, glyphtag));
      }
      else
      {
         auto label = props["label"];
         if( label != "" )
         {
            if( label.c_str()[0] == '$' )
               label = stringFromStringMap(label);
            res->setGlyphText(label);
         };
         // FIXME - font and alignment stuff
      }

      auto fgkeys = { "offfg", "onfg", "hoverofffg", "hoveronfg", "pressofffg", "pressonfg" };
      int curr = CGlyphSwitch::Off;
      for( auto k : fgkeys )
      {
         if( props[k] != "" )
         {
            res->setFGColor(curr, this->colorFromColorMap(props[k], "#ffffff" ));
         }
         curr++;
      }
      
      auto bgkeys = { "offbg", "onbg", "hoveroffbg", "hoveronbg", "pressoffbg", "pressonbg" };
      curr = CGlyphSwitch::Off;
      for( auto k : bgkeys )
      {
         if( props[k] != "" )
         {
            res->setBGColor(curr, this->colorFromColorMap(props[k], "#ffffff" ));
         }
         curr++;
      }

      // Background Images in same order as DrawState
      auto svgkeys = { "offsvg", "onsvg", "hoveroffsvg", "hoveronsvg", "pressoffsvg", "pressonsvg" };
      curr = CGlyphSwitch::Off;
      for( auto k : svgkeys )
      {
         if( props[k] != "" )
         {
            if( ! this->loadSVGToBitmapStore(props[k]) )
            {
               return (CGlyphSwitch*)nullptr;
            }
            res->setBGBitmap(curr, bitmapStore->getLayoutBitmap(this->layoutId, props[k]));
         }
         curr++;
      }

      auto rows = std::max(1,std::atoi(props["rows"].c_str()));
      auto cols = std::max(1,std::atoi(props["cols"].c_str()));
      if( rows != 1 || cols != 1 )
      {
         res->setRows(rows);
         res->setCols(cols);
      }
      
      return res;
   };

   controlFactory["CSurgeSlider"] = [this](const guiid_t& guiid, VSTGUI::IControlListener* listener,
                                           long tag, SurgeGUIEditor* editor, LayoutElement* p) {
      auto comp = components[p->component];
      point_t nopoint(0, 0);
      nopoint.offset(p->xoff, p->yoff);

      auto pprops = p->properties;
      auto props = Surge::mergeProperties(comp->properties, pprops);

      int style = Surge::ParamConfig::Style::kHorizontal;
      if( props["orientation"] == "vertical" )
      {
         style = Surge::ParamConfig::Style::kVertical;
      }
      if( props["white"] == "true" )
      {
         style |= kWhite;
      }

      auto res = new CSurgeSlider(nopoint, style,
                                  listener, tag, true, bitmapStore);
      /*
      ** SO MUCH to fix here; like horizontal and vertical; and like the bitmaps we choose and the
      *background.
      ** But this actually works.
      */

      return res;
   };

   controlFactory["COscillatorDisplay"] = [this](const guiid_t& guiid,
                                                 VSTGUI::IControlListener* listener, long tag,
                                                 SurgeGUIEditor* editor, LayoutElement* p) {
      rect_t rect(0, 0, p->width, p->height);
      rect.offset(p->xoff, p->yoff);
      auto oscdisplay =
          new COscillatorDisplay(rect,
                                 &editor->synth->storage.getPatch()
                                      .scene[editor->synth->storage.getPatch().scene_active.val.i]
                                      .osc[editor->current_osc],
                                 &editor->synth->storage);

#if 0        
        oscdisplay->setBGColor( this->colorFromColorMap(comp->properties["bg"], "#000000" ) );
        oscdisplay->setFGcolor( this->colorFromColorMap(comp->properties["fg"], "#ffff90" ) );
        oscdisplay->setRuleColor( this->colorFromColorMap(comp->properties["rule"], "#ccccaa0" ) );
#endif
      return oscdisplay;
   };

   controlFactory["CStringMultiSwitch"] = [this](const guiid_t& guiid,
                                                 VSTGUI::IControlListener *listener,
                                                 long tag,
                                                 SurgeGUIEditor *editor,
                                                 LayoutElement *p) {
      auto pprops = p->properties;
      auto comp = components[p->component];
      auto props = Surge::mergeProperties(comp->properties, pprops);

      point_t nopoint(0, 0);
      rect_t rect(0, 0, p->width, p->height);

      rect.offset(p->xoff, p->yoff);
      auto res = new CStringMultiSwitch(rect, listener, tag);

      int r = std::max(std::atoi(props["rows"].c_str()), 1);
      int c = std::max(std::atoi(props["cols"].c_str()), 1);

      std::string bs = props["border"];
      if( bs == "rounded" )
         res->setBorderStyle(CStringMultiSwitch::RoundedRect);

      res->bgcolor = this->colorFromColorMap(props["bgcolor"], "#ffffff" );
      res->fontcolor = this->colorFromColorMap(props["fontcolor"], "#000000" );
      res->selectcolor = this->colorFromColorMap(props["selectcolor"], "#ff0000" );
      if( props["selectfontcolor"].length() > 0 )
         res->selectfontcolor = this->colorFromColorMap(props["selectfontcolor"], "#000000" );
      else
         res->selectfontcolor = res->fontcolor;
      
      res->depresscolor = this->colorFromColorMap(props["depresscolor"], "#aaaaaa" );
      res->hovercolor = this->colorFromColorMap(props["hovercolor"], "#aaaabb" );
      res->bordercolor = this->colorFromColorMap(props["bordercolor"], "#cccccc" );

      int fs = std::atoi(props["fontsize"].c_str());
      if( fs == 0 ) fs = 10;
      res->fontsize = fs;
      
      
      res->setRows(r);
      res->setCols(c);

      auto sl = props["choices"];
      std::vector<std::string> ch;
      int pos;
      while( (pos = sl.find( "|" )) != std::string::npos )
      {
         auto first = sl.substr(0,pos);
         auto rest = sl.substr(pos+1);
         ch.push_back(first);
         sl = rest;
      }
      ch.push_back(sl);
      std::vector<std::string> finalCh;
      for( auto q : ch )
      {
         if( q.c_str()[0] == '$' )
         {
            finalCh.push_back(stringFromStringMap(q));
         }
         else
         {
            finalCh.push_back(q);
         }
      }
      res->setChoiceLabels(finalCh);

      return res;
   };
}

}
