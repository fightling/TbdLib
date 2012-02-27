///////////////////////////////////////////////////////////////////////////////
/// @file dump.h
/// @brief Dump tools
/// @author Patrick Hoffmann
/// @date 24.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__DUMP_H
#define __TBD__DUMP_H

#include <iostream>
#include <sstream>
#include "stream.h"
#include <boost/static_assert.hpp>

/// @defgroup dump Dumping
/// @brief Methods and classes that help you to dump memory into an human
///        readable form.
/// @ingroup tbd

namespace tbd
{
  /// @brief Dumping functions
  namespace dump
  {
    /// @ingroup dump
    template<class T> void _hex(T& os, const char& ch)
    {
      static char s_aHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
      os << s_aHexDigits[(unsigned char) ch >> 4] << s_aHexDigits[(unsigned char) ch & 0xF];
    }
    /// @ingroup dump
    template<class T> void _hex(T& os, const unsigned char& uch)
    {
      _hex(os, (char) uch);
    }
    /// @ingroup dump
    template<class T, class V> void _hex(T& os, const V& v)
    {
      for (unsigned int i = 0; i < sizeof(V); i++)
        _hex(os, ((unsigned char*) &v)[i]);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const unsigned char& uch)
    {
      _hex(os, uch);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const short& s)
    {
      _hex(os, s);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const unsigned short& us)
    {
      _hex(os, us);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const long& l)
    {
      _hex(os, l);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const unsigned long& ul)
    {
      _hex(os, ul);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const int& n)
    {
      _hex(os, n);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const unsigned int& un)
    {
      _hex(os, un);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const long long& ll)
    {
      _hex(os, ll);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const unsigned long long& ull)
    {
      _hex(os, ull);
    }
    /// @ingroup dump
    template<class T> void hex(T& os, const char *pch, size_t count)
    {
      for (unsigned int i = 0; i < count; i++)
      {
        if (i > 0)
          os << " ";
        _hex(os, *pch++);
      }
    }
    /// @ingroup dump
    template<class T> __inline void hex(T& os, const std::vector<T>& vec)
    {
      for (unsigned int i = 0; i < vec.size(); i++)
      {
        if (i > 0)
          os << " ";
        _hex(os, vec[i]);
      }
    }
    /// @ingroup dump
    template<class T> void hex(T& os, std::istream& is)
    {
      std::streampos pos = is.tellg();
      is.seekg(0);
      while (!is.eof())
        dump::_hex(os, (char) is.get());
      is.seekg(pos);
    }
    /// @ingroup dump
    template<class T> void ascii(T& os, char ch)
    {
      if ((unsigned char) ch >= 32 && (unsigned char) ch < 128)
        os << ch;
      else
        os << '.';
    }
    /// @ingroup dump
    template<class T> void ascii(T& os, const char *pch, size_t count)
    {
      for (unsigned int i = 0; i < count; i++)
        ascii(os, *pch++);
    }
    template<class T> void hex_ascii(T& os, const char* pch, size_t count, const std::string& strLineFeed = "\n", unsigned int unLineLength = 16)
    {
      unsigned int i = 0;
      if (count > unLineLength)
      {
        for (; i + unLineLength - 1 < count; i += unLineLength)
        {
          if (i > 0)
            os << strLineFeed;
          tbd::dump::hex(os, pch + i, unLineLength);
          os << "  ";
          tbd::dump::ascii(os, pch + i, unLineLength);
        }
      }
      if (i < count)
      {
        if (i > 0)
          os << strLineFeed;
        tbd::dump::hex(os, pch + i, count - i);
        os << std::string(3 * ((unLineLength - (count + i) % unLineLength) % unLineLength), ' ') << "  ";
        tbd::dump::ascii(os, pch + i, count - i);
      }
    }
    inline std::string hex(const char *pch, size_t count)
    {
      std::stringstream ss;
      hex(ss, pch, count);
      return ss.str();
    }
    template<class T> std::string hex_ascii(const T* p, size_t count, const std::string& strLineFeed = "\n", unsigned int unLineLength = 16)
    {
      std::stringstream ss;
      hex_ascii(ss, (char*) p, count / sizeof(T), strLineFeed, unLineLength);
      return ss.str();
    }
    template<class T> std::string hex(const T& t)
    {
      std::stringstream ss;
      hex(ss, t);
      return ss.str();
    }

    /// @ingroup dump
    template<class T> void _binary(T& os, unsigned char ch, int bits = 8)
    {
      if (bits > 8)
        bits = 8;
      for (int i = bits - 1; i >= 0; --i)
      {
        os << (((ch & (1 << i)) != 0) ? '1' : '0');
      }
    }
    /// @ingroup dump
    template<class T> void _binary(T& os, char ch, int bits = 8 * sizeof(char))
    {
      _binary(os, (unsigned char) ch,bits);
    }

    template<class T, class V> void _binary(T& os, const V& v, int bits = 8 * sizeof(V))
    {
      for (unsigned int i = 0; i < sizeof(V) && bits > 0; i++, bits -= 8 * sizeof(unsigned char))
        _binary(os, ((unsigned char*) &v)[i], bits);
    }
    template<class T> void binary(T& os, const short& s, int bits = 8 * sizeof(short))
    {
      _binary(os, s, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const unsigned short& us, int bits = 8 * sizeof(unsigned short))
    {
      _binary(os, us, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const long& l, int bits = 8 * sizeof(long))
    {
      _binary(os, l, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const unsigned long& ul, int bits = 8 * sizeof(unsigned long))
    {
      _binary(os, ul, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const int& n, int bits = 8 * sizeof(int))
    {
      _binary(os, n, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const unsigned int& un, int bits = 8 * sizeof(unsigned int))
    {
      _binary(os, un, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const long long& ll, int bits = 8 * sizeof(long long))
    {
      _binary(os, ll, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const unsigned long long& ull, int bits = 8 * sizeof(unsigned long long))
    {
      _binary(os, ull, bits);
    }
    /// @ingroup dump
    template<class T> void binary(T& os, const char *pch, int bits)
    {
      for (unsigned int i = 0; bits>0; i++)
      {
        if (i > 0)
          os << " ";
        _binary(os, *pch++, bits);
        bits -= 8*sizeof(char);
      }
    }
    /// @ingroup dump
    template<class T> __inline void binary(T& os, const std::vector<T>& vec)
    {
      for (unsigned int i = 0; i < vec.size(); i++)
      {
        if (i > 0)
          os << " ";
        _binary(os, vec[i]);
      }
    }
  }

  /// @brief A filter that you can attach between streams. It converts all
  ///        your binary output into human readable (hexadecimal) output.
  /// @ingroup dump
  template<class OS, class T = unsigned long, bool ASCII = false>
  class Hexifier: public OStream<T>
  {
  IMPLEMENT_OSTREAM_OPERATORS(Hexifier,T)
    ;
  public:
    Hexifier(OS& os) :
      m_os(os), m_unLineLength(16)
    {
    }
    Hexifier(OS& os, const std::string& strLineFeed, unsigned int unLineLength = 16) :
      m_os(os), m_strLineFeed(strLineFeed), m_unLineLength(unLineLength)
    {
      BOOST_STATIC_ASSERT(ASCII);
    }
    virtual void write(const char* pch, size_t uiSize)
    {
      if (ASCII)
        dump::hex_ascii(m_os, pch, uiSize, m_strLineFeed, m_unLineLength);
      else
        dump::hex(m_os, pch, uiSize);
    }
    virtual void flush()
    {
      m_os.flush();
    }
    /// @brief Returns the current read position.
    virtual typename OStream<T>::streampos tellp() const
    {
      return m_os.tellp();
    }
    /// @brief Sets the current read position.
    virtual void seekp(const typename OStream<T>::streampos& p)
    {
      m_os.seekp(p);
    }
    virtual void seekp2end()
    {
      BOOST_ASSERT(0);
    }
  private:
    OS& m_os;
    std::string m_strLineFeed;
    unsigned int m_unLineLength;
  };
}

#endif
