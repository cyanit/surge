Here's my sort of running todo as I think of it, in rough order

* Glyph Switch
  * Multi-Glyph not just Multi-Text
  * Kill the CMultiSwitch
  * Also move the POLY switch over
* Do we keep the Multi-StringSwitch (like AB and OSC select) or combine? Probably combine.
* Pitch Bend Area
* Labels and Fonts
* ctrlstyles in the xml
  * add ctrlstyle to the nodeproperties which can push onto p->ctrlstyle
  * Supress infowindow for scene (kNoPopup)
  * don't do kMeta or kEasy - separate those out
* Move the CHSwitch2s
* XML configured multi-switch rather than SVG configured multi-switch
  * properties for drawing and a few classes
  * Add a multi-text switch which encompasses toggle
* Alternate switch SVG Mode (non-stacked; give a pair of SVGs)
* Slider Parameterization and Work
  * Handle and BG SVGs
  * API for painting ticks on sliders; and sizing sliders
  * SVG for slider handles
* Modulation Button Properties for Colors exposed
* Find and swap layouts
  * LayoutDir to '.layout'
  * Scan for Layouts in the userpath and factorypath
  * Menu to swap layouts
* Move the LFO section over
* Finish the Scene and Patch section
  * What to do about the patch browser
* Finish OSC Section
  * Especially the rounded rects
* Sublayouts maybe?
* Finish positioning all the sliders with the XML
* Move the rest of the CHSwitch2s over
* FX routing and FX section
* Patch Browser
* What's left in legacy?
* Why's it coredump if layout is missing?
* Font Controls
* Second layout with stupid pet tricks
* Cleanup of bitmaps at close. Do they all get forgotten?
* About Screen 
  * Lets do proper git ID at make time also

* Knobs glorious knobs
* Strings file
  * Layout strings to a saprate file
  * Engine Strings
* Multi-OSC view (thumbnails!)
* Add 3 more oscillators and have the parameters not break (use the parameter defered chaning)

* Context Help
* Where to put menus? How to specify menus in the layout?

* VST2 as well as 3 and AU
* Installer -> Surge++ nomenclature mac
* Installer Windows
* Installer Linux
* Pipeline in BaconPaul land

* Unleash the hounds


