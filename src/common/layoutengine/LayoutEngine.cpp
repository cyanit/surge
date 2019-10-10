#include "SurgeGUIEditor.h"
#include "LayoutEngine.h"
#include <iostream>
#include "CHSwitch2.h"
#include "COscillatorDisplay.h"
#include "CGlyphSwitch.h"
#include "CScalableBitmap.h"
#include "CSurgeSlider.h"
#include "CSurgeKnob.h"
#include "CSwitchControl.h"
#include "CStringMultiSwitch.h"
#include "CModulationSourceButton.h"
#include "LayoutEngineContainer.h"
#include "LayoutLog.h"
#include <strstream>
#include <fstream>

#include "UserInteractions.h"

#if LINUX 
#include <experimental/filesystem>
#elif MAC || (WINDOWS && TARGET_RACK)
#include <filesystem.h>
#else
#include <filesystem>
#endif

namespace fs = std::experimental::filesystem;

#define DUMPR(r)                                                                                   \
   "(x=" << r.getTopLeft().x << ",y=" << r.getTopLeft().y << ")+(w=" << r.getWidth()               \
         << ",h=" << r.getHeight() << ")"



#if !defined(TINYXML_SAFE_TO_ELEMENT)
#define TINYXML_SAFE_TO_ELEMENT(expr) ((expr) ? (expr)->ToElement() : NULL)
#endif

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
   
swrap LayoutLog::i, LayoutLog::w, LayoutLog::e;
template <typename T> swrap& operator<<(swrap& l, T const& value)
{
   l.log << value;
   std::cout << value;
   return l;
}

typedef std::ostream& (*ostream_manipulator)(std::ostream&);
swrap& operator<<(swrap& os, ostream_manipulator pf)
{
   return operator<<<ostream_manipulator>(os, pf);
}

LayoutEngine::LayoutEngine()
{
   LayoutLog::resetErrors();
   LayoutLog::info() << "Constructing a Layout Engine" << std::endl;
   setupControlFactory();
}

LayoutEngine::~LayoutEngine()
{
   LayoutLog::info() << "Destroying a Layout Engine" << std::endl;
}

bool LayoutEngine::parseLayout()
{
   TiXmlDocument doc;
   // Obviously fix this
   doc.SetTabSize(4);

   doc.LoadFile(layoutResource("layout.xml"));
   TiXmlElement* surgeskin = TINYXML_SAFE_TO_ELEMENT(doc.FirstChild("surge-skin"));

   if( surgeskin == nullptr )
   {
      // FIXME - better error message here
      Surge::UserInteractions::promptError( "layout.xml didn't contain surge-skin.",
                                     "Layout Error" );
      return false;
   }
   
   const char* vers = surgeskin->Attribute("version");
   if (vers && strcmp(vers, "1") == 0)
   {
   }
   else
   {
      LayoutLog::error() << "XML has unrecognized version '" << (vers ? "NULL" : vers)
                         << "' and we expected '1'" << std::endl;
   }

   TiXmlElement* cms = TINYXML_SAFE_TO_ELEMENT(surgeskin->FirstChild("colormaps"));
   parseColormaps(cms);

   TiXmlElement* ss = TINYXML_SAFE_TO_ELEMENT(surgeskin->FirstChild("strings"));
   parseStringmaps(ss);

   TiXmlElement* components = TINYXML_SAFE_TO_ELEMENT(surgeskin->FirstChild("components"));
   if (components)
   {
      for (auto child = components->FirstChild(); child; child = child->NextSibling())
      {
         auto* lkid = TINYXML_SAFE_TO_ELEMENT(child);
         if (lkid && strcmp(lkid->Value(), "component") == 0)
         {
            auto comp = LayoutComponent::fromXML(lkid);
            this->components[comp->name] = nullptr;
            this->components[comp->name].reset(comp);
         }
      }
   }

   TiXmlElement* layout = TINYXML_SAFE_TO_ELEMENT(surgeskin->FirstChild("layout"));

   rootLayoutElement.reset(LayoutElement::fromXML(layout, nullptr, this));
   // FIX ME do better than this
   assert(rootLayoutElement->xoff == 0 && rootLayoutElement->yoff == 0 &&
          rootLayoutElement->width > 0 && rootLayoutElement->height > 0);

   rootLayoutElement->setupChildSizes();

   buildNodeMapFrom(rootLayoutElement.get());

   LayoutLog::info() << "Layout dump\n" << rootLayoutElement->toString() << std::endl;
   return true;
}

void LayoutEngine::buildNodeMapFrom(LayoutElement* here)
{
   if (here->type == LayoutElement::Node)
   {
      nodeMap[here->guiid] = here;
   }
   else
   {
      for (auto k : here->children)
         buildNodeMapFrom(k);
   }
}

LayoutEngine::container_t* LayoutEngine::generateLayoutRootControl()
{
   assert(rootLayoutElement.get());
   rootLayoutElement->generateLayoutControl(this);
   return rootLayoutElement->associatedContainer;
}

LayoutEngine::container_t* LayoutEngine::getSubContainerByLabel(std::string label)
{
   std::function<container_t*(std::string, LayoutElement*)> recurse;

   recurse = [&recurse](std::string n, LayoutElement* el) -> container_t* {
      if (el->label == n)
      {
         return el->associatedContainer;
      }
      for (auto k : el->children)
      {
         auto kc = recurse(n, k);
         if (kc != nullptr)
         {
            return kc;
         }
      }
      return nullptr;
   };
   return recurse(label, rootLayoutElement.get());
}
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

bool LayoutEngine::loadSVGToBitmapStore(std::string svg, float w, float h)
{
   if (!bitmapStore->containsLayoutBitmap(this->layoutId, svg))
   {
      auto svgf = layoutResource(svg);
      std::ifstream t(svgf.c_str());
      if( ! t.is_open() )
      {
         LayoutLog::error() << "Unable to open SVG " << svg << std::endl;
         return false;
      }
      else
      {
         std::stringstream buffer;
         buffer << t.rdbuf();
         auto bm = bitmapStore->storeLayoutBitmap(this->layoutId, svg, buffer.str(), this->frame);
         if( w > 0 && h > 0 )
         {
            bm->setInherentScaleForSize(w,h);
         }
      }
   }
   return true;
}

LayoutEngine::control_t* LayoutEngine::addLayoutControl(const guiid_t& guiid,
                                                        VSTGUI::IControlListener* listener,
                                                        long tag,
                                                        SurgeGUIEditor* synth)
{
   assert(rootLayoutElement->associatedContainer);
   if (!bitmapStore)
   {
      LayoutLog::error() << "addLayoutControl called without a bitmapstore set on Layout Engine"
                         << std::endl;
      return nullptr;
   }

   point_t nopoint(0, 0);

   auto* p = nodeMap[guiid];
   std::string failmsg = "";

#define FAIL_IF(condition, msg)                                                                    \
   if (condition)                                                                                  \
   {                                                                                               \
      failmsg = msg;                                                                               \
      goto fail;                                                                                   \
   }

   {
      FAIL_IF(!p, "No Component for guiid ");
      FAIL_IF(!p->parent, "Component is not parented");
      FAIL_IF(!p->parent->associatedContainer, "Component has no associated container");
      FAIL_IF(components.find(p->component) == components.end(),
              "Component type is not mapped for type '" + p->component + "'");
      auto comp = components[p->component];

      FAIL_IF(controlFactory.find(comp->classN) == controlFactory.end(),
              "No Factory for control type " + comp->classN);

      controlGenerator_t gen = controlFactory[comp->classN];
      auto res = gen(guiid, listener, tag, synth, p);

      FAIL_IF(!res, "Factory returned a null component");

      p->parent->associatedContainer->addView(res);
      return res;
   }

fail:
   LayoutLog::error() << "Failed to construct component '" << guiid << "' with failmsg '" << failmsg
                      << "'" << std::endl;
   return nullptr;
}

float LayoutEngine::getWidth()
{
   return rootLayoutElement->width;
}
float LayoutEngine::getHeight()
{
   return rootLayoutElement->height;
}

void LayoutEngine::parseColormaps(TiXmlElement* cms)
{
   if (!cms || strcmp(cms->Value(), "colormaps") != 0)
   {
      LayoutLog::warn() << "Colormaps node missing or misnamed" << std::endl;
      return;
   }

   for (auto child = cms->FirstChild(); child; child = child->NextSibling())
   {
      colormap_t thisMap;
      auto lkid = TINYXML_SAFE_TO_ELEMENT(child);
      if (lkid && strcmp(lkid->Value(), "colormap") == 0)
      {
         const char* s;
         std::string name = "invalid";
         s = lkid->Attribute("name");
         if (s)
            name = s;
         for (auto cmc = child->FirstChild(); cmc; cmc = cmc->NextSibling())
         {
            auto lcmc = TINYXML_SAFE_TO_ELEMENT(cmc);

            if (lcmc && strcmp(lcmc->Value(), "color") == 0)
            {
               const char* s;
               std::string name = "invalid", value = "#ff0000";
               s = lcmc->Attribute("name");
               if (s)
                  name = s;
               s = lcmc->Attribute("value");
               if (s)
                  value = s;
               thisMap[std::string("$") + name] = value;
            }
         }
         colormaps[name] = thisMap;
      }
   }

   // find the name of the default colormap
   const char* s;
   std::string name = "default";
   s = cms->Attribute("default");
   if (s)
      name = s;
   // FIXME error checking
   active_colormap = colormaps[name];
}

void LayoutEngine::parseStringmaps(TiXmlElement* ss)
{
   if (!ss || strcmp(ss->Value(), "strings") != 0)
   {
      LayoutLog::warn() << "strings map missing or misnamed" << std::endl;
      return;
   }

   for (auto child = ss->FirstChild(); child; child = child->NextSibling())
   {
      stringmap_t thisMap;
      auto lkid = TINYXML_SAFE_TO_ELEMENT(child);
      if (lkid && strcmp(lkid->Value(), "stringmap") == 0)
      {
         const char* s;
         std::string lang = "en";
         s = lkid->Attribute("language");
         if (s)
            lang = s;
         for (auto cmc = child->FirstChild(); cmc; cmc = cmc->NextSibling())
         {
            auto lcmc = TINYXML_SAFE_TO_ELEMENT(cmc);

            if (lcmc && strcmp(lcmc->Value(), "string") == 0)
            {
               const char* s;
               std::string name = "invalid", value = "invalid";
               s = lcmc->Attribute("name");
               if (s)
                  name = s;
               s = lcmc->Attribute("value");
               if (s)
                  value = s;
               thisMap[std::string("$") + name] = value;
            }
         }
         strings[lang] = thisMap;
      }
   }

   // find the name of the default colormap
   const char* s;
   std::string lang = "en";
   s = ss->Attribute("default");
   if (s)
      lang = s;

   active_stringmap = strings[lang];
}

LayoutElement* LayoutElement::fromXML(TiXmlElement* el, LayoutElement* parent, LayoutEngine* eng)
{
   if (!el)
      return nullptr;

   const char* s;

   auto res = new LayoutElement();

   if (strcmp(el->Value(), "layout") == 0)
   {
      res->type = LayoutElement::Layout;
   }
   else if (strcmp(el->Value(), "node") == 0)
   {
      res->type = LayoutElement::Node;
   }
   else if (strcmp(el->Value(), "label") == 0)
   {
      res->type = LayoutElement::Label;
   }
   else
   {
      LayoutLog::error() << "Unable to parse node with value '" << el->Value() << "' at line "
                         << el->Row() << std::endl;
      return nullptr;
   }

   s = el->Attribute("style");
   if (s)
      res->style = s;
   s = el->Attribute("bgcolor");
   if (s)
      res->bgcolor = s;
   s = el->Attribute("fgcolor");
   if (s)
      res->fgcolor = s;
   s = el->Attribute("label");
   if (s)
      res->label = s;

   s = el->Attribute("component");
   if (s)
      res->component = s;
   s = el->Attribute("guiid");
   if (s)
      res->guiid = s;

   s = el->Attribute("mode");
   if (s)
   {
      if (strcmp(s, "free") == 0)
         res->mode = Free;
      else if (strcmp(s, "hlist") == 0)
         res->mode = Hlist;
      else if (strcmp(s, "vlist") == 0)
         res->mode = Vlist;
      else if (strcmp(s, "grid") == 0)
         res->mode = Grid;
      else
      {
         LayoutLog::error() << "Unknown layout mode '" << s << "' at line " << el->Row()
                            << std::endl;
      }
   }

   // OK so now set up margins for some custom types
   if( res->type == Layout && ( res->style == "roundedblock" || res->style == "roundedlabel" ) && res->mode != Free )
   {
      // FIXME we probably want this as a parameter
      res->marginx = 5;
      res->marginy = 5;
   }
   
   // If I'm have a component:
   if (res->component != "")
   {
      // My default width (which I can override) comes from the component
      if (eng->components.find(res->component) == eng->components.end())
      {
         LayoutLog::error() << "Unknown component for guiid '" << res->guiid
                            << "' requesting component '" << res->component << "' at line "
                            << el->Row() << std::endl;
      }
      else
      {
         auto cp = eng->components[res->component];
         // ERROR CHECKING
         res->width = cp->width;
         res->height = cp->height;
      }
   }

   s = el->Attribute("width");
   if( s )
   {
      if( strcmp(s,"auto")== 0 ) res->width = kAutomaticSize;
      else res->width=std::atof(s);
   }
   s = el->Attribute("height");
   if( s )
   {
      if( strcmp(s,"auto")== 0 ) res->height = kAutomaticSize;
      else res->height=std::atof(s);
   }

   s = el->Attribute("xoff");
   if( s )
   {
      if( strcmp(s,"left")==0 ) res->xoff = kOffLeft;
      else if( strcmp(s, "center") == 0 ) res->xoff= kOffCenter;
      else if( strcmp(s, "right") == 0 ) res->xoff = kOffRight;
      else res->xoff = std::atof(s);
   }
   s = el->Attribute("yoff");
   if( s )
   {
      if( strcmp(s,"left")==0 ) res->yoff = kOffLeft;
      else if( strcmp(s, "center") == 0 ) res->yoff= kOffCenter;
      else if( strcmp(s, "right") == 0 ) res->yoff = kOffRight;
      else res->yoff = std::atof(s);
   }

   // Grab all the attributes into a map
   for (auto a = el->FirstAttribute(); a; a = a->Next())
   {
      res->properties[a->Name()] = a->Value();
   }

   

   if (parent)
   {
      res->parent = parent;
      res->depth = res->parent->depth + 1;
   }
   else
   {
      res->parent = nullptr;
      res->depth = 0;
   }

   if( res->type == Node )
   {
      TiXmlElement* nprops = TINYXML_SAFE_TO_ELEMENT(el->FirstChild("nodeproperties"));
      
      if (nprops)
      {
         for (auto a = nprops->FirstAttribute(); a; a = a->Next())
         {
            res->properties[a->Name()] = a->Value();
         }
      }
   } else {
      for (auto child = el->FirstChild(); child; child = child->NextSibling())
      {
         auto* lkid = fromXML(TINYXML_SAFE_TO_ELEMENT(child), res, eng);
         if (lkid)
         {
            res->children.push_back(lkid);
         }
         else
         {
            // I got some other type like a comment most likely
         }
      }
   }

   return res;
}

std::string LayoutElement::toString(bool recurse)
{
   std::ostringstream oss;

   for (auto i = 0; i < depth; ++i)
      oss << "|--";
   if (depth > 0)
      oss << " ";

   switch (type)
   {
   case Layout:
      oss << "layout ";
      switch (mode)
      {
      case Free:
         oss << "(free|";
         break;
      case Hlist:
         oss << "(hlist|";
         break;
      case Vlist:
         oss << "(vlist|";
         break;
      case Grid:
         oss << "(grid " << properties["cols"] << "x" << properties["rows"] << "|";
         break;
      }
      if( style == "" )
         oss << "default) ";
      else
         oss << style << ") ";
      break;
   case Node:
      oss << "node guiid='" << guiid << "' ";
      break;
   case Label:
      oss << "label string='" << properties["string"] << "' ";
   }
   oss << "label=\"" << label << "\" ";

   oss << "off=(" << xoff << "," << yoff << ") sz=(" << width << "," << height << ") ";

   if (associatedContainer)
      oss << " ac=" << (size_t)associatedContainer << " ";

   if (recurse)
   {
      oss << "\n";
      for (auto kid : children)
         oss << kid->toString(recurse);
   }
   return oss.str();
}

void LayoutElement::generateLayoutControl(LayoutEngine* eng, bool recurse)
{
   if (type == Node)
      return; // Nodes are only generated when added
   if (type == Label)
   {
      if (!parent || !parent->associatedContainer)
      {
         LayoutLog::error() << "Label has either a missing or invalid parent. Software error" << std::endl;
         return;
      }
      VSTGUI::CRect pos(xoff, yoff, xoff + width, yoff + height);

      auto label = new LayoutEngineLabel(pos, "");
      label->setTransparency(true);
      
      auto getprop = [this](std::string s, std::string d) -> std::string {
         auto pnode = this->properties.find(s);
         auto p = std::string(d);
         if (pnode != this->properties.end())
            p = pnode->second;
         return p;
      };

      auto halign = getprop("halign", "left");
      if (halign == "right")
         label->setHoriAlign(VSTGUI::kRightText);
      else if (halign == "center")
         label->setHoriAlign(VSTGUI::kCenterText);
      else
         label->setHoriAlign(VSTGUI::kLeftText);

      auto fg = getprop("fgcolor", "#ffffff");
      label->setFontColor(eng->colorFromColorMap(fg, "#ff00ff"));
      auto c = label->getFontColor();

      auto txt = getprop("string", "error");
      if( txt[0] == '$' )
         txt = eng->stringFromStringMap(txt);

      label->setText(txt.c_str());

      parent->associatedContainer->addView(label);
      associatedControl = label;
      return;
   }

   // Since we are adding to our parent we can use the offset not the absolute position here
   LayoutEngine::rect_t position(LayoutEngine::point_t(xoff, yoff),
                                 LayoutEngine::point_t(width, height));
   auto lec = new LayoutEngineContainer(position);
   lec->label = label;
   lec->style = style;
   lec->bgcolor = eng->colorFromColorMap(bgcolor, "#ff0000");
   lec->fgcolor = eng->colorFromColorMap(fgcolor, "#000000");

   associatedContainer = lec;

   if (parent && parent->associatedContainer)
   {
      auto ret = parent->associatedContainer->addView(associatedContainer);
      if (!ret)
      {
         LayoutLog::error() << "Failed to add physical component parent for guiid '" << guiid << "'"
                            << std::endl;
      }
   }
   if (recurse)
   {
      for (auto kid : children)
         kid->generateLayoutControl(eng, recurse);
   }
}

std::string LayoutEngine::stringFromStringMap(std::string name)
{
   if (name.c_str()[0] == '$')
   {
      std::string strip = name.c_str() + 1;
      auto p = active_stringmap.find(name);
      if (p != active_stringmap.end())
         return p->second;
   }

   return name + "=error";
}

LayoutEngine::color_t LayoutEngine::colorFromColorMap(std::string name, std::string hexDefault)
{
   std::string hexCol = hexDefault;

   if (name.c_str()[0] == '#')
   {
      hexCol = name;
   }
   else if (name.c_str()[0] == '$')
   {
      auto mapCol = active_colormap[name];
      if (mapCol.c_str()[0] == '#')
      {
         hexCol = mapCol;
      }
      else
      {
         // std::cout << "Bad lookup: hexCol " << name << " -> " << mapCol << std::endl;
      }
   }
   else
   {
      // std::cout << "Don't know what to do with name '" << name << "'" << std::endl;
   }

   int r, g, b;
   int amp = 50;
   sscanf(hexCol.c_str() + 1, "%02x%02x%02x", &r, &g, &b);

   return color_t(r, g, b);
}

void LayoutElement::setupChildSizes()
{
   // FIXME - think about this
   if (type == Node)
      return;

   switch (mode)
   {
   case Free:
   {
      for (auto kid : children)
      {
         // FIXME do better than this. Like check for invalid states and stuff
         kid->setupChildSizes();
         
         if( kid->xoff == kOffLeft ) kid->xoff = marginx;
         else if( kid->xoff == kOffRight ) kid->xoff = width - kid->width - marginx;
         else if( kid->xoff == kOffCenter ) kid->xoff = (width - kid->width ) / 2;
         else kid->xoff += marginx;

      
         if( kid->yoff == kOffLeft ) kid->yoff = marginy;
         else if( kid->yoff == kOffRight ) kid->yoff = height - kid->height - marginy;
         else if( kid->yoff == kOffCenter ) kid->yoff = (height - kid->height ) / 2;
         else kid->yoff += marginy;
      }
      break;
   }
   case Hlist:
   {
      // So basically the yOff is the running width
      // if the height is kAutomaticSize it is my height
      // if width is kAutomaticSize it is fill the box along with other -1s evenly
      float given_width = 0;
      int free_widths = 0;
      for (auto kid : children)
      {
         if (kid->width > kAutomaticSize )
            given_width += kid->width;
         else
            free_widths++;
      }

      float default_width = 0;
      if (free_widths)
         default_width = (width - 2 * marginx - given_width) / free_widths;

      float kid_xoff = marginx;
      for (auto kid : children)
      {
         if (kid->height == kAutomaticSize)
            kid->height = height - 2 * marginy;
         if (kid->width == kAutomaticSize)
            kid->width = default_width;
         kid->xoff = kid_xoff;
         kid_xoff += kid->width;
         kid->setupChildSizes();
         
         if( kid->yoff == kOffLeft ) kid->yoff = marginy;
         else if( kid->yoff == kOffRight ) kid->yoff = height - kid->width - marginy;
         else if( kid->yoff == kOffCenter ) kid->yoff = (height - kid->width ) / 2;
         else kid->yoff = marginy;
      }
      break;
   }
   case Vlist:
   {
      // Copy and Paste with x<->y and w<->h. What could go wrong?
      float given_height = 0;
      int free_heights = 0;
      for (auto kid : children)
      {
         if (kid->height > kAutomaticSize)
            given_height += kid->height;
         else
            free_heights++;
      }

      float default_height = 0;
      if (free_heights)
         default_height = (height - 2 * marginy - given_height) / free_heights;

      float kid_yoff = marginy;
      for (auto kid : children)
      {
         if (kid->height == kAutomaticSize)
            kid->height = default_height;
         if (kid->width == kAutomaticSize)
            kid->width = width - 2 * marginy;
         kid->yoff = kid_yoff;
         kid_yoff += kid->height;

         kid->setupChildSizes();
         if( kid->xoff == kOffLeft ) kid->xoff = marginx;
         else if( kid->xoff == kOffRight ) kid->xoff = width - kid->width - marginx;
         else if( kid->xoff == kOffCenter ) kid->xoff = (width - kid->width ) / 2;
         else kid->xoff += marginx;
      }
      break;
   }
   case Grid:
   {
      int rows = std::max(1,std::atoi(properties["rows"].c_str()));
      int cols = std::max(1,std::atoi(properties["cols"].c_str()));

      float dc = width * 1.0 / cols;
      float dr = height * 1.0 / rows;

      int cr=0, cc=0;

      for( auto kid : children )
      {
         kid->xoff = cc * dc + marginx;
         kid->yoff = cr * dr + marginy;
         kid->width = dc;
         kid->height = dr;
         cc++;
         if( cc == cols )
         {
            cc = 0;
            cr++;
         }
      }
      
      break;
   }
   }
}

LayoutComponent* LayoutComponent::fromXML(TiXmlElement* e)
{
   // FIXME - more defensive parsing please
   std::string name = e->Attribute("name");
   std::string classN = e->Attribute("class");

   auto res = new LayoutComponent(name, classN);

   double d;
   if (e->QueryDoubleAttribute("width", &d) == TIXML_SUCCESS)
      res->width = d;
   if (e->QueryDoubleAttribute("height", &d) == TIXML_SUCCESS)
      res->height = d;

   TiXmlElement* cprops = TINYXML_SAFE_TO_ELEMENT(e->FirstChild("componentproperties"));

   if (cprops)
   {
      for (auto a = cprops->FirstAttribute(); a; a = a->Next())
      {
         res->properties[a->Name()] = a->Value();
      }
   }

   return res;
}

void LayoutLibrary::initialize(SurgeStorage *storage)
{
   // FIXME - implement this somewhat tedious code
   return;
   LayoutLog::info() << "Initializing LayoutLibrary" << std::endl;
   LayoutLog::info() << "  dp=" << storage->datapath << std::endl;
   LayoutLog::info() << "  up=" << storage->userDataPath << std::endl;

   std::vector<std::string> paths = { storage->datapath, storage->userDataPath };

   for( auto sourceS : paths )
   {
      fs::path source(sourceS);
      std::vector<fs::path> candidates;
   
      std::vector<fs::path> alldirs;
      std::deque<fs::path> workStack;
      workStack.push_back( source );
      while (!workStack.empty())
      {
         auto top = workStack.front();
         workStack.pop_front();
         for (auto &d : fs::directory_iterator( top ))
         {
            if (fs::is_directory(d))
            {
               alldirs.push_back(d);
               workStack.push_back(d);
            }
         }
      }


      int sourceSubstrLength= source.generic_string().size() + 1;
      if (source.generic_string().back() == '/' || source.generic_string().back() == '\\')
       sourceSubstrLength --;

      for( auto &d : fs::directory_iterator(source))
      {
         if( fs::is_directory(d) )
         {
            std::string name;
#if WINDOWS && ! TARGET_RACK
            /*
            ** Windows filesystem names are properly wstrings which, if we want them to 
            ** display properly in vstgui, need to be converted to UTF8 using the 
            ** windows widechar API. Linux and Mac do not require this.
            */
            std::wstring str = d.path().wstring().substr(sourceSubstrLength);
            name = Surge::Storage::wstringToUTF8(str);
#else
            name = d.path().generic_string().substr(sourceSubstrLength);
#endif
         }
      }
   }
}

std::vector<std::string> LayoutLibrary::availbleLayouts;

} // namespace Surge
