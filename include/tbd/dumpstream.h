///////////////////////////////////////////////////////////////////////////////
/// @file dumpstream.h
/// @brief Dump stream implementation
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
# pragma warning(disable:4290 4996)
#endif

#include "domstream.h"
#include <sstream>
#include <stdlib.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#ifndef __TBD__DUMPSTREAM_H
#define __TBD__DUMPSTREAM_H

/// @defgroup DumpStreams Dump streams
/// @brief A set of classes that can be used to produce and parse Dump
///        representations of data.
/// @ingroup DomStreams

namespace tbd
{
  namespace dump
  {
    /// @brief writes a node and all children into an output stream.
    /// @param os output stream
    /// @param node Node to write.
    /// @param strLineFeed For better reading you can insert line feeds with
    ///        this parameter. This should be Dump complaint "\n" or "" if you
    ///        don't want line feeds in your output (this is the default).
    /// @param depth Deepness in the node tree at the current write
    ///        position. This value will be used to generate indentations.
    /// @param properties if true dumps attribute items too
    /// @param indent amount of indentation before every line
    /// @param maxChildren maximum number of children per item
    ///        (inserts "more items.." button at end if exceeded)
    template<class O> void write(O& os, DomNode* node, const std::string& strLineFeed = "\n", int depth = INT_MAX, bool properties = true, int indent = 0, int maxChildren = 256)
    {
      // measure written characters to estimate child indent from here
      typename O::pos_type nBegin = os.tellp();
      // indent name
      // hide hidden values
      if (node->isHidden())
        os << std::string(indent, ' ') << "(" << node->getName() << ")";
      else
        os << std::string(indent, ' ') << node->getName();
      // if there is a INDEX child, write index
      os << " = ";
      // measure written characters to estimate child indent to here
      int nValueIndent = (int) (os.tellp() - nBegin);
      // separate lines of value
      std::vector<std::string> strs;
      {
        // must use a temporary string because boost::split is squeamish
        std::string str = node->getValueStr();
        boost::split(strs, str, boost::is_any_of(std::string("\n")));
      }
      // write each value line
      bool bFirst = true;
      if (strs.empty())
        os << strLineFeed;
      else
      {
        BOOST_FOREACH(std::string& s,strs)
        {
          if (!bFirst)
            os << std::string(nValueIndent, ' ');
          else
            bFirst = false;
          os << s << strLineFeed;
        }
      }
      int children = 0;
      BOOST_FOREACH( DomNode* child, *node )
      {
        if (children++ < maxChildren)
        {
          if (child->isAttribute() || child->isHidden())
          {
            if (depth >= 0 && properties)
              write(os, child, strLineFeed, depth - 1, properties, indent + 2);
          }
          else if (depth > 0)
            write(os, child, strLineFeed, depth - 1, properties, indent + 2);
        }
      }

      if (children >= maxChildren)
        os << std::string(indent, ' ') << "(" << children - maxChildren << " more items)" << strLineFeed;
    }
    /// @brief Writes the DOM of this instance in Dump to a given output stream.
    /// @param os Output stream to write into
    /// @param dos DOM stream to write
    /// @param strLineFeed For better reading you can insert line feeds with
    ///        this parameter. This should be Dump complaint "\n" or "" if you
    ///        don't want line feeds in your output (this is the default).
    /// @param depth Deepness in the node tree at the current write
    ///        position. This value will be used to generate indentations.
    /// @param properties if true dumps attribute items too
    /// @param maxChildren maximum number of children per item
    ///        (inserts "more items.." button at end if exceeded)
    template<class O> void write(O& os, DomOStream& dos, const std::string& strLineFeed = "\n", int depth = INT_MAX, bool properties = true, int maxChildren = 256)
    {
      int children = 0;
      // Enumerate all children of the internal root node
      BOOST_FOREACH( DomNode* child, *dos.getRoot() )
      {
        // if maximum children isn't reached
        if (children++ < maxChildren)
          // write the node into the output stream
          write(os, child, strLineFeed, depth, properties, 0, maxChildren);
      }
      if (children >= maxChildren)
        os << "(" << children - maxChildren << " more items)" << strLineFeed;
    }
    template<class T,class O> void write(O& os,const T& t, const std::string& strLineFeed = "\n", int depth = INT_MAX, bool properties = true, int nIndent = 0, int maxChildren = 256)
    {
      DomOStream dos;
      t.dump(dos);
      write<O>(os, dos, strLineFeed, depth, properties, maxChildren);
    }
    template<class T> std::string str(const T& t, const std::string& strLineFeed = "\n", int depth = INT_MAX, bool properties = true, int nIndent = 0, int maxChildren = 256)
    {
      std::stringstream ss;
      write(ss, t, strLineFeed, depth, properties, nIndent, maxChildren);
      return ss.str();
    }
  }
}

#ifdef _MSC_VER
# pragma warning(default:4290 4996)
#endif
#endif
