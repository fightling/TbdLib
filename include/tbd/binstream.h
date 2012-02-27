///////////////////////////////////////////////////////////////////////////////
/// @file binstream.h
/// @brief Binary DOM streams
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#include "domstream.h"
#include <map>
#include <algorithm>
#include "network.h"
#include "dump.h"
#include "memstream.h"
#include <boost/numeric/conversion/converter.hpp>

#ifndef __TBD__BINSTREAM_H
#define __TBD__BINSTREAM_H
#ifdef _MSC_VER
# pragma warning(disable:4290)
#endif

/// @defgroup BinStreams Binary Streams
/// @brief Binary document object model (DOM) streams
/// @ingroup DomStreams
/// @details
/// @par Purpose
/// Here you find a set of classes that can be used to produce and parse binary
/// representations of data.
/// @par Usage Use BinOStream to produce binary DOM structure into std::ostream
///      or BinIStream for reading them from str::istream.@n You can build a DOM
///      in these streams if you use DomCommand to inject navigation
///      information. The string identifiers that are injected must be
///      associated to numeric IDs with BinIndex. The resulting protocol uses
///      these IDs to encode the names.@n@n
/// @code BinIndex bi<>;
/// bi.add(0,"myroot"); bi.add(1,"myitem"); @endcode@n
///      This code produces a parent node "myroot" that contains a child
///      "myitem" with the integer value 1. With the index from above the
///      "myroot" node gets ID=0 and "myitem" gets ID=1.@n@n
/// @code
/// BinOStream<> os(bi); os << domopen("myroot") << domattr("myitem") << 1 <<
/// domclose(); @endcode@n
///      You will get the result if you use:@n@n
/// @code
/// char*           pBuffer; unsigned int    uSize; os.write(pBuffer,uSize);
/// @endcode@n
///      With BinIStream it's nearly the same:@n@n
/// @code
/// BinIStream<> is(bi); is.read(pBuffer,uSize); @endcode@n
///      ...and...@n@n
/// @code
/// int i; is >> domopen("myroot") >> domattr("myitem") >> i >> domclose();
/// @endcode@n
///      And your result is in @c i . And you should never forget:@n@n
/// @code
/// delete[] pBuffer @endcode@n
/// @attention A encoding results are in network byte order on both: little
///            endian and big endian systems

namespace tbd
{
  /// @brief Exception thrown by the BinOStream class
  /// @ingroup BinStreams
  class BinNodeException
    : public Exception
  {
  public:
    enum ErrCode { Ok, ChildrenSizeExceedsSizeType };
    /// @brief Constructor
    /// @param eErrCode Identifier for what has happened exception
    /// @param pNode node
    explicit BinNodeException(ErrCode eErrCode, const DomNode* pNode )
      : m_eErrCode(eErrCode), m_pNode(pNode) {}
    /// Destructor
    const DomNode* getNode() const { return m_pNode; }
    ErrCode getErrCode() const { return m_eErrCode; }
  private:
    ErrCode                 m_eErrCode;
    const DomNode           *m_pNode;
  };


  /// @brief DomNode derivative for a binary DOM format
  /// @details
  /// The BinNode is the node used for binary streaming of DOMs. It transports
  /// data in binary form that can be streamed together with an identifier and a
  /// size information.
  /// @par Template parameters
  /// @li @c I is the type that is used for the index that is used to represent
  ///     the string names in the binary output (see BinIndex).
  /// @li @c S is the size type that is used for the size of the node's pay load
  ///     in the binary output.
  /// @attention The highest bit of the @c I type will be reserved for internal
  ///            use!
  /// @ingroup BinStreams
  template<class I=unsigned long, class S=unsigned long>
  class BinNode
    : public DomNode
  {
  public:
    virtual DomNode* createNode() const { return new BinNode; }
    virtual void set( const bool& b ) { setValue((char)(b?1:0)); }
    virtual void set( const char& ch ) { setValue(ch); }
    virtual void set( const unsigned char& uch ) { setValue(uch); }
    virtual void set( const short& s ) { setValue(s); }
    virtual void set( const unsigned short& us ) { setValue(us); }
    virtual void set( const long& l ) { setValue(l); }
    virtual void set( const unsigned long& ul ) { setValue(ul); }
    virtual void set( const long long& ll ) { setValue(ll); }
    virtual void set( const unsigned long long& ull ) { setValue(ull); }
    virtual void set( const int& n ) { setValue(n); }
    virtual void set( const unsigned int& un ) { setValue(un); }
    virtual void set( const double& d ) { setValue(d); }
    virtual void set( const std::string& str )
    { setBufferSize((S)str.size()); memcpy(m_pchBuffer,str.c_str(),m_unSize); }
    virtual void set( const void* pvBuffer, S unSizeBytes )
    { setBufferSize(unSizeBytes); memcpy(m_pchBuffer,pvBuffer,m_unSize); }
    virtual void get( bool& rb ) { char ch; getValue(ch); rb = ch!=0; }
    virtual void get( char& rch ) const { getValue(rch); }
    virtual void get( unsigned char& ruch ) const { getValue(ruch); }
    virtual void get( short& rs ) const { getValue(rs); }
    virtual void get( unsigned short& rus ) const { getValue(rus); }
    virtual void get( long& rl ) const { getValue(rl); }
    virtual void get( unsigned long& rul ) const { getValue(rul); }
    virtual void get( long long& rll ) const { getValue(rll); }
    virtual void get( unsigned long long& rull ) const { getValue(rull); }
    virtual void get( int& rn ) const { getValue(rn); }
    virtual void get( unsigned int& run ) const { getValue(run); }
    virtual void get( double& rd ) const { getValue(rd); }
    virtual void get( std::string& rstr ) const
    { rstr.assign((const char*)m_pchBuffer,m_unSize); }
    virtual void get( void*& rpvBuffer, S& runSizeBytes ) const
    {
      // take the size
      runSizeBytes = m_unSize;
      // create a new buffer of this size
      rpvBuffer = new char[runSizeBytes];
      // copy the source into the new buffer
      memcpy(rpvBuffer,m_pchBuffer,runSizeBytes);
    }

    /// @brief Standard constructor
    BinNode() : m_unSize(0), m_pchBuffer(NULL) {}
    /// @brief Destructor cleans the buffer if necessary.
    virtual ~BinNode() { if( NULL != m_pchBuffer ) delete[] m_pchBuffer; }
    /// @brief Return the buffer.
    /// @return A pointer to the buffer of this node.
    char* getBuffer() { return m_pchBuffer; }
    /// @brief Returns the buffer's size.
    /// @return The size of the buffer in this node.
    const S& getBufferSize() { return m_unSize; }
    /// @brief Set the buffer to a specific size.
    /// @attention Don't call this method twice!
    /// @param unSize The new size of the buffer in this node.
    void setBufferSize( S unSize )
    {
      // check if there is a buffer already
      BOOST_ASSERT(NULL==m_pchBuffer);
      // take the size
      m_unSize = unSize;
      // create a new buffer of this size
      m_pchBuffer = new char[m_unSize];
    }
    /// @brief Returns the overall size of this node.
    /// @return The size of this node, when it will be streamed including all
    ///         children and the size and ID information.
    S getOverallSize() const
    {
      // if this node has no children
      if( empty() )
        // return sizeof(data + ID + size)
        return m_unSize+sizeof(I)+sizeof(S);
      else
        // return sizeof(children + ID + size)
        return getChildrenSize()+sizeof(I)+sizeof(S);
    }
    /// @brief Calculates the overall size of all children nodes
    /// @return The size of this all children.
    S getChildrenSize() const throw(BinNodeException*)
    {
      // initialize a variable that gets the size
      size_t unSize=0;
      // iterate all children
      for( const_iterator it=begin(); it!=end(); it++ )
        // add the children's overall size to the size variable
        unSize += ((BinNode*)(*it))->getOverallSize();
      // check if size type S can take unSize
      if( unSize > bit::bitmask<S>() )
        throw BinNodeException(BinNodeException::ChildrenSizeExceedsSizeType,this);
      // return the resulting children size
      return (S)unSize;
    }
    virtual void dumpValue( std::ostream& os )
    {
      if( NULL != m_pchBuffer )
      {
        os << "[" << m_unSize << "] = ";
        dump::hex(os,m_pchBuffer,m_unSize);
        os << '\'';
        dump::ascii(os,m_pchBuffer,m_unSize);
        os << '\'';
      }
    }
    /// @brief Returns the @b container @b bit that is used to mark container nodes in the stream representation.
    /// @return A value of type @c I with a set container bit.
    static I containerBit() { return (I)1 << (sizeof(I)*8-1); }
    /// @brief Checks if a given id is marked as container.
    /// @param id ID to check.
    /// @return True, if the ID is marked as container, false if not.
    static bool isContainer(I id) { return 0!=(id&containerBit()); }
    /// @brief Mark the given id as container.
    /// @param id The ID to mark.
    /// @return The marked ID.
    static I makeContainer(I id) { return id|containerBit(); }
    /// @brief Clear the container mark in the given id.
    /// @param id The ID to clear the container mark from.
    /// @return The ID with the cleared container mark.
    static I unmakeContainer(I id) { return id & ~containerBit(); }
  protected:
    /// @brief Template that covers the most set-cases.
    /// @details
    /// This method is used by most of the set() overrides to store a value into
    /// a binary representation.
    /// @param t The value to store.
    template<class T> void setValue( T t )
    {
      t = host2net(t);
      setBufferSize(sizeof(T));
      memcpy(m_pchBuffer,&t,m_unSize);
    }
    /// @brief Template that covers the most get-cases.
    /// @details
    /// This method is used by most of the get() overrides to restore a value
    /// from the binary representation.
    /// @param rt The value that gets the restored content.
    template<class T> void getValue( T& rt ) const
    {
      // no buffer?
      BOOST_ASSERT(NULL!=m_pchBuffer);
      // illegal size?
      BOOST_ASSERT(sizeof(T)==m_unSize);
      // copy content
      memcpy(&rt,m_pchBuffer,m_unSize);
      rt = net2host(rt);
    }
  private:
    /// @brief Size of the binary representation buffer.
    S           m_unSize;
    /// @brief Binary representation buffer.
    char*       m_pchBuffer;
  };

  /// @brief Index map which is used to map node names with node IDs that are
  ///        used in binary streams instead of strings.
  /// @details
  /// @par Template parameters
  /// @li @c I is the type that is used for the index that is used to represent
  ///     the string names in the binary output.
  /// @li @c S is the size type that is used for the size of the node's payload
  ///     in the binary output.
  /// @ingroup BinStreams
  template<class I=unsigned long,class S=unsigned long>
  class BinIndex
  {
  public:
    /// @brief Adds a id <-> name relation into this index.
    /// @param id ID of the item to add.
    /// @param strName Name of the item to add.
    /// @attention This method crashes with assertion if doublets are added.
    void add( I id, const std::string& strName )
    {
      // Don't use the the container bit!
      BOOST_ASSERT((!BinNode<I,S>::isContainer(id)));
      bool bSuccess;
      bSuccess = m_mapId2Name.insert(std::pair<I,std::string>(id,strName)).second;
      BOOST_ASSERT(bSuccess);
      bSuccess = m_mapName2Id.insert(std::pair<std::string,I>(strName,id)).second;
      BOOST_ASSERT(bSuccess);
    }
    /// @brief Relates a name to an ID.
    /// @param strName The name to relate.
    /// @return The related ID, if one exist
    /// @attention This method crashes with assertion if name isn't related to any ID.
    I name2id(const std::string& strName) const
    {
      // find name
      typename std::map<std::string,I>::const_iterator it=m_mapName2Id.find(strName);
      // not found?
      BOOST_ASSERT( it != m_mapName2Id.end() );
      // return found ID
      return it->second;
    }
    /// @brief Relates an ID to a name.
    /// @param id The ID to relate.
    /// @return Pointer to the related name, if one exist. NULL if not.
    const std::string* id2name(I id) const
    {
      // find ID
      typename std::map<I,std::string>::const_iterator it=m_mapId2Name.find(id);
      // not found?
      if( it == m_mapId2Name.end() )
        return NULL;
      // return found name
      return &it->second;
    }
  private:
    /// @brief Map of ID to name.
    std::map<I,std::string>  m_mapId2Name;
    /// @brief Map of name to ID.
    std::map<std::string,I>  m_mapName2Id;
  };

  /// @brief Binary DOM output stream
  /// @details
  /// @par Template parameters
  /// @li @c I is the type that is used for the index that is used to represent
  ///     the string names in the binary output (see BinIndex).
  /// @li @c S is the size type that is used for the size of the node's payload
  ///     in the binary output.
  /// @par Output format
  /// The format the nodes will be streamed looks like this, if the node
  /// contains data:@n@n
  /// @image html BinaryDataStreamingFormat.png
  /// @n@n And like this if the node contains children nodes: @n@n
  /// @image html BinaryChildrenStreamingFormat.png
  /// @attention A encoding results are in network byte order on both: little
  ///            endian and big endian systems
  /// @ingroup BinStreams
  template<class I=unsigned long, class S=unsigned long>
  class BinOStream
    : public DomOStream
  {
  public:
    /// @brief Constructor that gets an BinIndex.
    /// @param rBinIndex Instance with an BinIndex that relates name identifiers
    ///        to binary IDs.
    /// @attention The index instance won't be copied! The BinOStream stores
    ///            only a reference. So it has to stay valid until this instance
    ///            is destroyed. This shouldn't become a problem because the
    ///            BinIndex is such a "const" thing!
    BinOStream(const BinIndex<I,S>& rBinIndex)
      : DomOStream(new BinNode<I,S>)
      , m_rBinIndex(rBinIndex)
    {}
    /// @brief Writes the DOM into an std::ostream.
    /// @param os The ostream to write to.
    /// @return The size of the generated output in bytes.
    template<class O> S write(O& os)  const throw(BinNodeException*)
    {
      // remember the current write position of ostream
      typename O::streampos pbegin = os.tellp();
      // write all nodes
      for( const_iterator it=getRoot()->begin(); it!=getRoot()->end(); it++ )
        write(os,(BinNode<I,S>*)*it);
      // calculate the size of our output
      return boost::numeric::converter<S,typename O::streampos>(os.tellp() - pbegin);
    }
    void write( char*& rpBuffer, size_t& runSize )  const throw(BinNodeException*)
    {
      // string stream to stream output into
      MemOStream<size_t> mos;
      {
        // write the DOM to the stream
        write(mos);
      }
      mos.detach(rpBuffer,runSize);
    }
  protected:
    template<class O> void write( O& os, BinNode<I,S>* pNode ) const throw(BinNodeException*)
    {
      // has data?
      if( NULL != pNode->getBuffer() )
      {
        // fetch the id for the node's name
        I id = m_rBinIndex.name2id(pNode->getName());
        // convert ID to network byte order
        id = host2net(id);
        // write ID
        os.write((const char*)&id,sizeof(id));
        // get size of our payload
        S unSize = ((BinNode<I,S>*)pNode)->getBufferSize();
        // convert size to network byte order
        unSize = host2net(unSize);
        // write the size
        os.write((const char*)&unSize,sizeof(unSize));
        // write the buffer
        os.write(((BinNode<I,S>*)pNode)->getBuffer(),((BinNode<I,S>*)pNode)->getBufferSize());
      }
      else
      {
        // get the ID of the node's name and mark it as container
        I id = BinNode<I,S>::makeContainer(m_rBinIndex.name2id(pNode->getName()));
        // convert ID to network byte order
        id = host2net(id);
        // write ID
        os.write((const char*)&id,sizeof(id));
        // Calculate sizes of all children
        S unSize = ((BinNode<I,S>*)pNode)->getChildrenSize();
        // convert size to network byte order
        unSize = host2net(unSize);
        // write size
        os.write((const char*)&unSize,sizeof(unSize));
        // write all children
        for( iterator it=pNode->begin(); it!=pNode->end(); it++ )
          write(os,(BinNode<I,S>*)*it);
      }
    }
  private:
    /// @brief Index that maps name identifiers to IDs and backwards.
    const BinIndex<I,S>&    m_rBinIndex;
  };

  /// @brief Exception thrown by the BinIStream class
  /// @ingroup BinStreams
  template<class O> class BinParseException
    : public Exception
  {
  public:
    enum ErrCode { Ok, ObjectToLarge, UnknownNodeId, SizeMissmatch };
    /// @brief Constructor
    /// @param eErrCode Identifier for what has happened exception
    /// @param pos The iostream's write position when exception was thrown.
    explicit BinParseException(ErrCode eErrCode, typename O::streampos pos )
      : m_eErrCode(eErrCode), m_pos(pos) {}
    /// Destructor
    typename O::streampos getPos() const { return m_pos; }
    ErrCode getErrCode() const { return m_eErrCode; }
  private:
    ErrCode               m_eErrCode;
    typename O::streampos m_pos;
  };

  /// @brief Binary DOM input stream
  /// @details
  /// @par Template parameters
  /// @li @c I is the type that is used for the index that is used to represent
  ///     the string names in the binary output (see BinIndex).
  /// @li @c S is the size type that is used for the size of the node's payload
  ///     in the binary output.
  /// @par Input format
  /// The format the nodes will be streamed looks like this, if the node
  /// contains data:@n@n
  /// @image html BinaryDataStreamingFormat.png
  /// @n@n And like this if the node contains children nodes:@n@n
  /// @image html BinaryChildrenStreamingFormat.png
  /// @attention The input format is read in network byte order on both: little
  ///            endian and big endian systems
  /// @ingroup BinStreams
  template<class I=unsigned long, class S=unsigned long>
  class BinIStream
    : public DomIStream
  {
  public:
    /// @brief Constructor that gets an BinIndex.
    /// @details
    /// @param rBinIndex Instance with an BinIndex that relates name identifiers
    ///        to binary IDs.
    /// @param maxSize Maximum size for an object to get created. This value
    ///        should be safe but small as possible. This avoids the parser to
    ///        reserve huge amounts of memory in cause of accidental malformed
    ///        input. Default is the largest possible value of type S.
    /// @attention The index instance won't be copied! The BinOStream stores
    ///            only a reference. So it has to stay valid until this instance
    ///            is destroyed. This shouldn't become a problem because the
    ///            BinIndex is such a "const" thing!
    BinIStream( const BinIndex<I,S>& rBinIndex, S maxSize=~S(0))
      : DomIStream(new BinNode<I,S>), m_MaxSize(maxSize), m_rBinIndex(rBinIndex)
    {}
    /// @brief Parses a binary stream out of an std::istream.
    /// @param is Input stream to read from.
    /// @throw BinParseException May be thrown when parsing fails.
    template<class IS> void read(IS& is) throw(BinParseException<IS>*)
    {
      // while there is something to read
      while( is.peek() >= 0 )
        // read a child node for root
        readchild(is,(BinNode<I,S>*)getRoot());
    }
    /// @brief Parses a binary stream out from a memory buffer
    /// @param pBuffer Pointer to the buffer to read from.
    /// @param unSize Size of the buffer to read from.
    /// @throw BinParseException May be thrown when parsing fails.
    void read( char* pBuffer, size_t unSize )
    {
      MemIStream<size_t> mis(pBuffer,unSize);
      read(mis);
    }
    static unsigned int getHeaderLength() { return sizeof(I)+sizeof(S); }
    template<class IS> void readHeader( IS& is, I& rId, S& rSize ) throw(BinParseException<IS>*)
    {
      // read ID
      is.read((char*)&rId,sizeof(rId));
      // convert ID to host byte order
      rId = net2host(rId);
      // read size
      is.read((char*)&rSize,sizeof(rSize));
      // convert size to host byte order
      rSize = net2host(rSize);
      // check size for not being too huge
      if( rSize > m_MaxSize )
        // Have read a size that exceeds the maximum object size!
        throw BinParseException<IS>(BinParseException<IS>::ObjectToLarge,is.tellg());
    }
  protected:
    /// @brief Read the content of a node.
    /// @param is Input stream to read from
    /// @param pNode Node to fill the data with.
    /// @throw BinParseException May be thrown when parsing fails.
    template<class IS> void read( IS& is, BinNode<I,S>* pNode ) throw(BinParseException<IS>*)
    {
      // space for ID and size
      I  id;
      S  size;
      readHeader(is,id,size);
      // check size for not being too huge
      if( size > m_MaxSize )
        // Have read a size that exceeds the maximum object size!
        throw BinParseException<IS>(BinParseException<IS>::ObjectToLarge,is.tellg());
      // check if this node contains children
      if( BinNode<I,S>::isContainer(id) )
      {
        // get the name that is related to the read ID and clear it's container
        // mark
        const std::string* pName=m_rBinIndex.id2name(BinNode<I,S>::unmakeContainer(id));
        // if name wasn't found
        if( NULL == pName )
          // throw exception
          throw BinParseException<IS>(BinParseException<IS>::UnknownNodeId,is.tellg());
        // fill the nodes name with this name
        pNode->setName(*pName);
        // remember stream position for validating the read size
        typename IS::streampos pos = is.tellg();
        // read until read size is reached
        while( is.tellg() - pos < (typename IS::streampos)size )
          // read a child node
          readchild(is,pNode);
        // check the size
        if( is.tellg() - pos != (typename IS::streampos)size )
          // Size of the containing node doesn't match the sum of the children
          // sizes!
          throw BinParseException<IS>(BinParseException<IS>::SizeMissmatch,is.tellg());
      }
      else
      {
        // get the name that is related to the read ID.
        const std::string* pName=m_rBinIndex.id2name(id);
        // if name wasn't found
        if( NULL == pName )
          // throw exception
          throw BinParseException<IS>(BinParseException<IS>::UnknownNodeId,is.tellg());
        // fill the nodes name with this name
        pNode->setName(*pName);
        // initialize the binary buffer of the node
        ((BinNode<I,S>*)pNode)->setBufferSize(size);
        // read the buffer's content
        is.read(((BinNode<I,S>*)pNode)->getBuffer(),size);
      }
    }
    /// @brief Reads a complete node with ID, size and payload and attach it to
    ///        a parent node.
    /// @param is Input stream to read from.
    /// @param pNode Node that will be the proud new parent.
    /// @throw BinParseException May be thrown when parsing fails.
    template<class IS> void readchild( IS& is, BinNode<I,S>* pNode ) throw(BinParseException<IS>*)
    {
      // create a child
      BinNode<I,S>* pChild = new BinNode<I,S>;
      // attach it to the parent node
      pNode->push_back(pChild);
      // read the node from the binary stream
      read(is,pChild);
    }
  private:
    /// @brief Maximum object size.
    /// @details
    /// If a read size is larger than this value all parsing will be aborted with an
    /// BinParseException("ObjectToLarge").
    S                     m_MaxSize;
    /// @brief Index that maps name identifiers to IDs and backwards.
    const BinIndex<I,S>&  m_rBinIndex;
  };

  typedef BinOStream<unsigned long,unsigned long> Bin32OStream;
  typedef BinIStream<unsigned long,unsigned long> Bin32IStream;
  typedef BinOStream<unsigned short,unsigned short> Bin16OStream;
  typedef BinIStream<unsigned short,unsigned short> Bin16IStream;
  typedef BinOStream<unsigned char,unsigned char> Bin8OStream;
  typedef BinIStream<unsigned char,unsigned char> Bin8IStream;
}

#ifdef _MSC_VER
# pragma warning(default:4290)
#endif
#endif
