///////////////////////////////////////////////////////////////////////////////
/// @file stream.h
/// @brief Basic stream interfaces
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__STREAM_H
#define __TBD__STREAM_H

#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <share.h>
#else
# include <unistd.h>
#endif

#include <string>

/** @defgroup streams Streams
 *  @brief Several streaming classes
 *  @ingroup Streaming
 */
namespace tbd
{
  /// @ingroup Streams
  template<class SP = size_t> class Stream
  {
  public:
    virtual ~Stream() {}
    typedef SP streampos;
  };
  /// @ingroup Streams
  template<class SP = size_t> class IStream: virtual public Stream<SP>
  {
  public:
    typedef SP streampos;
    virtual ~IStream()
    {
    }
    virtual bool is_open() const = 0;
    /** @brief Read exactly size from this stream to pch.
     *  @param pch Pointer to a piece of memory of at least size bytes
     *  length.
     *  @param size Number of bytes to read.
     */
    virtual void read(char* pch, size_t size) = 0;
    /// @brief Returns the current read position.
    virtual streampos tellg() const = 0;
    /// @brief Sets the current read position.
    virtual void seekg(const streampos& g) = 0;
    virtual void seekg2end() = 0;
    /// @brief Flushes the stream to memory.
    virtual streampos gcount() const = 0;
    virtual bool fail() const = 0;
    virtual bool isTemporary() const { return false; }
    int get()
    {
      char ch;
      read(&ch, 1);
      if( fail() )
        return -1;
      else
        return (unsigned char)ch;
    }
    template<class T> void get(T& t)
    {
      read((char*) &t, sizeof(T));
    }
    template<class T> void getn(T* t, size_t n)
    {
      read((char*) t, n * sizeof(T));
    }
    virtual void unget() { seekg(tellg()-1); }
    virtual int peek() {
      streampos g=tellg();
      // paranoia checks
      if( !is_open() )
        return -1;
      int nRet = get();
      // return if error occurred
      if( 0 <= nRet )
        seekg(g);
      return nRet;
    }
  };
  /// @ingroup Streams
  template<class SP = size_t> class OStream: virtual public Stream<SP>
  {
  public:
    typedef SP streampos;
    virtual ~OStream()
    {
    }
    /** @brief Write size bytes from pch into this stream.
     *  @param pch Pointer to the content to write.
     *  @param size Size of the content.
     */
    virtual void write(const char* pch, size_t size) = 0;
    virtual void flush() = 0;
    /// @brief Returns the current read position.
    virtual streampos tellp() const = 0;
    /// @brief Sets the current read position.
    virtual void seekp(const streampos& p) = 0;
    virtual void seekp2end() = 0;
    virtual bool isTemporary() const { return false; }
    template<class T> void put(const T& t)
    {
      write((const char*) &t, sizeof(T));
    }
    template<class T> void putn(const T* t, size_t n)
    {
      write((const char*) t, n * sizeof(T));
    }
  };
}

#define IMPLEMENT_OSTREAM_OPERATORS(class_name,streampos) \
public: \
  class_name& operator<<( char ch )                       { ::tbd::OStream<streampos>::put(ch);  return *this; } \
  class_name& operator<<( unsigned char uch )             { ::tbd::OStream<streampos>::put(uch); return *this; } \
  class_name& operator<<( short s )                       { ::tbd::OStream<streampos>::put(s);   return *this; } \
  class_name& operator<<( unsigned short us )             { ::tbd::OStream<streampos>::put(us);  return *this; } \
  class_name& operator<<( long l )                        { ::tbd::OStream<streampos>::put(l);   return *this; } \
  class_name& operator<<( unsigned long ul )              { ::tbd::OStream<streampos>::put(ul);  return *this; } \
  class_name& operator<<( const long long& ll )           { ::tbd::OStream<streampos>::put(ll);  return *this; } \
  class_name& operator<<( const unsigned long long& ull ) { ::tbd::OStream<streampos>::put(ull); return *this; } \
  class_name& operator<<( int n )                         { ::tbd::OStream<streampos>::put(n);   return *this; } \
  class_name& operator<<( unsigned int un )               { ::tbd::OStream<streampos>::put(un);  return *this; } \
  class_name& operator<<( const ::std::string& str )      { ::tbd::OStream<streampos>::putn(str.c_str(),str.size());  return *this; } \
private:
#define IMPLEMENT_ISTREAM_OPERATORS(class_name,streampos) \
public: \
  class_name& operator>>( char& rch )                 { ::tbd::IStream<streampos>::get(rch);   return *this; } \
  class_name& operator>>( unsigned char& ruch )       { ::tbd::IStream<streampos>::get(ruch);  return *this; } \
  class_name& operator>>( short& rs )                 { ::tbd::IStream<streampos>::get(rs);    return *this; } \
  class_name& operator>>( unsigned short& rus )       { ::tbd::IStream<streampos>::get(rus);   return *this; } \
  class_name& operator>>( long& rl )                  { ::tbd::IStream<streampos>::get(rl);    return *this; } \
  class_name& operator>>( unsigned long& rul )        { ::tbd::IStream<streampos>::get(rul);   return *this; } \
  class_name& operator>>( long long& rll )            { ::tbd::IStream<streampos>::get(rll);   return *this; } \
  class_name& operator>>( unsigned long long& rull )  { ::tbd::IStream<streampos>::get(rull);  return *this; } \
  class_name& operator>>( int& rn )                   { ::tbd::IStream<streampos>::get(rn);    return *this; } \
  class_name& operator>>( unsigned int& run )         { ::tbd::IStream<streampos>::get(run);   return *this; } \
private:

#endif
