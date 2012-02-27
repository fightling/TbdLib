///////////////////////////////////////////////////////////////////////////////
/// @file nulstream.h
/// @brief NUL output stream class
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__NULSTREAM_H
#define __TBD__NULSTREAM_H

#include <algorithm>

/// @defgroup NulStream NUL Stream
/// @brief Streaming class that throws away output but counts written bytes
/// @details
/// usefull for determining the size of your streamable stuctures).
/// @ingroup Streams
namespace tbd
{
  /// @ingroup NulStream
  template <class SP=size_t> class NulOStream
    : public OStream<SP>
  {
    typedef SP streampos;
    streampos m_size;
    streampos m_pos;
  public:
    NulOStream() : m_size(0), m_pos(0) {}
    /// @brief Write size bytes from pch into this stream.
    /// @param size Size of the content.
    virtual void write( const char*, size_t size )
    { 
      m_pos += size;
#ifdef max
      m_size = max(m_size,m_pos);
#else
      m_size = std::max(m_size,m_pos);
#endif
    }
    virtual void flush() { }
    /// @brief Returns the current read position.
    virtual streampos tellp() const { return m_pos; }
    /// @brief Sets the current read position.
    virtual void seekp(const streampos& p) { m_pos=p; }
    virtual void seekp2end() { m_pos=m_size; }
    streampos size() const { return m_size; }
  };
}
#endif
