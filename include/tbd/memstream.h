/******************************************************************************
 *  @file memstream.h
 *  @brief Memory I/O streams based on basic stream intefaces
 *  @author Patrick Hoffmann
 *  @date 05.11.2006
 ******************************************************************************/

#ifndef __TBD__MEMSTREAM_H
#define __TBD__MEMSTREAM_H

#include "bit.h"
#include "stream.h"
#include <string>
#include <boost/numeric/conversion/cast.hpp>

/** @defgroup MemStream Memory Streams
 *  @brief Streaming classes that operate in memory
 *  @ingroup Streams
 */
namespace tbd
{
  /** @brief Memory input stream class.
   *  @details
   *  Manages a piece of memory, that can be accessed similar like using
   *  std::istream. The memory piece automatically grows, while writing.
   *  Assertion will be initiated when reading beyond the stored data.
   *  @ingroup MemStream
   */
  template<class SP, class T> class MemStreamBase
  {
  public:
    typedef SP streampos;
    /// @brief Constructor
    MemStreamBase(T* pBuffer=0, size_t size=0, streampos start=0) :
      m_pBuffer(pBuffer), m_size(size), m_start(start)
    {
    }
    /// @brief Destructor
    virtual ~MemStreamBase()
    {
    }
    bool is_open() const
    {
      return m_size > 0;
    }
    T* buffer()
    {
      return m_pBuffer;
    }
    size_t size() const { return m_size; }
    bool empty() const
    {
      return 0 == m_size;
    }
  protected:
    void size( size_t _size ) { m_size = _size; }
    void clear() { m_pBuffer=NULL; m_size=0; }
    void buffer(T* pBuffer )
    {
      m_pBuffer = pBuffer;
    }
    streampos start() const { return m_start; }
  private:
    /// @brief Current piece of memory.
    T*          m_pBuffer;
    /// @brief Current size of written data.
    size_t      m_size;
    /// @brief current read position.
    streampos   m_start;
  };
  template<class SP=size_t,class T=const char> class MemIStream: virtual public MemStreamBase<SP,T>, public IStream<SP>
  {
    IMPLEMENT_ISTREAM_OPERATORS(MemIStream,SP);
  public:
    typedef MemStreamBase<SP,T> base;
    typedef SP                  streampos;
    MemIStream(T* pBuffer, size_t size, streampos start=0) : base(pBuffer, size, start), m_g(0),  m_gcount(0), m_bFail(false) {}
    bool is_open() const { return base::is_open(); }
    /** @brief Read up to size bytes from this stream to pch.
     *  @param pch Pointer to a piece of memory of at least size bytes length.
     *  @param _size Maximum number of bytes to read.
     *  @return Number of bytes that could be read.
     */
    void read(char* pch, size_t _size)
    {
      if (_size > base::size() - m_g)
        _size = base::size() - m_g;
      if( _size > 0 )
      {
        ::memcpy(pch, ((unsigned char*)base::buffer()) + m_g, _size);
        m_g += _size;
        m_gcount = _size;
        ok();
      }
      else
      {
        m_gcount = 0;
        failed();
      }
    }
    int peek()
    {
      if (base::size() - m_g >= 1)
      {
        ok();
        return ((unsigned char*)base::buffer())[m_g];
      }
      else
      {
        failed();
        return -1;
      }
    }
    /// @brief Returns the current read position.
    streampos tellg() const
    {
      return base::start() + boost::numeric_cast<streampos>(m_g);
    }
    /// @brief Sets the current read position.
    void seekg(const streampos& g)
    {
      if (g >= base::start() && g < base::start()+boost::numeric_cast<streampos>(base::size()))
      {
        m_g =boost::numeric_cast<size_t>(g-base::start());
        ok();
      }
      else
        failed();
    }
    void seekg2end()
    {
      m_g = base::size();
    }
    streampos gcount() const
    {
      return boost::numeric_cast<streampos>(m_gcount);
    }
    bool fail() const
    {
      return m_bFail;
    }
    void failed()
    {
      m_bFail = true;
    }
    void ok()
    {
      m_bFail = false;
    }
    bool isTemporary() const { return true; }
  private:
    size_t      m_g;
    size_t      m_gcount;
    bool        m_bFail;
  };
  /** @brief Memory input stream which manages the deletion of the given buffer
   *  @ingroup MemStream
   */
  template<class SP=size_t,class T=char> class AutoMemIStream: public MemIStream<SP,const T>
  {
    IMPLEMENT_ISTREAM_OPERATORS(AutoMemIStream,SP)
    typedef MemIStream<SP,const T> base;
    typedef MemStreamBase<SP,const T> bbase;
  public:
    typedef SP streampos;
    /// @brief Constructor
    AutoMemIStream(T* pBuffer, size_t size, streampos start=0) :
      bbase(pBuffer,size,start), base(pBuffer, size, start), m_pBuffer(pBuffer)
    {
    }
    /// @brief Destructor
    ~AutoMemIStream()
    {
      if( base::buffer() )
        delete base::buffer();
    }
    T* release(size_t* _size=NULL) { if (_size) *_size = base::size(); T* p=m_pBuffer; m_pBuffer = NULL; base::clear(); return p; }
  private:
    T* m_pBuffer;
  };
  /** @brief Memory input/output stream class.
   *  @details
   *  Extends MemOStream about the IStream interface. This class can be used to
   *  write into a memory stream followed by a read of the written content
   *  @ingroup MemStream
   */
  template<class SP=size_t,class T=char> class MemOStream: public virtual MemStreamBase<SP,T>, public OStream<SP>
  {
  IMPLEMENT_OSTREAM_OPERATORS(MemOStream,SP)
  public:
    typedef MemStreamBase<SP,T> base;
    typedef SP streampos;
    /// @brief Constructor
    MemOStream(size_t grow = 1024, streampos start=0) :
      base(NULL,0,start), m_reserved(0), m_grow(grow), m_p(0)
    {
    }
    virtual ~MemOStream()
    {
      if ( base::buffer() )
      {
        delete[] base::buffer();
      }
    }
    bool is_open() const { return base::is_open(); }
    /** @brief Write size bytes from pch into this stream.
     *  @param pch Pointer to the content to write.
     *  @param _size Size of the content.
     */
    void write(const T* pch, size_t _size)
    {
      if (m_reserved - m_p < _size)
      {
        if (_size > m_grow)
          resize(m_p + (_size / m_grow + 1) * m_grow);
        else
          resize(m_p + m_grow);
      }
      // grow within reserved
      base::size(std::max(m_p + _size, base::size()));
      std::copy(pch, pch+_size, (char*)base::buffer() + m_p);
      m_p += _size;
    }
    /// @brief Returns the current write position.
    streampos tellp() const
    {
      return base::start()+boost::numeric_cast<streampos>(m_p);
    }
    /// @brief Sets the current write position.
    void seekp(const streampos& p)
    {
      if (p < base::start())
        BOOST_ASSERT(0);
      m_p = boost::numeric_cast<size_t>(p-base::start());
      if (p > base::start()+boost::numeric_cast<streampos>(base::size()))
        resize(m_p);
    }
    void seekp2end()
    {
      m_p = base::size();
    }
    /// @brief Flushes the stream to memory.
    void flush()
    {
    }
    void reset()
    {
      base::size(0);
      m_p = 0;
    }
    template<class PTR_TYPE> void snap(PTR_TYPE& pBuffer, size_t& _size)
    {
      pBuffer = base::buffer();
      _size = base::size();
    }
    template<class PTR_TYPE> void detach(PTR_TYPE& pBuffer, size_t& _size)
    {
      pBuffer = (PTR_TYPE) base::buffer();
      _size = base::size();
      detach();
    }
    template<class PTR_TYPE> void detach(PTR_TYPE& pBuffer, size_t& _size, size_t& _reserved)
    {
      pBuffer = (PTR_TYPE) base::buffer();
      _size = base::size();
      _reserved = m_reserved;
      detach();
    }
    std::pair<T*,size_t> detach()
    {
      std::pair<T*,size_t> r(base::buffer(),base::size());
      base::buffer(NULL);
      base::size(0);
      return r;
    }
    bool isTemporary() const { return true; }
  protected:
    /** @brief Resize the buffer so that there is enough space to store
     *  _size bytes beginning from the current writing position.
     *  @param _size Size that is needed.
     */
    void resize(size_t _size)
    {
      m_reserved = (_size / m_grow + 1) * m_grow;
      char* pBuf = new char[m_reserved];
      memcpy(pBuf, base::buffer(), base::size());
      delete[] base::buffer();
      base::buffer(pBuf);
    }
  private:
    /// @brief Size of the current memory piece.
    size_t m_reserved;
    /// @brief When memory must grow, it will grow in steps of this size.
    size_t m_grow;
    /// @brief current write position.
    size_t m_p;
  };
  /** @brief Memory output stream class.
   *  @details
   *  Reads a memory block similar like std::ostream.
   *  @ingroup MemStream
   */
  template<class SP=size_t,class T=char> class MemStream: public MemIStream<SP,T>, public MemOStream<SP,T>
  {
    IMPLEMENT_ISTREAM_OPERATORS(MemStream,SP)
    IMPLEMENT_OSTREAM_OPERATORS(MemStream,SP)
  public:
    typedef MemIStream<SP,T>    gbase;
    typedef MemOStream<SP,T>    pbase;
    typedef MemStreamBase<SP,T> base;
    typedef SP                  streampos;
    /// @brief Constructor
    MemStream(size_t grow = 1024, streampos start=0) :
      base(NULL,0,start), gbase(NULL,0,start), pbase(grow,start)
    {
    }
    /// @brief Destructor
    virtual ~MemStream()
    {
      if (pbase::buffer())
        delete[] pbase::buffer();
      base::clear();
    }
    bool is_open() const { return base::is_open(); }
    T* buffer() { return base::buffer(); }
    void reset()
    {
      base::size(0);
      pbase::seekp(base::start());
      gbase::seekg(base::start());
    }
/*    void write(const char* pch, size_t size)  { pbase::write(pch, size); }
    streampos tellp() const                   { return pbase::tellp(); }
    void seekp(const streampos& p)            { pbase::seekp(p); }
    void seekp2end()                          { pbase::seekp2end(); }
    void flush()                              { pbase::flush(); }
    void reset()                              { pbase::reset(); }
    template<class PTR_TYPE> void snap(PTR_TYPE& pBuffer, size_t& _size) { pbase::snap(pBuffer,_size); }
    template<class PTR_TYPE> void detach(PTR_TYPE& pBuffer, size_t& _size) { pbase::detach(pBuffer,_size); }
    template<class PTR_TYPE> void detach(PTR_TYPE& pBuffer, size_t& _size, size_t& _reserved) { pbase::detach(pBuffer,_size,_reserved); }
    std::pair<char*,size_t> detach()          { return pbase::detach(); }
    const char* const buffer() const          { return pbase::buffer(); }
    size_t size()                             { return pbase::size(); }
  protected:
  */
  };
}
#endif
