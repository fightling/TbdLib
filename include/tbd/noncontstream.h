///////////////////////////////////////////////////////////////////////////////
/// @file noncontstream.h
/// @brief noncontinuous stream proxy
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__NONCONTSTREAM_H
#define __TBD__NONCONTSTREAM_H

#include "stream.h"
#include "exception.h"
#include "memstream.h"
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <limits>
#include <algorithm>
#include <list>
#include <map>
#include <vector>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <share.h>
#else
# include <unistd.h>
#endif

namespace tbd
{
  class NonContStreamException :
    public Exception
  {
  public:
    enum ErrCode { Ok, ReadingUnknown, BlockNotFound };
    /// @brief Constructor
    /// @param eErrCode Identifier for what has happened exception
    explicit NonContStreamException(ErrCode eErrCode) : m_eErrCode(eErrCode) {}
    ErrCode getErrCode() { return m_eErrCode; }
  private:
    ErrCode   m_eErrCode;
  };
  /** @brief base class of non continuous streams
   *  @ingroup streams
   *  @tparam STREAM Type of stream object that represents a partial block of content
   *  @tparam META Type of user specific meta data which can be attached to each block
   */
  template<class STREAM, class META=int>
  class NonContStreamBase :
    public Stream<typename STREAM::streampos>
  {
  public:
    typedef STREAM stream_t;
    typedef META meta_t;
    typedef typename STREAM::streampos streampos;
  protected:
    struct Block
    {
      Block(stream_t* stream, streampos start, streampos end, const META& meta, bool _delete) :
        m_stream(stream), m_start(start), m_end(end), m_meta(meta), m_delete(_delete)
      {
        // never trust a stranger
        BOOST_ASSERT(stream);
        BOOST_ASSERT(start < end);
      }
      ~Block() { if (m_delete && m_stream) delete m_stream; }
      stream_t* stream() { return m_stream; }
      const stream_t* stream() const { return m_stream; }
      stream_t* take()
      {
        stream_t* stream = m_stream;
        m_stream = NULL;
        return stream;
      }
      streampos leftover(streampos size) const { return std::max(0, tellg() + size - m_end); }
      void seekg(streampos pos) const { m_stream->seekg(pos); }
      void seekg2end() const { m_stream->seekg(m_end); }
      streampos tellg() const { return m_stream->tellg(); }
      bool overlaps(streampos start, streampos end) const { return end > m_start && start < m_end; }
      bool has(streampos pos) const { return pos >= m_start && pos < m_end; }
      streampos size() const { return m_end-m_start; }
      stream_t* m_stream;
      streampos m_start;
      streampos m_end;
      META      m_meta;
      bool      m_delete;
    };
    typedef boost::ptr_list<Block> BlockList;
  public:
    /** @brief create a non-continuous stream
     *  @param max maximum count of blocks before oldest blocks are deleted
     */
    NonContStreamBase(std::size_t max) : m_max(max), m_current(0) { }
    /** @brief insert new block
     *  @param stream pointer to stream instance which represents the block
     *  @param start start position the block is addressed to
     *  @param end end position of the block
     *  @param meta user specified meta information that will be attached to the block
     *  @param _delete take care of stream destruction
     */
    void insert(stream_t* stream, streampos start, streampos end, const meta_t& meta, bool _delete)
    {
      // check if new block would overlap existing ones
      BOOST_FOREACH( Block& block, m_blocks )
      {
        BOOST_ASSERT( !block.overlaps(start,end) );
      }
      // add new block
      m_blocks.push_back(new Block(stream, start, end, meta, _delete));
      // block count is exceeded maximum?
      if (m_blocks.size() > m_max)
        // drop block
        m_blocks.pop_front();
    }
    void insert(stream_t* stream, streampos start, streampos end, bool _delete) { insert( stream, start, end, meta_t(), _delete); }
    /** @brief Take ownership of the stream object of a block stored in the
     *  non-continuous stream and remove this block.
     *  @param start Start position of the block like given on insert
     *  @return the taken stream
     */
    stream_t* release(streampos start)
    {
      // through all blocks
      for (typename BlockList::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it)
      {
        // check if start an end position do match
        if (it->m_start == start)
        {
          // take the stream
          stream_t* stream = it->take();
          // erase block
          m_blocks.erase(it);
          // return stream
          return stream;
        }
      }
      // error
      TBD_THROW(NonContStreamException(NonContStreamException::BlockNotFound));
    }
    template<class T> Block* find(const T& param, bool bThrowException=true)
    {
      // search for a block including the given position (using reverse search might optimize access)
      BOOST_REVERSE_FOREACH( Block& block, m_blocks )
      {
        if (block.m_meta == param)
          return &block;
      }
      if( bThrowException )
        TBD_THROW(NonContStreamException(NonContStreamException::BlockNotFound));
      return 0;
    }
    void order(std::vector<const Block*>& result) const
    {
      BOOST_FOREACH( const Block& block, m_blocks )
        result.push_back(&block);
      std::sort(result.begin(),result.end(),compareBlocks);
    }
    Block* findvalid(streampos posFrom )
    {
      // find the block which is first after the given position
      streampos posMin = std::numeric_limits<streampos>::max();
      Block* result = 0;
      BOOST_FOREACH( Block& block, m_blocks )
      {
        if( block.m_start >= posFrom && block.m_start < posMin )
        {
          posMin = block.m_start;
          result = &block;
        }
      }
      return result;
    }
    template<class T> void discard(const T& param)
    {
      for( typename BlockList::iterator it=m_blocks.begin(); it!=m_blocks.end(); it++ )
      {
        if (it->m_meta == param)
          m_blocks.erase(it);
      }
    }
    void clear() { m_blocks.clear(); m_current = 0; }
  protected:
    static bool compareBlocks( const Block * l, const Block * r ) { return l->m_start < r->m_start; }
    const Block* current() const { return m_current; }
    Block* current() { return m_current; }
    Block* current(Block* f) { return m_current = f; }
    Block* find(streampos pos)
    {
      // search for a block including the given position (using reverse search might optimize access)
      BOOST_REVERSE_FOREACH( Block& block, m_blocks )
      {
        if (block.has(pos))
          return &block;
      }
      return 0;
    }
    Block* last()
    {
      Block* result=0;
      // search for a block including the given position (using reverse search might optimize access)
      BOOST_FOREACH( Block& block, m_blocks )
      {
        if (!result || block.m_end > result->m_end)
          result = &block;
      }
      return result;
    }
  private:
    /// unsorted fragments
    BlockList m_blocks;
    /// maximum number of fragments (if exceeded oldest fragments will be dropped
    size_t m_max;
    /// current block
    Block* m_current;
  };
  /// @ingroup Streams
  template<class ISTREAM, class META=int, size_t WRITE_SIZE=4096> class NonContIStream :
    protected NonContStreamBase<ISTREAM,META>,
    public IStream<typename ISTREAM::streampos>
  {
  public:
    typedef NonContStreamBase<ISTREAM,META> base;
    typedef ISTREAM stream_t;
    typedef META meta_t;
    typedef typename ISTREAM::streampos streampos;
    typedef typename NonContStreamBase<stream_t,meta_t>::Block Block;
    NonContIStream(std::size_t max = std::numeric_limits<std::size_t>::max()) :
      base (max), m_g(0), m_gcount(0) { }
    virtual ~NonContIStream() { }
    void insert(const char* buffer, streampos start, streampos end, const meta_t& meta=meta_t())
    { base::insert( new stream_t(buffer,end-start,start), start, end, meta, true); }
    void insert(stream_t* pis, streampos start, streampos end, const meta_t& meta=meta_t(), bool _delete=false)
    { base::insert( pis, start, end, meta, _delete); }
    /// @brief Read exactly size from this stream to pch.
    /// @param pch Pointer to a piece of memory of at least size bytes
    /// length.
    /// @param size Number of bytes to read.
    virtual void read(char* pch, std::size_t size)
    {
      // try to load relevant block if we're not consistent
      if (fail())
        seekg(m_g);
      m_gcount = 0;
      // while we are consistent and there is still more to read
      while (!fail() && size > 0)
      {
        stream_t* is=base::current()->stream();
        // read what we can get from the current block
        is->read(pch, size);
        size -= is->gcount();
        m_g += is->gcount();
        m_gcount += is->gcount();
        // if we need to read more
        if (size > 0)
          // seeking to current position may switch us to the next block
          seekg(m_g);
      }
    }
    /// @brief Returns the current read position.
    virtual streampos tellg() const { return m_g; }
    /// @brief Sets the current read position.
    virtual void seekg(const streampos& g)
    {
      if (base::current(base::find(m_g = g)))
        base::current()->seekg(g);
    }
    virtual void seekg2end()
    {
      if( base::current() )
        base::current()->seekg2end();
    }
    /// @brief Flushes the stream to memory.
    virtual streampos gcount() const { return m_gcount; }
    virtual bool fail() const { return base::current() ? !base::current()->has(m_g) : true; }
    /// @todo may be we should get STREAM::isTemporary() but the method is not static :(
    virtual bool isTemporary() const { return base::current() ? base::current()->stream()->isTemporary() : true; }
    virtual bool is_open() const { return base::current() ? base::current()->stream()->is_open() : false; }
    typedef std::map<streampos, streampos> Gaps;
    void gaps( Gaps& _gaps ) const
    {
      std::vector<const Block*> ordered;
      order(ordered);
      streampos pos=0;
      BOOST_FOREACH( const Block* block, ordered )
      {
        if( block->m_start > pos )
          _gaps[pos] = block->m_start;
        pos = block->m_end;
      }
    }
    template<class OSTREAM> void save( OSTREAM& os, Gaps& _gaps ) const
    {
      std::vector<Block const*> ordered;
      order(ordered);
      streampos pos=0;
      std::pair<char*, streampos> buffer = std::get_temporary_buffer<char>(WRITE_SIZE);
      BOOST_FOREACH( const Block* block, ordered )
      {
        if( block->m_start > pos )
          _gaps[pos] = block->m_start;
        os.seekp(block->m_start);
        block->m_stream->seekg(block->m_start);
        for( streampos cp=block->size(); cp > 0; cp -= std::min(cp,buffer.second) )
        {
          block->m_stream->read(buffer.first, std::min(cp, buffer.second));
          os.write(buffer.first, std::min(cp, buffer.second));
        }
        pos = block->m_end;
      }
      std::return_temporary_buffer(buffer.first);
    }
    void clear() { base::clear(); }
    template<class T> const meta_t& jumpg(const T& param)
    {
      static const meta_t nil=meta_t();
      typename base::Block* f=find(param);
      if( f )
      {
        current(f);
        m_g = f->m_start;
        return f->m_meta;
      }
      return nil;
    }
    void jumpg()
    {
      typename base::Block* f=findvalid(tellg());
      if( f )
        m_g = f->m_start;
      base::current(f);
    }
  private:
    /// current position
    streampos m_g;
    size_t m_gcount;
  };
  template<class ISTREAM, class META=int, class TEMPSTREAM=AutoMemIStream<typename ISTREAM::streampos> > class AutoNonContIStream :
    public NonContIStream<ISTREAM,META>
  {
    typedef NonContIStream<ISTREAM,META> base;
    typedef typename ISTREAM::streampos streampos;
    typedef ISTREAM stream_t;
    typedef META meta_t;
    typedef TEMPSTREAM tempstream_t;
  public:
    AutoNonContIStream(std::size_t max = std::numeric_limits<std::size_t>::max()) :
      base (max) { }
    void insert(char* buffer, streampos start, streampos end, const meta_t& meta=meta_t())
    { base::insert( new tempstream_t(buffer,end-start,start), start, end, meta, true); }
    void insert(stream_t* pis, streampos start, streampos end, const meta_t& meta=meta_t(), bool _delete=false)
    { base::insert( pis, start, end, meta, _delete); }
    char* release( streampos start )
    {
      stream_t* pis=base::release(start);
      if( dynamic_cast<tempstream_t*>(pis) )
        return dynamic_cast<tempstream_t*>(pis)->release();
      else
        return 0;
    }
  };
}

#endif
