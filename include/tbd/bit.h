///////////////////////////////////////////////////////////////////////////////
/// @file bit.h
/// @brief Tools for bit operations and type properties
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////
#include <boost/assert.hpp>

#ifndef __TBD__BIT_H
#define __TBD__BIT_H

namespace tbd
{

  /// @defgroup Bit Bit Tools
  /// @brief Classes and functions that manage bit content
  /// @ingroup BitStreams

  /// @brief Classes and functions that manage bit content
  /// @ingroup Bit
  namespace bit
  {
    /// @brief Small bit count.
    /// @ingroup Bit
    class Count
    {
      size_t m_unCount;
    public:
      Count( size_t unCount ) : m_unCount(unCount) {}
      Count( const Count& cBitCount ) : m_unCount(cBitCount.m_unCount) {}
      Count& operator=( const Count& cBitCount ) { m_unCount = cBitCount.m_unCount; return *this; }
      const Count& operator+=( const Count& cBitCount ) { m_unCount += cBitCount.m_unCount; return *this; }
      const Count& operator++(int) { m_unCount++; return *this; }
      const Count& operator-=( const Count& cBitCount ) { m_unCount -= cBitCount.m_unCount; return *this; }
      operator size_t () const { return (size_t)m_unCount; }
      bool aligned( const Count& cBitCount ) const { return m_unCount % cBitCount.m_unCount == 0; }
    };

    /// @brief Checks if type T is signed
    /// @ingroup Bit
    template<class T> __inline bool is_type_signed() { BOOST_ASSERT(0); return false; }
    template<> __inline bool is_type_signed<char>() { return true; }
    template<> __inline bool is_type_signed<unsigned char>() { return false; }
    template<> __inline bool is_type_signed<short>() { return true; }
    template<> __inline bool is_type_signed<unsigned short>() { return false; }
    template<> __inline bool is_type_signed<long>() { return true; }
    template<> __inline bool is_type_signed<unsigned long>() { return false; }
    template<> __inline bool is_type_signed<long long>() { return true; }
    template<> __inline bool is_type_signed<unsigned long long>() { return false; }
    template<> __inline bool is_type_signed<int>() { return true; }
    template<> __inline bool is_type_signed<unsigned int>() { return false; }

    template<class U, class S> U tou( S s ) { BOOST_ASSERT(s>=0); return (U)s; }

    /// @brief copy lower bits of a value into another value by inserting 
    /// them at the lower side.
    /// @param s Destination buffer of type S.
    /// @param t Source buffer of type T.
    /// @param uCount Number of bits to copy.
    /// @param uStart Starting bit position in t for copy.
    /// @ingroup Bit
    template<class S, class T> 
    __inline void copyBits( S& s, const T& t, const Count& uCount, const Count& uStart=0 )
    { 
      // uCount has to be smaller than destination buffer
      BOOST_ASSERT(uCount<=sizeof(S)*8); 
      // this method runs only if uCount doesn't exceeds bit size of unsigned
      // int
      BOOST_ASSERT(uCount<=sizeof(unsigned int)*8); 
      // t has to be large enough
      BOOST_ASSERT(uStart+uCount<=sizeof(T)*8);
      // if uCount matches the number of bits in unsigned int
      if( uCount == sizeof(unsigned int)*8 )
        // reserve uCount bits in s and OR it with the significant part of the
        // value
        s = (S)((s << uCount) | (S)((t >> uStart))); 
      else
      {
        // create a mask for the shifted source value
        const unsigned int tMask = ~(~(unsigned int)0 << uCount);
        // reserve uCount bits in s and OR it with the significant part of the
        // value
        s = (S)((s << uCount) | ((S)((t >> uStart) & tMask))); 
      }
    }
    /// @brief converts a bit count value of type T into the corresponding 
    /// byte count of type T and asserts when the bit count isn't byte 
    /// aligned
    /// @ingroup Bit
    template<class T> __inline T tobyte( const T& t )
    {
      BOOST_ASSERT(0==t%8);
      return t/(T)8;
    }
    template<class T> __inline T tobit( const T& t )
    {
      return t*(T)8;
    }
    /// @brief Returns the number of "used" bits in a value
    /// @ingroup Bit
    template<class T> __inline unsigned char usedBits( T ulValue )
    {
      unsigned char i=0;
      while( ulValue>0 )
      {
        ulValue >>= 1;
        i++;
      }
      return i;
    }
    /// @ingroup Bit
    /// @brief Returns the bitmask for a given type or instance.
    template<class T> __inline T bitmask(const T& =T(0)) { return (T)~T(0); }
    namespace intern 
    {
      template<typename T> struct Type2Type { typedef T OriginalType; };
      template<class T, class V> __inline T get( const V& v, unsigned int from, unsigned int num, Type2Type<T> ) 
      { return (T)((v >> from) & (bitmask(v) >> (sizeof(V)*8-num))); }
      template<class T, class V> __inline bool get( const V& v, unsigned int from, unsigned int num, Type2Type<bool> ) 
      { return 0 != (unsigned int)((v >> from) & (bitmask(v) >> (sizeof(V)*8-num))); }
    }
    template<class T, class V> __inline T get( const V& v, unsigned int from, unsigned int num ) 
    { return intern::get<T>(v,from,num,intern::Type2Type<T>()); }
    template<class T, class V> __inline void set( V &v, const T& t, unsigned int from, unsigned int num ) 
    { v = (v & ~((bitmask<V>() >> (sizeof(V)*8-num)) << from)) | (((V)t & (bitmask<V>() >> (sizeof(V)*8-num))) << from); }
  }
}

#endif
