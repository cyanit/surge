#pragma once
// -*- c++-mode -*-

// Thanks to http://wordaligned.org/articles/cpp-streambufs

#include <iostream>
#include <streambuf>
#include <strstream>

namespace Surge
{
class swrap
{
public:
   std::ostringstream log;
};
template <typename T> swrap& operator<<(swrap& l, T const& value);
typedef std::ostream& (*ostream_manipulator)(std::ostream&);
swrap& operator<<(swrap& os, ostream_manipulator pf);

class LayoutLog
{
   static swrap i, w, e;

public:
   static swrap& info()
   {
      i << "INFO:  ";
      return i;
   }

   static swrap& warn()
   {
      w << "WARN:  ";
      return w;
   }

   static swrap& error()
   {
      e << "ERR :  ";
      return e;
   }

   static void resetErrors()
   {}
};

} // namespace Surge
