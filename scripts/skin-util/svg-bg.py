# Make SVG Backgrounds

w = 60
h = 18
horiz = True

for i in range(6):

    on = False
    if(i < 3):
        on = True
    mode = i % 3

    modes = ""
    if(mode == 1):
        modes = "Hover"
    if(mode == 2):
        modes = "Press"
    ons = "Off"
    if(on):
        ons = "On"

    name = "%dx%d_Switch_%s%s.svg" % (w, h, modes, ons)

    res = "<!-- %s -->" % (name)

    res = res + "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    res = res + ("<svg viewbox=\"0 0 %d %d\" xmlns=\"https://www.w3.org/2000/svg\">\n" % (w, h))
    res = res + "  <defs>\n"

    if(mode == 0):
        g1r = 90
        g1g = 80
        g1b = 75
    elif (mode == 1):
        g1r = 120
        g1g = 88
        g1b = 50
    else:
        g1r = 80
        g1g = 28
        g1b = 10

    gradient = """
    <linearGradient id="gradient-0" gradientUnits="userSpaceOnUse" x1="%lf" y1="0" x2="%lf" y2="%lf" 
    spreadMethod="pad">
      <stop offset="0" style="stop-color: rgb(%d, %d, %d);"/>
      <stop offset="1" style=""/>
    </linearGradient>
    """ % (w / 2.0, w / 2.0, h / 2, g1r, g1g, g1b)
    res = res + gradient

    gradient = """
   <linearGradient id="gradient-1" gradientUnits="userSpaceOnUse" x1="%lf" y1="0" x2="%lf" y2="%lf" 
                   spreadMethod="pad">
      <stop offset="0" style="stop-color: rgb(238,128,0);"/>
      <stop offset="1" style="stop-color: rgb(255,144,0);"/>
    </linearGradient>
""" % (w / 2.0, w / 2.0, h)

    res = res + gradient

    res = res + "  </defs>\n"

    res += "  <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" style=\"fill: url(#gradient-0);\"/>\n" % (w, h)
    res += "  <rect x=\"5\" y=\"0\" width=\"2\" height=\"%d\" style=\"fill-rule: evenodd; paint-order: stroke; fill: rgb(0,0,0); \"/>\n" % (
        h)

    if(on):
        res += "  <rect x=\"0\" y=\"0\" width=\"5\" height=\"%d\" style=\"fill: url(#gradient-1);\"/>\n" % (h)

    res = res + "</svg>\n"

    with open("/tmp/" + name, "w") as f:
        f.write(res)
