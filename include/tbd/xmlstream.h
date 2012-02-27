///////////////////////////////////////////////////////////////////////////////
/// @file xmlstream.h
/// @brief XML stream implementation
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#include "domstream.h"
#include <sstream>
#include <stdlib.h>

#ifndef __TBD__XML_H
#define __TBD__XML_H
#ifdef _MSC_VER
# pragma warning(disable:4290)
#endif

/** @defgroup XmlStreams XML streams
 *  @brief A set of classes that can be used to produce and parse XML
 *         representations of data.
 *  @ingroup DomStreams
 */

namespace tbd
{
  /** @brief Exception thrown by the XmlOStream class
   *  @ingroup XmlStreams
   */
  class XmlWriteException: public Exception
  {
  public:
    enum ErrCode
    {
      Ok, ValueAndElements
    };
    explicit XmlWriteException(ErrCode eErrCode, DomNode* pNode) :
      m_eErrCode(eErrCode), m_strPath(pNode->getPath())
    {
    }
    virtual ~XmlWriteException() throw ()
    {
    }
    ErrCode getErrCode() const
    {
      return m_eErrCode;
    }
    const std::string& getPath() const
    {
      return m_strPath;
    }
    virtual void explain(std::stringstream& ss) const
    {
      switch (m_eErrCode)
      {
      case Ok:
        break;
      case ValueAndElements:
        ss << "element has value and child elements at '" << getPath();
        break;
      default:
        ss << "(unknown error) at " << getPath();
      }
    }
  private:
    ErrCode m_eErrCode;
    std::string m_strPath;
  };

  /// XML read and write functions for DOM streams
  namespace xml
  {
    struct Context
    {
      Context() :
        m_unLine(0), m_unColumn(0)
      {
      }
      unsigned int m_unLine;
      unsigned int m_unColumn;
    };
  }
  /** @brief Exception thrown by the XmlIStream class
   *  @ingroup XmlStreams
   */
  class XmlParseException: public Exception
  {
  public:
    enum ErrCode
    {
      Ok, CharExpected, ValueExpected, NameExpected, WrongCloseTag
    };
    explicit XmlParseException(ErrCode eErrCode, const xml::Context& context, char chChar = 0) :
      m_eErrCode(eErrCode), m_cContext(context), m_chChar(chChar)
    {
    }
    virtual ~XmlParseException() throw ()
    {
    }
    ErrCode getErrCode() const
    {
      return m_eErrCode;
    }
    char getChar() const
    {
      return m_chChar;
    }
    virtual void explain(std::stringstream& ss) const
    {
      switch (m_eErrCode)
      {
      case Ok:
        break;
      case CharExpected:
        ss << "character '" << getChar() << "' expected at line " << m_cContext.m_unLine << " in column " << m_cContext.m_unColumn;
        break;
      case ValueExpected:
        ss << "value expected at line " << m_cContext.m_unLine << " in column " << m_cContext.m_unColumn;
        break;
      case NameExpected:
        ss << "name expected at line " << m_cContext.m_unLine << " in column " << m_cContext.m_unColumn;
        break;
      case WrongCloseTag:
        ss << "wrong close tag at line " << m_cContext.m_unLine << " in column " << m_cContext.m_unColumn;
        break;
      default:
        ss << "(unknown error) at line " << m_cContext.m_unLine << " in column " << m_cContext.m_unColumn;
      }
    }
  private:
    ErrCode m_eErrCode;
    xml::Context m_cContext;
    char m_chChar;
  };

  namespace xml
  {
    template<class O> void writeHeader(O& os, const std::string& strVersion = "1.0", const std::string& strEncoding = "UTF-8", const std::string& strLineFeed = "\n")
    {
      os << "<?xml version=\"" << strVersion << "\" encoding=\"" << strEncoding << "\"?>" << strLineFeed;
    }
    /** @brief writes a node and all children into an output stream.
     *  @param os output stream
     *  @param pNode Node to write.
     *  @param strLineFeed For better reading you can insert line feeds with
     *         this parameter. This should be XML complaint "\n" or "" if you
     *         don't want line feeds in your output (this is the default).
     *  @param strIndent Also for better reading one can set an indentation
     *         string with this parameter. This might be "\t", " ", "  "
     *         or "" which is the default for no indentation.
     *  @param nDeepness Deepness in the node tree at the current write
     *         position. This value will be used to generate indentations.
     *  @param nFewAttributes This parameter sets the maximum amount of
     *         attributes in a node that will be formatted without including
     *         line feeds.
     *  @param bShowHidden show nodes that were created with domhattr() or domhopen()
     */
    template<class O> void write(O& os, DomNode* pNode, const std::string& strLineFeed = "", const std::string& strIndent = "", int nDeepness = 0, unsigned int nFewAttributes = 1,
        bool bShowHidden = false)
    {
      // hide hidden values
      if ((!bShowHidden && pNode->isHidden()) || pNode->isMissing())
        return;
      if (pNode->isAttribute())
      {
        os << strLineFeed;
        for (int i = 0; i < nDeepness; i++)
          os << strIndent;
        os << pNode->getName() << "=\"" << pNode->getValueStr() << "\"";
      }
      else
      {
        for (int i = 0; i < nDeepness; i++)
          os << strIndent;
        os << "<" << pNode->getName();
        bool bOnlyAttributes = pNode->getValueStr().empty();
        bool bFewAttributes = pNode->attributes() <= nFewAttributes;
        for (DomOStream::iterator it = pNode->begin(); it != pNode->end(); it++)
        {
          if ((*it)->isAttribute())
          {
            if (bFewAttributes)
              write(os, *it, " ", "", 0, nFewAttributes, bShowHidden);
            else
              write(os, *it, strLineFeed, strIndent, nDeepness + 1, nFewAttributes, bShowHidden);
          }
          else if ((bShowHidden || !(*it)->isHidden()) && !pNode->isMissing())
            bOnlyAttributes = false;
        }
        if (bOnlyAttributes)
          os << "/>" << strLineFeed;
        else
        {
          os << ">";
          if (pNode->getValueStr().empty())
          {
            os << strLineFeed;
            for (DomOStream::iterator it = pNode->begin(); it != pNode->end(); it++)
            {
              if (!(*it)->isAttribute())
              {
                BOOST_ASSERT(pNode->getValueStr().empty());
                write(os, *it, strLineFeed, strIndent, nDeepness + 1, nFewAttributes, bShowHidden);
              }
            }
            for (int i = 0; i < nDeepness; i++)
              os << strIndent;
          }
          else
          {
            if (!pNode->hasOnlyAttributes())
              TBD_THROW(XmlWriteException(XmlWriteException::ValueAndElements, pNode));
            os << pNode->getValueStr();
          }
          os << "</" << pNode->getName() << ">" << strLineFeed;
        }
      }
    }

    /** @brief Writes the DOM of this instance in XML to a given output stream.
     *  @param os Output stream to write to
     *  @param dos DOM stream to write
     *  @param strLineFeed For better reading you can insert line feeds with
     *         this parameter. This should be XML complaint "\n" or "" if you
     *         don't want line feeds in your output (this is the default).
     *  @param strIndent Also for better reading one can set an indentation
     *         string with this parameter. This might be "\t", " ", "  "
     *         or "" which is the default for no indentation.
     *  @param nFewAttributes This parameter sets the maximum amount of
     *         attributes in a node that will be formatted without including
     *         line feeds.
     *  @param bShowHidden show children created with domhattr() and domhopen()
     */
    template<class O> void write(O& os, DomOStream& dos, const std::string& strLineFeed = "\n", const std::string& strIndent = "  ", unsigned int nFewAttributes = 1,
        bool bShowHidden = false)
    {
      // Enumerate all children of the internal root node
      for (DomOStream::iterator it = dos.getRoot()->begin(); it != dos.getRoot()->end(); it++)
        // write the node into the output stream
        write(os, *it, strLineFeed, strIndent, 0, nFewAttributes, bShowHidden);
    }
    template<class O, class T> void write(O& os, const T& t, const std::string& strLineFeed = "\n", const std::string& strIndent = "  ", unsigned int nFewAttributes = 1, bool bShowHidden = false)
    {
      DomOStream dos;
      dos << domopen(t.classname()) << t << domclose();
      write(os, dos, strLineFeed, strIndent, nFewAttributes, bShowHidden);
    }
    template<class T> std::string str(const T& t, const std::string& strLineFeed = "\n", const std::string& strIndent = "  ", unsigned int nFewAttributes = 1, bool bShowHidden = false)
    {
      std::stringstream ss;
      write(ss, t, strLineFeed, strIndent, nFewAttributes, bShowHidden);
      return ss.str();
    }
  }
  namespace xml
  {
    namespace details
    {
      template<class I> int get(I& is, Context& context)
      {
        int n = is.get();
        if ('\n' == n)
        {
          context.m_unLine++;
          context.m_unColumn = 0;
        }
        else
          context.m_unColumn++;
        return n;
      }
      template<class I> void skip(I& is, const std::string& strWhitespaces, Context& context)
      {
        while (strWhitespaces.find((char) is.peek()) != std::string::npos)
          get(is,context);
      }
      template<class I> void expect(I& is, const char ch, Context& context)
      {
        if (ch != get(is,context))
          TBD_THROW(XmlParseException(XmlParseException::CharExpected, context, ch));
      }
      template<class I> void expect(I& is, std::string str, Context& context)
      {
        for (std::string::iterator it = str.begin(); it != str.end(); it++)
          expect(is, *it, context);
      }
      template<class I> bool readname(I& is, DomNode* pNode, Context& context)
      {
        std::string str;
        for (;;)
        {
          int n = is.peek();
          switch (n)
          {
          case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
          case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
          case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
          case '_': case ':':
            str += (char) get(is,context);
            n = is.peek();
            break;
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            if (str.empty())
              return false;
            str += (char) get(is,context);
            n = is.peek();
            break;
          default:
            pNode->setName(str);
            return !str.empty();
          }
        }
      }
      template<class I> bool readquotes(I& is, DomNode* pNode, Context& context)
      {
        if ('\"' != is.peek())
          return false;
        get(is,context);
        std::string str;
        while ('\"' != is.peek())
          str += (char) get(is,context);
        pNode->setValueStr(str);
        get(is,context);
        return true;
      }
      template<class I> bool readvalue(I& is, DomNode* pNode, const std::string& strWhitespaces, Context& context)
      {
        std::string str;
        for (;;)
        {
          if ('<' == is.peek())
          {
            get(is,context);
            if ('/' == is.peek())
            {
              is.unget();
              break;
            }
            else
              str += '<';
          }
          str += (char) get(is,context);
        }
        // remove all white spaces from the value's tail
        std::string::size_type pos = str.find_last_not_of(strWhitespaces);
        if (std::string::npos != pos)
          str.erase(pos + 1);
        pNode->setValueStr(str);
        // success
        return true;
      }
      template<class I> bool readattr(I& is, DomNode* pNode, const std::string& strWhitespaces, Context& context)
      {
        skip(is, strWhitespaces,context);
        switch (is.peek())
        {
        case '?':
        case '>':
        case '/':
          return false;
        default:
          {
            DomNode *pChild = new DomNode(ATTRIBUTE);
            if (!readname(is, pChild,context))
              TBD_THROW(XmlParseException(XmlParseException::NameExpected, context));
            skip(is, strWhitespaces,context);
            expect(is, '=',context);
            skip(is, strWhitespaces,context);
            if (!readquotes(is, pChild,context))
              TBD_THROW(XmlParseException(XmlParseException::ValueExpected, context));
            pNode->push_back(pChild);
            return true;
          }
        }
      }
      template<class I> void read(I& is, DomNode* pNode, const std::string& strWhitespaces, Context& context)
      {
        skip(is, strWhitespaces, context);
        expect(is, '<',context);
        skip(is, strWhitespaces, context);
        if ('!' == is.peek())
        {
          bool bComment = true;
          get(is,context);
          expect(is, '-',context);
          expect(is, '-',context);

          while (bComment)
          {
            while ('-' != is.peek())
              get(is,context);
            get(is,context);
            if ('-' == is.peek())
              bComment = false;
          }
          get(is,context);
          expect(is, '>',context);
        }
        else
        {
          if ('?' == is.peek())
          {
            get(is,context);
            pNode->setName("?");
          }
          readname(is, pNode,context);
          skip(is, strWhitespaces, context);
          switch (is.peek())
          {
          case '?':
            if (pNode->getName()[0] != '?')
              TBD_THROW(XmlParseException(XmlParseException::WrongCloseTag, context));
            // no break
          case '/':
            get(is,context);
            skip(is, strWhitespaces,context);
            expect(is, '>',context);
            break;
          default:
            while (readattr(is, pNode, strWhitespaces,context))
              ;
            skip(is, strWhitespaces, context);
            switch (is.peek())
            {
            case '?':
            case '/':
              get(is,context);
              skip(is, strWhitespaces,context);
              expect(is, '>',context);
              break;
            default:
              expect(is, '>',context);
              skip(is, strWhitespaces,context);
              switch (is.peek())
              {
              case '<':
                while (readchild(is, pNode, strWhitespaces, context))
                  skip(is, strWhitespaces, context);
              default:
                readvalue(is, pNode, strWhitespaces,context);
              }
              skip(is, strWhitespaces, context);
              expect(is, '<',context);
              skip(is, strWhitespaces, context);
              expect(is, '/',context);
              skip(is, strWhitespaces, context);
              expect(is, pNode->getName(),context);
              skip(is, strWhitespaces, context);
              expect(is, '>',context);
            }
          }
        }
      }
      template<class I> bool readchild(I& is, DomNode* pNode, const std::string& strWhitespaces, Context& context)
      {
        skip(is, strWhitespaces, context);
        if ('<' != is.peek())
          return false;
        get(is,context);
        if ('/' != is.peek())
        {
          is.unget();
          DomNode *pChild = new DomNode(OPEN);
          read(is, pChild, strWhitespaces,context);
          if (!pChild->getName().empty())
            pNode->push_back(pChild);
          return true;
        }
        else
        {
          is.unget();
          return false;
        }
      }
    }
    template<class I> void read(I& is, DomIStream& dis, const std::string& strWhiteSpaces = " \r\n\t")
    {
      Context context;
      while (details::readchild(is, dis.getRoot(), strWhiteSpaces, context))
        ;
    }
    template<class I> void read(I& is, DomNode* pNode, const std::string& strWhitespaces = " \r\n\t")
    {
      Context context;
      details::read(is,pNode,strWhitespaces,context);
    }
    template<class I, class T> void read(I& is, T& t, const std::string& strWhitespaces = " \r\n\t")
    {
      DomIStream dis;
      read(is, dis, strWhitespaces);
      dis >> domopen(t.classname()) >> t >> domclose();
    }
  }
}

#ifdef _MSC_VER
# pragma warning(default:4290)
#endif
#endif
