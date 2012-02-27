///////////////////////////////////////////////////////////////////////////////
/// @file bitstream.h
/// @brief Bitwise I/O streams that can be attached to basic stream inteface
///         implementions (including ostream/istream, that are compatible)
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__BITSTREAM_H
#define __TBD__BITSTREAM_H

#define _SCL_SECURE_NO_DEPRECATE
#include <stdexcept>
#include <sstream>
#include "network.h"
#include "dump.h"
#include "exception.h"
#include "bit.h"
#include "string.h"

#ifdef _MSC_VER
# pragma warning(disable:4512 4127)
#endif

#define __BITSTEAM_INLINE __inline

/// @defgroup BitStreams Bit Streams
/// @brief Bit streams that can be used to read/write binary data on bit
///        level.
/// @ingroup Streaming
/// @details
/// @par Purpose
/// These classes provide you bitwise input and output streaming.
/// @par Usage
/// Bit streams behave like filters. They have to be attached to some
/// real input or output stream to work. You might attach them to some
/// std::istream, std::ostream or tbd::IStream, tbd::OStream derived
/// classes.
/// For example we use a memory input/output stream to write some things
/// into and read them back
/// @code
///   // memory stream to write into and read from
///   tbd::MemStream<> ms;
/// @endcode
/// Here is how reading works:
/// @code
///   // create a bit output stream to write bitwise items into mos
///   tbd::BitOStream< tbd::MemStream<> > bos(ms);
///
///   // write 4 bytes with 0xdeadbeef
///   bos << 0xdeadbeef;
///   // write 3 bits of 0xFF
///   bos << tbd::bits(0xFF,3);
///   // write 5 bits of 0
///   bos << tbd::bits(0,5);
///   // write zero terminated string
///   bos << "Text";
///   // write 4 bytes of "Texts" (-> "Text")
///   bos << tbd::bitstr("Texts",4);
///   // write 8 bytes of "Texts" and fill up with '\0' (-> "Text\0\0\0\0")
///   bos << tbd::bitstr("Text",8);
///   // write prefix length of "Text" as byte and then write "Text"
///   // without termination
///   bos << tbd::bitstr<unsigned char>("Text");
/// @endcode
/// And now let's read it back:
/// @code
///   // create a bit output stream to write bitwise items into mos
///   tbd::BitIStream< tbd::MemStream<> > bis(ms);
///
///   // dummy variables to read into
///   unsigned long ul;
///   std::string   str;
///
///   // read 4 bytes
///   bis >> ul;
///   // read 3 bits
///   bis >> tbd::bits(ul,3);
///   // read 5 bits
///   bis >> tbd::bits(ul,5);
///   // read zero terminated string
///   bis >> str;
///   // read 12 bytes
///   bis >> tbd::bitstr(str,12);
///   // read string with length prefix
///   bis >> tbd::bitstr<unsigned char>(str);
/// @endcode
/// @see BitStreamExamples
namespace tbd
{
  /// @brief Base class of BitOStream and BitIStream that manages the bit cache.
  /// @ingroup BitStreams
  class BitStream
  {
  public:
    typedef long long   streampos;
    /// @brief Returns true, if the bit stream is currently byte aligned
    /// and false otherwise.
    __BITSTEAM_INLINE bool aligned( unsigned char uchBits=8 ) const { return m_uCount.aligned(uchBits); }
  protected:
    /// @brief Type to use as bit cache.
    typedef unsigned int        T;
    /// @brief Constructor
    BitStream() : m_tCurrent(0), m_uCount(0) { }
    /// @brief Return the bit cache.
    __BITSTEAM_INLINE const T& current() const { return m_tCurrent; }
    /// @brief Return the current bit cache size.
    __BITSTEAM_INLINE const bit::Count& count() const { return m_uCount; }
    /// @brief Returns true, if bit cache contains no bits.
    __BITSTEAM_INLINE bool empty() const { return m_uCount == 0; }
    /// @brief Returns the number of currently free bits in bit cache.
    __BITSTEAM_INLINE bit::Count left() const { return sizeof(T)*8-m_uCount; }
    /// @brief Convert the bit cache into network byte order.
    __BITSTEAM_INLINE void host2net() { m_tCurrent = tbd::host2net(m_tCurrent); }
    /// @brief Convert the bit cache into host byte order.
    __BITSTEAM_INLINE void net2host() { m_tCurrent = tbd::net2host(m_tCurrent); }
    /// @brief Return the bit cache.
    __BITSTEAM_INLINE T& current() { return m_tCurrent; }
    /// @brief Set the bit cache and it's size.
    __BITSTEAM_INLINE void set(const T t, const bit::Count c) { m_tCurrent=t; m_uCount=c; }
    /// @brief Set size of the bit cache.
    __BITSTEAM_INLINE void setCount( const bit::Count& c) { m_uCount = c; }
    __BITSTEAM_INLINE void setCount( size_t c) { m_uCount = (unsigned int)c; }
    /// @brief Adds an amount to the size of the bit cache.
    __BITSTEAM_INLINE void addCount( const bit::Count& c) { m_uCount += c; }
    /// @brief Subtracts an amount from the size of the bit cache.
    __BITSTEAM_INLINE void subCount( const bit::Count& c) { BOOST_ASSERT(m_uCount>=c); m_uCount -= c; }
  private:
    /// @brief Bit cache in host byte order.
    T             m_tCurrent;
    /// @brief Current size of the bit cache.
    bit::Count      m_uCount;
  };

  struct NulConfig
  {
  };
  /// @brief Bit output stream
  /// @ingroup BitStreams
  template<class O, class C=NulConfig>
  class BitOStream
    : public BitStream
    , public C
  {
  public:
    /// @brief Configures this output bit stream to write all the results
    /// into a specific output stream of type O.
    BitOStream( O& os ) : m_os(os) { }
    ~BitOStream() { flush2stream(); }
    O& ostream() { flush2stream(); seekp(tellp()); return m_os; }
    /// @brief Before this method flushes this stream it may append bits to
    /// fill up the stream to the next byte alignment.
    /// @param bFillBit If true, a possible fill-up will be made by appending
    ///        set bits. If false, it appends cleared bits.
    __BITSTEAM_INLINE void flush( bool bFillBit )
    {
      // if not aligned
      if( !aligned() )
        // fill up bits to next byte alignment
        put(bFillBit?0xFF:0x00,(8-(count()%8))%8);
      // flush the bit stream that is byte aligned now
      flush();
    }
    /// @brief Flushes this stream and calls O::flush() for the destination
    /// stream.
    /// @attention This method crashes with assertion if this stream
    /// currently is not byte aligned! Use flush(bool) for automatic byte
    /// alignment.
    __BITSTEAM_INLINE void flush()
    {
      // flush bit cache to stream
      flush2stream();
      // flush the destination stream
      m_os.flush();
    }
    /// @brief Put some elements of type S into this bit stream.
    /// @param psItems Pointer to the first item.
    /// @param unNumItems Number of items to put.
    /// This method writes each item in network byte order into the destination
    /// stream. This method is much faster with bit streams that are currently
    /// byte aligned!
    template<class S>
    __BITSTEAM_INLINE void putn( const S* const psItems, size_t unNumItems )
    {
      // check if this stream is currently byte aligned
      if( aligned() )
      {
        // flush the bit cache
        flush2stream();
        // if S is doesn't need byte order conversion
        if( sizeof(S) == 1 )
        {
          // write the items directly (fastest) into the destination stream
          m_os.write((const char*)psItems,unNumItems);
        }
        else
        {
          // buffer that stores an item in network byte order
          S _s;
          // write every item in network byte order into destination stream
          // (fast)
          for( size_t i=0; i<unNumItems; i++ )
          {
            // convert current item into network byte order
            _s = tbd::host2net(psItems[i]);
            // write it into the destination stream
            m_os.write((const char*)&_s,sizeof(S));
          }
        }
      }
      else
      {
        // write every item through the bit cache into destination stream
        // (slowest)
        for( size_t i=0; i<unNumItems; i++ )
          // put the item into the stream using the bit cache
          put(psItems[i],sizeof(S)*8);
      }
    }
    template<class S, class NR>
    __BITSTEAM_INLINE void putr( const S sItem, NR unNumRepeat )
    {
      // convert current item into network byte order
      S _s = sItem;
      // if S is needs byte order conversion
      if( sizeof(S) > 1 )
        _s = tbd::host2net(_s);

      // check if this stream is currently byte aligned
      if( aligned() )
      {
        // flush the bit cache
        flush2stream();
        // write item repeatedly into destination stream (fast)
        for( NR i=0; i<unNumRepeat; i++ )
          m_os.write((const char*)&_s,sizeof(S));
      }
      else
      {
        // write every item through the bit cache into destination stream
        // (slowest)
        for( unsigned long long i=0; i<unNumRepeat; i++ )
          // put the item into the stream using the bit cache
          put(_s,sizeof(S)*8);
      }
    }
    /// @brief Puts one item of type S in network byte order into this bit
    /// stream.
    /// @param s Item of type S to write.
    /// This method is faster with bit streams that are currently byte aligned!
    template<class S>
    __BITSTEAM_INLINE void put( const S& s )
    {
      // if this bit stream is currently byte aligned (fast)
      if( aligned() )
      {
        // flush the bit cache
        flush2stream();
        // copy s into network byte order
        S _s = tbd::host2net(s);
        // write the item in correct byte order
        m_os.write((const char*)&_s,sizeof(S));
      }
      else
      {
        // put the item using the bit cache (slow)
        put(s,sizeof(S)*8);
      }
    }
    /// @brief Puts some lower bits of an item of type S in network byte
    /// order into this bit stream.
    /// @param s Item to put.
    /// @param uCount Number of bits to put. Beginning at the lowest bit.
    /// This method is faster with bit streams that are currently byte aligned!
    template<class S>
    __BITSTEAM_INLINE void put( const S& s, const bit::Count& uCount )
    {
      // you have to copy at least one bit!
        BOOST_ASSERT( uCount>0 );
      // you can't copy more bits than are existing in s
      BOOST_ASSERT( uCount<=sizeof(S)*8 );
      // count what we still need to copy
      bit::Count uCopy=uCount;
      // copy all bits until uCopy is 0
      do
      {
        // if all bits can be written into current bit cache
        if( uCopy < left() )
        {
          // copy the remaining bits into the bit cache
          bit::copyBits(current(),s,uCopy);
          // adjust bit cache size
          addCount(uCopy);
          // ready
          break;
        }
        else
        {
          // remind the currently left bits in the bit cache
          const bit::Count _left=left();
          // adjust uCopy now for using it as starting bit position
          uCopy -= _left;
          // copy the bits we can get from bit cache into s
          bit::copyBits(current(),s,_left,uCopy);
          // convert the bit cache into network byte order
          host2net();
          // write bit cache into destination stream
          m_os.write((const char*)&current(),sizeof(T));
          // reset bit cache
          set(0,0);
        }
      }
      while(uCopy > 0);
    }
    __BITSTEAM_INLINE BitOStream& operator<<( bool b )                  { put(b?1:0,1); return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( char ch )                 { put(ch);      return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( unsigned char uch )       { put(uch);     return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( short s )                 { put(s);       return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( unsigned short us )       { put(us);      return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( long l )                  { put(l);       return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( unsigned long ul )        { put(ul);      return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( long long ll )            { put(ll);      return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( unsigned long long ull )  { put(ull);     return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( int n )                   { put(n);       return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( unsigned int un )         { put(un);      return *this; }
    __BITSTEAM_INLINE BitOStream& operator<<( const char* pch )
    {
      putstr(pch,strlen(pch),bit::bitmask<size_t>());
      return *this;
    }
    template<class S>
    __BITSTEAM_INLINE BitOStream& operator<<( std::pair<const char*,S> pairStr )
    {
      putstr(pairStr.first,pairStr.second,(S)strlen(pairStr.first));
      return *this;
    }
    template<class S>
    __BITSTEAM_INLINE BitOStream& operator<<( std::pair<const std::string*,S> pairStr )
    {
      putstr(pairStr.first->c_str(),pairStr.second,(S)pairStr.first->size());
      return *this;
    }
    template<class S>
    __BITSTEAM_INLINE BitOStream& operator<<( std::pair<std::string*,S> pairStr )
    { return operator<<(std::pair<const std::string*,S>(pairStr.first,pairStr.second)); }
    __BITSTEAM_INLINE BitOStream& operator<<( const std::string& str )
    {
      // write string content including zero termination
      putn(str.c_str(),(unsigned int)(str.size()+1));
      return *this;
    }
    template<class S>
    __BITSTEAM_INLINE BitOStream& operator<<( const std::pair<S*,bit::Count>& rbits )
    { put(*rbits.first,rbits.second);  return *this; }
    /// @brief Return the current write bit position of this stream.
    streampos tellp() const
    {
        // convert the current put position of the destination stream into bits
        // and add what's in the bit cache and return the result
        return (streampos)m_os.tellp()*8+count();
    }
    /// @brief Set the current write bit position of this stream.
    /// @todo cannot set position bitwise
    void seekp( streampos pos ) { flush2stream(); return m_os.seekp(bit::tobyte(pos)); }
    void seekp2end() { flush2stream(); m_os.seekp2end(); }
    void align( const bit::Count& uCount=8, bool bFillBit=0 )
    {
      bit::Count disalignment=tellp() % uCount;
      while( disalignment > 0 )
      {
        int n=(int)(32<(unsigned int)disalignment)?32:(unsigned int)disalignment;
        put(bFillBit?bit::bitmask<unsigned int>():0,n);
        disalignment -= n;
      }
    }
  protected:
    /// @brief Flush the bit cache into the destination stream
    void flush2stream()
    {
      // check alignment
      BOOST_ASSERT(aligned());
      // if bit cache is not empty
      if( !empty() )
      {
        // convert the bit cache into network byte order
        host2net();
        // write the uses bytes of the bit cache into the destination stream
        m_os.write((const char*)&current()+left()/8,count()/8);
        // reset the bit cache
        set(0,0);
      }
    }
    template<class S>
    void putstr( const char* psz, S size, S len )
    {
      // if size parameter equals bitmask<S> (special value)
      if( size == bit::bitmask<S>() )
      {
        // set size parameter to real size of the string
        size = len;
        // put the size into the stream
        put(size);
      }
      if( size > 0 )
      {
        // check if the real string size matches the size in parameter
        if( len < size )
        {
          // write string content if any
          if( len > 0 )
            putn(psz,(unsigned int)len);
          // fill up with zeros
          putr('\0',(unsigned int)(size-len));
        }
        else
          // write string content
          putn(psz,(unsigned int)size);
      }
    }
  private:
    /// @brief Reference to the destination stream.
    O&            m_os;
  };

  /// @brief Generates a pair of a const reference to an item of type T and a bit count.
  /// @details
  /// The resulting pair can be used with BitOStream::operator<<() to put some
  /// bits of the item into the stream.\n The following code adds 4 set bits to
  /// stdout. @code
  ///   BitOStream<std::cout> bos; bos << bits(0xf,4); bos,flush();
  /// @endcode
  /// @param t Item of type T.
  /// @param count Number of bits to put.
  /// @ingroup BitStreams
  template<class T> __BITSTEAM_INLINE
  std::pair<const T*,bit::Count> bits( const T& t, const bit::Count& count )
  { return std::pair<const T*,bit::Count>(&t,count); }
  /// @brief Exception thrown by the BitIStream class
  /// @ingroup BitStreams
  class BitParseException
    : public Exception
  {
  public:
    enum ErrCode { Ok, UnexpectedEndOfFile };
    /// @brief Constructor
    /// @param eErrCode Identifier for what has happened exception
    explicit BitParseException(ErrCode eErrCode) : m_eErrCode(eErrCode) {}
    ErrCode getErrCode() { return m_eErrCode; }
  private:
    ErrCode   m_eErrCode;
  };
  /// @brief Bit input stream
  /// @ingroup BitStreams
  template<class I, class C=NulConfig>
  class BitIStream
    : public BitStream
    , public C
  {
  public:
    /// @brief Initializes this bit stream with an istream to read from.
    /// @param is Input stream to read all content from.
    BitIStream( I& is ) : m_is(is) {}
    ~BitIStream()
    {
      BOOST_ASSERT(aligned());
      m_is.seekg(m_is.tellg()-(count()/8));
    }
    I& istream() { seekg(tellg()); return m_is; }
    /// @brief Get some items of type S from this bit stream and convert
    /// them into host byte order if necessary.
    /// @param psItems Pointer to the first of unNumItems items to be filled
    ///        with the results.
    /// @param unNumItems Number of items to get.
    template<class S> __BITSTEAM_INLINE void getn( S* psItems, size_t unNumItems )
    {
      // if this bit stream is currently byte aligned
      if( aligned() )
      {
        // read all the requested bytes from this bit stream (faster)
        read(((char*)psItems),unNumItems*sizeof(S));
        // byte order conversion needed?
        if( sizeof(S) > 1 )
        {
          // convert all items into host byte order
          for( size_t i=0; i<unNumItems; i++ )
            // convert the current item into host byte order
            psItems[i] = tbd::net2host(psItems[i]);
        }
      }
      else
      {
        // get all items using the bit cache
        for( size_t i=0; i<unNumItems; i++ )
          // get one item in host byte order using the bit cache
          get(psItems[i],sizeof(S)*8);
      }
    }
    __BITSTEAM_INLINE void get( bool& s )
    {
      int n=0;
      get(n,1);
      s = (n != 0);
    }
    /// @brief Get one item of type S from this bit stream and convert it
    /// into host byte order
    /// @param s Reference to an item that gets the result.
    template<class S> __BITSTEAM_INLINE void get( S& s )
    {
      // if bit stream is currently byte aligned
      if( aligned() )
      {
        // read the bytes for this item from the bit cache and the source stream
        // in network byte order into s
        read((char*)&s,sizeof(S));
        // convert s into host byte order
        s = tbd::net2host(s);
      }
      else
        // get the item using the bit cache
        get(s,sizeof(S)*8);
    }
    /// @brief Get some bits into the lower bits of an item of type S and
    /// convert into host byte order.
    /// @param s Reference to the item that gets the result.
    /// @param uCount Number of bits to read.
    template<class S> void get( S& s, const bit::Count& uCount )
    {
      // you have to copy at least one bit
      BOOST_ASSERT( uCount>0 );
      // you can't copy more bits than fit into type S
      BOOST_ASSERT( uCount<=sizeof(S)*8 );
      // if bit cache is currently empty
      if( empty() )
      {
        // read a portion from the source stream
        m_is.read((char*)&current(),sizeof(T));
        // adjust bit cache size
        setCount(8*m_is.gcount());
        current() <<= sizeof(T)*8-count();
        if( count()<uCount && count()!=sizeof(T)*8 )
          TBD_THROW(BitParseException(BitParseException::UnexpectedEndOfFile));
        // convert bit cache into host byte order
        net2host();
      }
      // count the copied bits
      bit::Count uCopied=0;
      // copy all bits until uCopied reaches uCount
      do
      {
        // if the remaining bits can be get completely from bit cache
        if( uCount-uCopied <= count() )
        {
          // copy the remaining bits from bit cache into s
          bit::copyBits(s,current(),uCount-uCopied,count()-(uCount-uCopied));
          // adjust the bit cache size
          subCount(uCount-uCopied);
          // ready.
          break;
        }
        else
        {
          // copy what we can get from bit cache into s
          bit::copyBits(s,current(),count());
          // adjust copied bits counter
          uCopied += count();
          // read a portion from the source stream
          m_is.read((char*)&current(),sizeof(T));
          // adjust bit cache size
          setCount(8*m_is.gcount());
          // check if we need something but got nothing
          if( !(uCopied == uCount || count() > 0) )
            TBD_THROW(BitParseException(BitParseException::UnexpectedEndOfFile));
          // convert bit cache into host byte order
          net2host();
          // if bit cache wasn't filled completely
          if( count() < sizeof(T)*8 )
            // move the bits in the cache to the correct position
            current() >>= (sizeof(T)*8 - count());
        }
      }
      while( uCopied < uCount );

      // if uCount is lower than the bit size of type S
      if( uCount < sizeof(S)*8 )
      {
        // if type S is signed and s is negative
        if( bit::is_type_signed< S >() && 0 != (s & (((S)1) << (uCount-1))) )
          // set all bits above the read ones
          s |= (~S(0)) << uCount;
        else
          // clear all bits above the read ones
          s &= ~((~S(0)) << uCount);
      }
    }
    __BITSTEAM_INLINE BitIStream& operator>>( bool& rb )                  { int n=0; get(n,1); rb=(n!=0); return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( char& rch )                 { get(rch);   return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( unsigned char& ruch )       { get(ruch);  return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( short& rs )                 { get(rs);    return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( unsigned short& rus )       { get(rus);   return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( long& rl )                  { get(rl);    return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( unsigned long& rul )        { get(rul);   return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( long long& rll )            { get(rll);   return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( unsigned long long& rull )  { get(rull);  return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( int& rn )                   { get(rn);    return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( unsigned int& run )         { get(run);   return *this; }
    __BITSTEAM_INLINE BitIStream& operator>>( std::string& str )
    {
      // clear string
      str.clear();
      // read a character
      char ch;
      get(ch);
      // while the character isn't zero termination
      while(ch!='\0')
      {
        // add character to the string
        str += ch;
        // read next character
        get(ch);
      }
      return *this;
    }
    template<class S> __BITSTEAM_INLINE BitIStream& operator>>( std::pair<std::string*,S> pairStr )
    {
      // if size parameter equals max(S)
      if( pairStr.second == bit::bitmask<S>() )
        // get size from stream
        get(pairStr.second);
      // resize string to parameter size
      pairStr.first->resize(pairStr.second);
      // get string content from stream
      for( S i=0; i<pairStr.second; i++ )
        get((*pairStr.first)[i]);
      return *this;
    }
    template<class S> __BITSTEAM_INLINE
      BitIStream& operator>>( std::pair<S*,bit::Count> rbits )
    { get(*rbits.first,rbits.second); return *this; }
    __BITSTEAM_INLINE
      BitIStream& operator>>( std::pair<void*,bit::Count> rbits )
    { skip(rbits.second); return *this; }
    /// streaming operator for boolean reference type in std::vector<bool> specialization
    __BITSTEAM_INLINE BitIStream& operator>>(std::vector<bool>::reference r)
    {
      bool b;
      operator>>(b);
      r = b;
      return *this;
    }
    __BITSTEAM_INLINE void peek( bool& b )
    {
      char ch=0;
      peek<char>(ch,1);
      b=(ch!=0);
    }
    template<class S> __BITSTEAM_INLINE void peek( S& s )
    {
      peek(s,sizeof(S)*8);
    }
    template<class S> __BITSTEAM_INLINE void peek( S& s, const bit::Count& uCount )
    {
      streampos pos=tellg();
      get<S>(s,uCount);
      seekg(pos);
    }
    /// @brief Return the current read bit position of this stream.
    streampos tellg() const { return (streampos)m_is.tellg()*(streampos)8-count(); }
    /// @brief Set the current read bit position of this stream.
    /// @attention This method crashes with assertion if the position given
    /// by parameter is not byte aligned!
    void seekg( streampos ullPos )
    {
      int bits = (int)(ullPos%(streampos)8);
      set(0,0);
      ullPos /= (streampos)8;
      m_is.seekg((typename I::streampos)ullPos);
      if( m_is.fail() )
        TBD_THROW(BitParseException(BitParseException::UnexpectedEndOfFile));
      else if( bits > 0 )
        skip(bits);
    }
    void seekg2end()
    {
      set(0,0);
      m_is.seekg2end();
    }
    /// @brief Skip some bits.
    /// @param ullCount Number of bits to skip.
    void skip( streampos ullCount )
    {
      BOOST_ASSERT(ullCount>0);
      T n;
      // if skip bits beyond the bit cache
      if( ullCount > (streampos)count() )
      {
        m_is.seekg(m_is.tellg()+(ullCount-count())/(streampos)8);
        ullCount = (ullCount-count()) % (streampos)8;
        set(0,0);
      }
      if( ullCount > 0 )
        get(n,(unsigned int)ullCount);
    }
    virtual bool fail() const { return m_is.fail(); }
    void align( const bit::Count& uCount=8 )
    {
      streampos disalignment=tellg() % uCount;
      if( disalignment > 0 )
        skip(uCount-disalignment);
    }
  protected:
    /// @brief Read some bytes from this bit stream in network byte order.
    /// @param psItems Pointer to the first byte to fill with the result.
    /// @param unNumBytes Number of bytes to read.
    /// @attention This method crashes with assertion if this bit stream
    /// isn't byte aligned!
    __BITSTEAM_INLINE void read(char* psItems, size_t unNumBytes)
    {
      // this bit stream must be byte aligned for this method to work
      BOOST_ASSERT( aligned() );
      // if bit cache isn't empty
      if( !empty() )
      {
        // count the bytes we read from bit cache
        size_t unRead=0;
        // read the first requested bytes from bit cache
        for( ; unRead < unNumBytes && unRead < (size_t)count()/8; unRead++ )
        {
          // read one byte from the bit cache
          psItems[unRead] = ((char*)&current())[count()/8-unRead-1];
        }
        // reset bit cache size
        subCount(unRead*8);

        // if we need more bytes
        if( unNumBytes > unRead )
        {
          // read the remaining bytes directly from source stream
          m_is.read(((char*)psItems)+unRead,unNumBytes-unRead);
          if( unNumBytes-unRead != (size_t)m_is.gcount() )
          {
            set(0,0);
            TBD_THROW(BitParseException(BitParseException::UnexpectedEndOfFile));
          }
        }
      }
      else
      {
        // read the bytes directly from source stream
        m_is.read(((char*)psItems),unNumBytes);
        if( unNumBytes != (size_t)m_is.gcount() )
        {
          set(0,0);
          TBD_THROW(BitParseException(BitParseException::UnexpectedEndOfFile));
        }
      }
    }
  private:
    /// @brief Source stream to read all content from.
    I&            m_is;
  };
  /// @brief Generates a pair of a NULL pointer and a bit count.
  /// @details
  /// This pair can be used with BitIStream::operator<<() to skip some bits from
  /// the stream.\n The following code skips 4 bits from stdin. @code
  ///   BitIStream<std::cin> bis; char ch; bis >> bits(4);
  /// @endcode
  /// @param count Number of bits to put.
  /// @ingroup BitStreams
  /// @brief Generates a pair of a reference to an item of type T and a bit
  /// count.
  /// This pair can be used with BitIStream::operator>>() to put some bits of
  /// the item into the stream.\n The following code gets 4 bits from stdin into
  /// ch. @code
  ///   BitIStream<std::cin> bis; char ch; bis >> bits(ch,4);
  /// @endcode
  /// @param t Item of type T.
  /// @param count Number of bits to put.
  /// @ingroup BitStreams
  template<class T> __BITSTEAM_INLINE
    std::pair<T*,bit::Count> bits( T& t, const bit::Count& count )
  { return std::pair<T*,bit::Count>(&t,count); }
  __BITSTEAM_INLINE std::pair<void*,bit::Count> bits( const bit::Count& count )
  { return std::pair<void*,bit::Count>((void*)NULL,count); }
  /// @brief Generates a string of a given size.
  /// @details
  /// The resulting string can be used with BitIStream::operator<<() to read a
  /// string of specific size or with zero termination. The following code reads
  /// a string of 4 bytes size from stdin. @code
  ///   BitIStream<std::cin> bis; std::string str; bis >> bitstr(str,4);
  /// @endcode The following code reads a zero terminated string from stdin.
  /// @code
  ///   BitIStream<std::cin> bis; std::string str; bis >> bitstr(str);
  /// @endcode
  /// @param str String to fill with result.
  /// @param unLength Length of the expected string. If unLength is zero this
  ///        signals BitIStream to read a zero terminated string.
  /// @ingroup BitStreams
  __BITSTEAM_INLINE std::pair<std::string*,std::string::size_type> bitstr( std::string& str, std::string::size_type unLength )
  { return std::pair<std::string*,std::string::size_type>(&str,unLength); }
  __BITSTEAM_INLINE std::pair<const std::string*,std::string::size_type> bitstr( const std::string& str, std::string::size_type unLength )
  { return std::pair<const std::string*,std::string::size_type>(&str,unLength); }
  __BITSTEAM_INLINE std::pair<const char*,std::string::size_type> bitstr( const char* psz, std::string::size_type unLength )
  { return std::pair<const char*,std::string::size_type>(psz,unLength); }

  template<class S> __BITSTEAM_INLINE std::pair<std::string*,S> bitstr( std::string& str )
  { return std::pair<std::string*,S>(&str,bit::bitmask<S>()); }
  template<class S> __BITSTEAM_INLINE std::pair<const std::string*,S> bitstr( const std::string& str )
  { return std::pair<const std::string*,S>(&str,bit::bitmask<S>()); }

  template<class T> T& skip() { static T t; return t; }
}

#ifdef _MSC_VER
# pragma warning(default:4512 4127)
#endif

#endif
