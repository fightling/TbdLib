///////////////////////////////////////////////////////////////////////////////
/// @file network.h
/// @brief Network byte order helpers
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#include <boost/assert.hpp>
#include <vector>

#ifndef __TBD__NETWORK_H
#define __TBD__NETWORK_H

#ifdef _MSC_VER
# pragma warning(disable:4127)
#endif

/// @defgroup network Network
/// @brief Network related helpers
/// @ingroup tbd
namespace tbd
{
  /// @brief This method template converts a big endian scalar type to
  /// little endian or backwards.
  /// @param t The item to convert.
  /// @return The converted item.
  /// @todo The current implementation works only for 1, 2, 4 and 8 byte values
  ///       (ok, 1 is trivial!).
  /// @ingroup Network
  template<class T> __inline void swapEndian(T& t)
  {
    if( sizeof(T) > 1 )
    {
      if( sizeof(T) == 2 )
        std::swap(((char*)&t)[1],((char*)&t)[0]);
      else if( sizeof(T) == 4 )
      {
        std::swap(((char*)&t)[3],((char*)&t)[0]);
        std::swap(((char*)&t)[2],((char*)&t)[1]);
      }
      else if( sizeof(T) == 8 )
      {
        std::swap(((char*)&t)[7],((char*)&t)[0]);
        std::swap(((char*)&t)[6],((char*)&t)[1]);
        std::swap(((char*)&t)[5],((char*)&t)[2]);
        std::swap(((char*)&t)[4],((char*)&t)[3]);
      }
      else
        // unknown type size
        BOOST_ASSERT(0);
    }
  }

#define TBD_LITTLE_ENDIAN

  /// @brief Converts an item from host to network byte order on little endian
  ///        systems. On big endian systems it does nothing!
  /// @param t The item to convert.
  /// @return The item in network byte order
  /// @ingroup Network
#ifdef TBD_LITTLE_ENDIAN
  template<class T> __inline T host2net(const T& t) { T r=t; swapEndian(r); return r; }
#else
  template<class T> __inline T host2net(const T& t) { return t; }
#endif

  /// @brief Converts an item from network to host byte order on little endian
  ///        systems. On big endian systems it does nothing!
  /// @param t The item to convert.
  /// @return The item in host byte order
  /// @ingroup Network
  template<class T> __inline T net2host(const T& t) { return host2net(t); }

  /// @brief reads an item byte wise in network byte and copies its content
  /// into a std::vector in host byte order. On big endian systems it does
  /// nothing!
  /// @ingroup Network
#ifdef TBD_LITTLE_ENDIAN
  template<class T> __inline void net2host(std::ostream& os, const T& t, unsigned int uBytes=sizeof(T))
  {
    BOOST_ASSERT(uBytes<=sizeof(T));
    for( int i=uBytes-1; i>=0; i-- )
      os.put(((char*)&t)[i]);
  }
#else
  template<class T> __inline void net2host(std::ostream& os, const T& t, unsigned int uBytes=sizeof(T))
  {
    BOOST_ASSERT(0); // untested
    BOOST_ASSERT(uBytes<=sizeof(T));
    for( int i=sizeof(t)-uBytes; i<sizeof(t); i++ )
      os.put(((char*)&t)[i]);
  }
#endif

  /// @brief reads an item byte wise in host byte order and copies its
  /// content into a std::vector in network byte order. On big endian systems it
  /// does nothing!
  /// @ingroup Network
  template<class T> __inline void host2net(std::ostream& os, const T& t, unsigned int uBytes=sizeof(T))
  { net2host(os,t,uBytes); }

  /// @ingroup Network
#ifdef TBD_LITTLE_ENDIAN
  template<class T> __inline int host2net(T& t, std::istream& is )
  {
    int i;
    for( i=sizeof(t)-1; i>=0 && !is.eof(); i-- )
      ((char*)&t)[i] = (char)is.get();
    return sizeof(t)-i-1;
  }
#else
  template<class T> __inline int net2host(T& t, std::istream& is )
  {
    int i;
    for( i=0; i<sizeof(t) && is.good(); i++ )
      ((char*)&t)[i] = (char)is.get();
    return i-1;
  }
#endif

  /// @ingroup Network
  template<class T> __inline int net2host(T& t, std::istream& is )
  { return host2net(t,is); }
}

#ifdef _MSC_VER
# pragma warning(default:4127)
#endif


#endif
