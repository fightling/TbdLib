/******************************************************************************
 *  @file domstream.h
 *  @brief DOM streams implementation
 *  @author Patrick Hoffmann
 *  @date 05.11.2006
 ******************************************************************************/

#ifndef __TBD__DOMSTREAM_H
#define __TBD__DOMSTREAM_H

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <list>
#include <deque>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/foreach.hpp>
#include "debug_assert.h"
#include "exception.h"
#include "dump.h"
#include <iostream>

//#define TBD_LOG_DOMSTREAM_OPERATIONS(x) std::cout << x << std::endl

#ifndef TBD_LOG_DOMSTREAM_OPERATIONS
# define TBD_LOG_DOMSTREAM_OPERATIONS(x)
#endif

#ifdef _MSC_VER
# pragma warning(disable:4290)
#endif

/** @defgroup DomStreams DOM Streams
 *  @brief Base classes of document object model (DOM) streams
 *  @ingroup Streaming
 *  @details
 *  @par Purpose
 *  Here you find a set of classes that can be used to manage a document object
 *  model (DOM). This data structure (a complex of DomNode
 *  instances) is a tree of nodes. A node can have children nodes xor it can
 *  have data. This tree can be generated successively by injecting instances of
 *  DomCommand "DomCommand" into a
 *  DomOStream "DomOStream" and or into DomIStream "DomIStream"
 *  to scan it.
 *  @par Usage
 *  To derive your own DOM implementation from this set of classes you need do
 *  derive three classes from DomIStream, DomOStream and DomNode.
 *  The input stream needs to provide all the abstract set members that copy
 *  items into your DomNode derivation. DomNode itself doesn't
 *  contain any data members. It just manages the children nodes, a pointer to
 *  the parent and the name of this node. The data members must be added within
 *  derivation.
 *  @par Runtime errors
 *  DomException will be thrown if requested nodes can't be found.
 *  @par Code errors
 *  If commands have been misused (like miss-matching open/close sequences)
 *  assertions will be initiated, because this happens when your code is wrong
 *  and not by runtime issues.
 */
namespace tbd
{
  /** @brief DOM command codes.
   *  @ingroup DomStreams
   *  @details
   *  These codes are used in DomCommand to navigate in the DOM while writing or
   *  reading it.
   *  @see DomCommand
   */
  enum EDomCommandCode {
    ROOT          = 0x00,
    OPEN          = 0x01,
    ATTRIBUTE     = 0x02,
    CLOSE         = 0x03,
    REOPEN        = 0x04,
    CANCEL        = 0x05,
    USER_DATA     = 0x06,
  };
  /// @brief flags
  enum
  {
    NORMAL        = 0x00,
    HIDDEN        = 0x01,
    MISSING       = 0x02
  };
  typedef char DomCommandFlags;
  class DomCommand;

  /** @brief This class represents a node in a DOM (document object model).
   *  @ingroup DomStreams
   *  @details
   *  A node is specified as an item in a tree. So every node itself can have
   *  zero to n children. These children are stored in the base class
   *  std::vector<DomNode*>.
   */
  class DomNode : public std::vector<DomNode*>
  {
    typedef std::vector<DomNode*> base;
  public:
    /// @brief Standard constructor.
    DomNode(const DomCommand& command);
    /// @brief Destructor clears all children.
    virtual ~DomNode() {
      for( iterator it=begin(); it!=end(); ++it )
        delete *it;
    }
    std::string getPath() const { if( NULL != getParent() ) return getParent()->getPath() + "/" + getName(); else return getName(); }
    /** @brief Return the parent node.
     *  @return Pointer to the parent node or NULL if this is root.
     */
    DomNode* getParent() const { return m_pParent; }
    void setParent(DomNode* pParent) { m_pParent=pParent; }
    /** @brief Indicates if this is the root node of a DOM.
     *  @return true if it is the root node, otherwise false.
     */
    bool isRoot() { return NULL == getParent(); }
    /** @brief Get the name of this node as const.
     *  @return Name of the node as const reference.
     */
    const std::string& getName() const { return m_strName; }
    /** @brief Set the name of this node.
     *  @param strName New name of this node.
     */
    void setName( const std::string& strName ) { m_strName = strName; }
    EDomCommandCode getCommandCode() const { return m_eCommandCode; }
    DomCommandFlags getFlags() const { return m_nDomCommandFlags; }
    bool isAttribute() const { return getCommandCode() == ATTRIBUTE; }
    bool isHidden() const { return 0 != (getFlags() & (HIDDEN|MISSING)); }
    bool isHiddenOnly() const { return 0 != (getFlags() & HIDDEN); }
    bool isMissing() const { return 0 != (getFlags() & MISSING); }
    void miss() { m_nDomCommandFlags |= MISSING; }
    void hide() { m_nDomCommandFlags |= HIDDEN; }
    /// @brief override this method to control value store
    std::string getValueStr() const
    {
      if( m_strValue.empty() && m_unBinaryDataSize > 0 )
      {
        std::stringstream ss;
        ss << dump::hex_ascii(m_pchBinaryData,m_unBinaryDataSize);
        return ss.str();
      }
      return m_strValue;
    }
    /// @brief override this method to control value restore
    void setValueStr( const std::string& str ) { m_strValue = str; }
    void setValueBinary( const char* pchBinaryData, size_t nBinaryDataSize ) { m_pchBinaryData = pchBinaryData; m_unBinaryDataSize = nBinaryDataSize; }
    void getValueBinary( const char*& rpchBinaryData, size_t& rnBinaryDataSize ) const { rpchBinaryData = m_pchBinaryData; rnBinaryDataSize = m_unBinaryDataSize; }
    const char* getBinaryBuffer() const { return m_pchBinaryData; }
    size_t getBinarySize() const { return m_unBinaryDataSize; }
    bool isBinary() const { return m_unBinaryDataSize > 0; }
    void setIndex( const std::string& strIndex ) { m_strIndex = strIndex; }
    const std::string& getIndex() const { return m_strIndex; }
    bool hasOnlyAttributes() const
    {
      BOOST_FOREACH( DomNode* node, *this)
      {
        if( !node->isAttribute() )
          return false;
      }
      return true;
    }
    /** @brief Find first child with a specified name starting from a given
     *         position.
     *  @details
     *  This method is made for using it with a standard iterator for loop:
     *  @code for( DomNode::iterator it=node.begin(); it=node.end(); it++ )
     *          find(it);
     *  @endcode Where the iterator can be incremented in the for loop header as
     *  usual!
     *  @param strName Name to search for
     *  @param it Iterator that holds the search beginning position. This
     *         parameter is given by reference. Beginning with begin() it
     *         successively gets back the next relevant position where to search
     *         for the next node with the given name.
     *  @return Returns a pointer to the node or NULL if no such node has been
     *          found.
     */
    iterator find( const std::string& strName, iterator& it )
    {
      // result iterator
      iterator itResult=end();
      {
        // check if this is an initial call
        if( it==begin() )
        {
          // further...
          while( it!=end() )
          {
            // check if the name matches
            if( (*it)->getName() == strName )
            {
              // store result
              itResult = it;
              // finished search
              break;
            }
            // further more...
            it++;
          }
        }
        else
        {
          // search for the next occurrence of the given name. Usually after a
          // find() call this will be exactly the next find position.
          while( it != end() && (*it)->getName() != strName )
            it++;

          // didn't found anything?
          if( it == end() )
          {
            // mixed iterator with different names or no more such items
            BOOST_ASSERT(0);
          }
          else
          {
            // store result;
            itResult = it;
          }
        }
        // if iterator isn't at end already
        if( it != end() )
        {
          // set iterator 'it' to the position before the next occurrence of the
          // searched name. So that it++ has to be called from outside to set it
          // to the next find position (which might by end()).
          while( (it+1) != end() && (*(it+1))->getName() != strName )
            it++;
        }
      }
      return itResult;
    }
    /** @brief Find first child with a specified name.
     *  @param strName Name to search for
     *  @return Returns a pointer to the node or NULL if no such node has been
     *          found.
     */
    iterator find( const std::string& strName ) { iterator it=begin(); return find(strName,it); }
    /** @brief Append one new node to this node's children.
     *  @details
     *  In addition to vector<DomNode*>::push_back() this method sets the parent
     *  attribute of the added child
     *  @param pNode Node to add.
     */
    void push_back( DomNode* pNode )
    {
      // connect child with this parent
      pNode->setParent(this);
      // add to children
      base::push_back(pNode);
    }
    void erase(DomNode* pNode)
    {
      delete pNode;
      std::vector<DomNode*>::erase(std::find(begin(), end(), pNode));
    }
    unsigned int attributes()
    {
      unsigned int un=0;
      for( iterator it=begin(); it!=end(); ++it )
        un += ((*it)->isAttribute()?1:0);
      return un;
    }
    /** @brief Dumps a human readable form of this node and it's content into an
     *         ostream.
     *  @details
     *  The methods call the override dumpValue() to dump the value that is
     *  defined by the derivate.
     *  @param os Stream to write to
     *  @param nIndent Current hierarchical indention deepness.
     */
    void dump( std::ostream& os, int nIndent=0 )
    {
      os << std::string(nIndent,' ') << getName();
      dumpValue(os);
      os << std::endl;
      for( iterator it=begin(); it!=end(); ++it )
        (*it)->dump(os,nIndent+1);
    }
    /** @brief Method that creates a node.
     *  @details
     *  You must overwrite this method to create the type of node you wish the
     *  DOM to use. Every node that will be created by the DOM will be created
     *  by using this method.
     */
    DomNode* createNode(const DomCommand& command) const { return new DomNode(command); }
    /// @brief Store a boolean in this node
    void set( const bool& b ) { setBoolValue(b); }
    /// @brief Store a char in this node
    void set( const char& ch ) { setValue((int)ch); }
    /// @brief Store a unsigned char in this node
    void set( const unsigned char& uch ) { setValue((unsigned int)uch); }
    /// @brief Store a short in this node
    void set( const short& s ) { setValue(s); }
    /// @brief Store a unsigned short in this node
    void set( const unsigned short& us ) { setValue(us); }
    /// @brief Store a long in this node
    void set( const long& l ) { setValue(l); }
    /// @brief Store a unsigned long in this node
    void set( const unsigned long& ul ) { setValue(ul); }
    /// @brief Store a long long in this node
    void set( const long long& ll ) { setValue(ll); }
    /// @brief Store a unsigned long long in this node
    void set( const unsigned long long& ull ) { setValue(ull); }
    /// @brief Store an integer in this node
    void set( const int& n ) { setValue(n); }
    /// @brief Store an unsigned integer in this node
    void set( const unsigned int& un ) { setValue(un); }
    /// @brief Store a double in this node
    void set( const double& d ) { setValue(d); }
    /// @brief Store a string in this node
    void set( const std::string& str ) { setValue(str); }
    /// @brief Store a binary object in this node
    void set( const void* p, size_t bytes ) { m_pchBinaryData=(char*)p; m_unBinaryDataSize=bytes; }
    /// @brief Read a boolean out of this node
    void get( bool& b ) const { getBoolValue(b); }
    /// @brief Read a char out of this node
    void get( char& rch ) const { getValue(rch); }
    /// @brief Read a unsigned char out of this node
    void get( unsigned char& ruch ) const { getValue(ruch); }
    /// @brief Read a short out of this node
    void get( short& rs ) const { getValue(rs); }
    /// @brief Read a unsigned short out of this node
    void get( unsigned short& rus ) const { getValue(rus); }
    /// @brief Read a long out of this node
    void get( long& rl ) const { getValue(rl); }
    /// @brief Read a unsigned long out of this node
    void get( unsigned long& rul ) const { getValue(rul); }
    /// @brief Read a long long out of this node
    void get( long long& rll ) const { getValue(rll); }
    /// @brief Read a unsigned long long out of this node
    void get( unsigned long long& rull ) const { getValue(rull); }
    /// @brief Read an integer out of this node
    void get( int& rn ) const { getValue(rn); }
    /// @brief Read an unsigned integer out of this node
    void get( unsigned int& run ) const { getValue(run); }
    /// @brief Read a double out of this node
    void get( double& rd ) const { getValue(rd); }
    /// @brief Read a string out of this node
    void get( std::string& rstr ) const { rstr = getValueStr(); }
    /// @brief Read an binary object out of this node
    void get( void*& p, size_t& bytes ) const { p=(void*)m_pchBinaryData; bytes=m_unBinaryDataSize; }
    void* data() const { return m_pData; }
    void data(void* pData) { m_pData = pData; }
    template<class DATA,class PARAM> PARAM* data() const
    {
      return dynamic_cast<PARAM*>(  // wow!
          reinterpret_cast<DATA*>(       // wow!
              const_cast<void*>(    // wow!
                  m_pData)));
    }
  protected:
    void setBoolValue( bool b )                     { std::stringstream ss; ss << getValueStr() << (b ? "1" : "0"); setValueStr(ss.str()); }
    template<class T> void setValue( const T& t )   { std::stringstream ss; ss << getValueStr() << t; setValueStr(ss.str()); }
    void setValue( const char& ch )                 { std::stringstream ss; ss << getValueStr() << (int)ch; setValueStr(ss.str()); }
    void setValue( const unsigned char& uch )       { std::stringstream ss; ss << getValueStr() << (unsigned int)uch; setValueStr(ss.str()); }
    template<class T> void getValue( T& rt ) const  { std::stringstream ss(getValueStr()); ss >> rt; }
    void getValue( char& rch ) const                { int i; std::stringstream ss(getValueStr()); ss >> i; rch = (char)i; }
    void getValue( unsigned char& ruch ) const      { unsigned int u; std::stringstream ss(getValueStr()); ss >> u; ruch=(unsigned char)u; }
    void getBoolValue( bool& rb ) const             { rb = 0 != strtoul(getValueStr().c_str(),NULL,10); }
    /** @brief dump the value as human readable.
     *  @details
     *  The content should be written in one single line!
     *  @param os Stream to dump to.
     */
    void dumpValue( std::ostream& os ) { os << " = " << getValueStr(); }
  private:
    EDomCommandCode         m_eCommandCode;
    DomCommandFlags         m_nDomCommandFlags;
    /// @brief Pointer to parent node or NULL at root.
    DomNode*                m_pParent;
    /// @brief Name of this node.
    std::string             m_strName;
    std::string             m_strValue;
    const char*             m_pchBinaryData;
    size_t                  m_unBinaryDataSize;
    void*                   m_pData;
    std::string             m_strIndex;
  };

  /** @defgroup DomStreamCommands DOM Stream Commands
   * @brief commands which can be used with DOM streams
   * @ingroup DomStreams
   */

  /** @brief Complete DOM navigation command.
   *  @ingroup DomStreamCommands
   *  @details
   *  This class holds a DOM command code and a name. Both can be injected into
   *  DomStream's to open new child nodes or close one.
   *  @see domopen domattr domclose
   */
  class DomCommand
  {
  public:
    DomCommand( EDomCommandCode eCode, DomCommandFlags nFlags=NORMAL )
    : m_eCode(eCode), m_nFlags(nFlags), m_pData(NULL), m_pszName(NULL), m_pit(NULL) {}
    /** @brief Initializes with command code and optional name.
     *  @param eCode Command code.
     *  @param nFlags Command flags.
     *  @param pData user data transportation parameter
     */
    DomCommand( EDomCommandCode eCode, void* pData, DomCommandFlags nFlags=NORMAL )
    : m_eCode(eCode), m_nFlags(nFlags), m_pData(pData), m_pszName(NULL), m_pit(NULL) {}
    /** @brief Initializes with command code and optional name.
     *  @param eCode Command code.
     *  @param nFlags Command flags.
     *  @param pszName Name parameter. If this parameter is NULL name won't
     *         matter within search.
     *  @param strIndex array index parameter
     */
    DomCommand( EDomCommandCode eCode, const char* pszName, const std::string& strIndex=std::string(), DomCommandFlags nFlags=NORMAL )
    : m_eCode(eCode), m_nFlags(nFlags), m_pData(NULL), m_pszName(pszName), m_strIndex(strIndex), m_pit(NULL) {}
    /** @brief Initializes with command code, name parameter and an optional
     *         position.
     *  @attention The iterator parameter may be (depending on command code)
     *             moved to the next significant search position!
     *  @param eCode Command code.
     *  @param nFlags Command flags.
     *  @param pszName Name parameter. If this parameter is NULL name won't
     *         matter within search.
     *  @param rit Position parameter. This parameter is given by reference and
     *         may be moved within command processing!
     */
    DomCommand( EDomCommandCode eCode, const char* pszName, DomNode::iterator& rit, DomCommandFlags nFlags=NORMAL )
      : m_eCode(eCode), m_nFlags(nFlags), m_pData(NULL), m_pszName(pszName), m_pit(&rit) {}
    /** @brief Initializes with command code, name parameter and an optional
     *         position.
     *  @attention The iterator parameter may be (depending on command code)
     *             moved to the next significant search position!
     *  @param eCode Command code.
     *  @param nFlags Command flags.
     *  @param rit Position parameter. This parameter is given by reference and
     *         may be moved within command processing!
     */
    DomCommand( EDomCommandCode eCode, DomNode::iterator& rit, DomCommandFlags nFlags=NORMAL )
      : m_eCode(eCode), m_nFlags(nFlags), m_pData(0), m_pszName(NULL), m_pit(&rit) {}
    /// @return The command code of this command.
    operator EDomCommandCode() const { return m_eCode; }
    DomCommandFlags flags() const { return m_nFlags; }
    /** @brief Returns the name parameter.
     *  @return The name parameter of this command.
     */
    void* data() const { return m_pData; }
    const char* name() const { return m_pszName; }
    const std::string& index() const { return m_strIndex; }
    /** @brief Returns the position.
     *  @return The position iterator of this command.
     */
    DomNode::iterator *getIt() const { return m_pit; }
  private:
    /// @brief Command code of this DOM command.
    EDomCommandCode     m_eCode;
    DomCommandFlags     m_nFlags;
    void*               m_pData;
    const char*         m_pszName;
    std::string         m_strIndex;
    /// @brief Position for this command.
    DomNode::iterator*  m_pit;
  };

  inline DomNode::DomNode(const DomCommand& command) :
    m_eCommandCode(command),
    m_nDomCommandFlags(command.flags()),
    m_pParent(NULL),
    m_strName(command.name()?command.name():std::string()),
    m_pchBinaryData(NULL),
    m_unBinaryDataSize(0),
    m_pData(command.data()),
    m_strIndex(command.index())
  { }

  namespace commands
  {
    /** @brief Generates an DOM open command for a specified name.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open a DOM node that matches the given name.
     *  @code is >> domopen("mynode");
     *  @endcode The code above opens the first child of the current node with the
     *  name "mynode".
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domopen(const char* pszName)
    { return DomCommand(OPEN,pszName); }
    template<class T> __inline DomCommand domopen(const char* pszName, const T& index)
    {
      std::stringstream ss;
      ss << index;
      return DomCommand(OPEN,pszName,ss.str());
    }
    /** @brief Generates an DOM open command for a specified name with a position
     *         given by a DomNode::iterator.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open the next DOM node that matches the given name.
     *  @code for( DomNode::iterator it=is.begin(); it!=is.end(); it++ )
     *          is >> domopen("mynode",it) >> domclose();
     *  @endcode The code above opens (and closes) every child of the current node
     *  with the name "mynode".
     *  @param pszName The name to search for.
     *  @param rit A reference to the position to start the search from.
     *             After the command has been processed rit points to the position
     *             before the next matching item or to end() if no more items with
     *             that name exist..
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domopen(const char* pszName,DomNode::iterator& rit)
    { return DomCommand(OPEN,pszName,rit); }
    /** @brief Generates an DOM open command for the given position that is given
     *         by a DomNode::iterator.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open the DOM node that the iterator points to.
     *  @code for( DomNode::iterator it=is.begin(); it!=is.end(); it++ )
     *          is >> domopen(it) >> domclose();
     *  @endcode The code above opens (and closes) every child of the current
     *  node.
     *  @param rit A reference to the position of the item to open.
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domopen(DomNode::iterator& rit)
    { return DomCommand(OPEN,rit); }
    /** @brief Generates an DOM open attribute command for a specified name.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open a DOM node that matches the given name. The node will be
     *  closed automatically after value injection:
     *  @code is >> doattr("mynode") >> value;
     *  @endcode The code above opens the first child of the current node with the
     *  name "mynode", reads it's value and closes it.
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domattr(const char* pszName)
    { return DomCommand(ATTRIBUTE,pszName); }
    template<class T> __inline DomCommand domattr(const char* pszName, const T& index)
    {
      std::stringstream ss;
      ss << index;
      return DomCommand(ATTRIBUTE,pszName,ss.str());
    }
    /** @brief Generates an DOM open attribute  command for a specified name
     *         with a position given by a DomNode::iterator.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open the next DOM node that matches the given name. The node
     *  will be closed automatically after value injection:
     *  @code for( DomNode::iterator it=is.begin(); it!=is.end(); it++ )
     *          is >> doattr("mynode",it) >> value;
     *  @endcode The code above opens (and closes) every child of the current node
     *  with the name "mynode".
     *  @param pszName The name to search for.
     *  @param rit A reference to the position to start the search from.
     *             After the command has been processed rit points to the position
     *             before the next matching item or to end() if no more items with
     *             that name exist..
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domattr(const char* pszName,DomNode::iterator& rit)
    { return DomCommand(ATTRIBUTE,pszName,rit); }
    /** @brief Generates an DOM open attribute command for the given position that
     *         is given by a DomNode::iterator.
     *  @details
     *  This method generates a DomCommand class instance that includes the
     *  command to open the DOM node that the iterator points to. The node will be
     *  closed automatically after value injection:
     *  @code for( DomNode::iterator it=is.begin(); it!=is.end(); it++ )
     *          is >> domattr(it) >> value;
     *  @endcode The code above opens (and closes) every child of the current
     *  node.
     *  @param rit A reference to the position of the item to open.
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domattr(DomNode::iterator& rit)
    { return DomCommand(ATTRIBUTE,rit); }
    /** @brief acts like domattr() but sets the hidden flag of the node
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domhattr(const char* pszName)
    { return DomCommand(ATTRIBUTE,pszName,std::string(),HIDDEN); }
    /// @ingroup DomStreamCommands
    template<class T> __inline DomCommand domhattr(const char* pszName, const T& index)
    {
      std::stringstream ss;
      ss << index;
      return DomCommand(ATTRIBUTE,pszName,ss.str(),HIDDEN);
    }
    /// @ingroup DomStreamCommands
    __inline DomCommand domhopen(const char* pszName)
    { return DomCommand(OPEN,pszName,std::string(),HIDDEN); }
    /// @ingroup DomStreamCommands
    template<class T> __inline DomCommand domhopen(const char* pszName, const T& index)
    {
      std::stringstream ss;
      ss << index;
      return DomCommand(OPEN,pszName,ss.str(),HIDDEN);
    }
    /** @brief Generates an DOM close attribute command.
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domclose()
    { return DomCommand(CLOSE); }
    /** @brief Reopens last closed node
     *  @ingroup DomStreamCommands
     */
    __inline DomCommand domreopen()
    { return DomCommand(REOPEN); }
    /// @ingroup DomStreamCommands
    __inline DomCommand domcancel()
    { return DomCommand(CANCEL); }
    /// @ingroup DomStreamCommands
    template<class DATA,class PARAM> DomCommand domdata(const PARAM* pData)
    {
      if( pData )
      {
  #ifdef _DEBUG
        // paranoia
        BOOST_ASSERT( dynamic_cast<const DATA*>(pData) );
  #endif
        // put data into a command
        return DomCommand(USER_DATA,
            const_cast<void*>(                  // yeah!
                reinterpret_cast<const void*>(  // yeah!
                    dynamic_cast<const DATA*>(  // yeah!
                        pData))),HIDDEN);
      }
      // commadn w/o data
      return DomCommand(USER_DATA,HIDDEN);
    }
    /// @ingroup DomStreamCommands
    template<class T, class S> std::pair<const char*,size_t> dombinary(const T* p, S size)
    { return std::pair<const char*,size_t>((char*)p,size); }
  }
  using namespace commands;

  /** @brief Exception thrown by the DomStream class
   *  @ingroup DomStreams
   */
  class DomException
    : public Exception
  {
  public:
    enum ErrCode { Ok, NodeNotFound };
    /** @brief Create a DOM exception
     *  @param eErrCode Identifier for what has happened exception
     *  @param cmd Command that was read, when this exceptions has happened
     *  @param pNode error source
     */
    explicit DomException(ErrCode eErrCode, const DomCommand& cmd, DomNode* pNode)
      : m_eErrCode(eErrCode), m_cmd(cmd)
    {
      m_strNodePath = pNode->getPath();
    }
    virtual ~DomException() throw() {}
    ErrCode getErrCode() const { return m_eErrCode; }
    const DomCommand& getCommand() const { return m_cmd; }
    const std::string& getNodePath() const { return m_strNodePath; }
    virtual void explain(std::stringstream& ss) const
    {
      switch( m_eErrCode )
      {
      case Ok:            break;
      case NodeNotFound:  ss << "node with the name '" << getNodePath() << "/" << getCommand().name() << "' expected"; break;
      default:            ss << "(unknown error)";
      }
    }
  private:
    ErrCode     m_eErrCode;
    DomCommand  m_cmd;
    std::string m_strNodePath;
  };

  /** @defgroup DomStreamStreams DOM Streams
   * @brief DOM streams
   * @ingroup DomStreams
   */

  /** @brief Base class for all DOM streams.
   *  @ingroup DomStreamStreams
   *  @details
   *  This class manages the DOM tree and provides methods for navigation of a
   *  current stream position, that are initiated by a DomCommand.
   */
  class DomStream
  {
  public:
    /** @brief Constructor that gets the root node.
     *  @param pRoot Root node instance. Derived classes should use this
     *         parameter to inject their own node class type.
     */
    DomStream(DomNode* pRoot)
      : m_pRoot(pRoot)
      , m_pcCurrent(pRoot)
      , m_eState(ROOT)
    { }
    /// @brief Destructor delete the root node and all it's children.
    ~DomStream() { delete m_pRoot; }
    /** @brief Returns the root node.
     *  @return A pointer to the root node
     */
    DomNode* getRoot() const { return m_pRoot; }
    /// @brief Check if the current node has a child with the given name
    bool exists(const char* pszName)
    { return getCurrent()->find(pszName) != getCurrent()->end(); }
    /// @brief An type for an iterator that represents a stream position.
    typedef DomNode::iterator         iterator;
    /** @brief Returns the begin of the array of children.
     *  @return Begin position as iterator.
     */
    iterator begin() { return getCurrent()->begin(); }
    /** @brief Returns the end of the array of children.
     *  @return End position as iterator.
     */
    iterator end() { return getCurrent()->end(); }
    /** @brief An type for an iterator that represents a stream position with
     *         const access.
     */
    typedef DomNode::const_iterator   const_iterator;
    /** @brief Returns the begin of the array of children.
     *  @return Begin position as const_iterator.
     */
    const_iterator begin() const { return getCurrent()->begin(); }
    /** @brief Returns the end of the array of children.
     *  @return End position as const_iterator.
     */
    const_iterator end() const { return getCurrent()->end(); }
    /** @brief returns the first child node
     *  @return The first child node (if this node has children).
     */
    DomNode* front() { return getCurrent()->front(); }
    /** @brief returns the first child node as const
     *  @return The first child node (if this node has children).
     */
    const DomNode* front() const { return getCurrent()->front(); }
    /** @brief Checks if the current node has children
     *  @return true, if the current node has one or more children.
     */
    bool empty() const { return getCurrent()->empty(); }
    DomNode* detach() {
      DomNode* node=m_pRoot;
      m_pRoot = NULL;
      m_pcCurrent = NULL;
      return node; }
  protected:
    /** @brief Returns the node at the current stream position
     *  @return Pointer to the current node.
     */
    DomNode* getCurrent() { return m_pcCurrent; }
    /** @brief Returns the node at the current stream position
     *  @return Const pointer to the current node.
     */
    const DomNode* getCurrent() const { return m_pcCurrent; }
    /** @brief Set the current node.
     *  @param pNewCurrent Pointer to the new current node.
     */
    void setCurrent( DomNode* pNewCurrent ) { m_pcCurrent = pNewCurrent; }
    /** @brief Set the state member to a command code.
     *  @param eCommandCode An instance of DomCommand that includes the command
     *         code, that will be stored.
     */
    void setState( EDomCommandCode eCommandCode ) { m_eState=eCommandCode; }
    /** @brief Returns the state.
     *  @return The state in kind of a command code.
     */
    const EDomCommandCode& getState() { return m_eState; }
    /** @brief Closes the current node,
     *  @details
     *  Makes the current's parent to be the current node
     */
    void closeNode()
    {
      // check if current node isn't at root
      if( !getCurrent()->isRoot() )
      {
        TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomStream::closeNode(): close node '" << getCurrent()->getPath() << "'");
        // set the current node
        setCurrent(getCurrent()->getParent());
        // store open code in state
        setState(OPEN);
      }
      else
      {
        // undershooting root node. There might be a close too many.
        BOOST_ASSERT(0);
        // store closed code in state
        setState(CLOSE);
      }
    }
    /// @brief Check if current state is OPEN or ATTRIBUTE.
    void checkOpen()
    {
      // bad placed value
      BOOST_ASSERT(OPEN==getState() || ATTRIBUTE==getState());
    }
    /// @brief Check if current state is OPEN or ATTRIBUTE.
    bool isAttr()
    {
      return ATTRIBUTE==getCurrent()->getCommandCode();
    }
  private:
    /// Root node of the DOM tree
    DomNode*                m_pRoot;
    /// Current stream position
    DomNode*                m_pcCurrent;
    /// State that is used to manage navigation with DomCommand.
    EDomCommandCode         m_eState;
  };
  /** @brief This class is for deriving output streams.
   *  @ingroup DomStreamStreams
   *  @details
   *  To create a new output stream based on DomStream you must derive your
   *  class from DomOStream. You may add read and write methods, that parse or
   *  produce the protocol you like. For example have a look at XmlOStream and
   *  BinOStream
   */
  class DomOStream
    : public DomStream
  {
  public:
    /** @brief Constructor that gets the initial root node instance
     *  @param pRoot Node instance that will be used as root node and to create
     *         all sub nodes via pRoot->createNode().
     */
    DomOStream(DomNode* pRoot=new DomNode(ROOT)) : DomStream(pRoot), m_bData(true), m_bShowMissing(false) {}
    /** @brief Stream operator that receives a DOM command,
     *  @param cCommand command to inject into the stream.
     *  @return This instance as reference
     */
    DomOStream& operator<<( const DomCommand& cCommand )
    {
      switch( cCommand )
      {
      case ATTRIBUTE:
      case OPEN:  if( isAttr() ) closeNode(); newNode(cCommand); break;
      case CLOSE: if( isAttr() ) closeNode(); closeNode(); break;
      case REOPEN: BOOST_ASSERT(!isAttr()); reopenNode(); break;
      case CANCEL: cancelNode(); break;
      case USER_DATA: getCurrent()->data(data()?cCommand.data():0); break;
      default:
        // unknown command code
        BOOST_ASSERT(0);
      }
      return *this;
    }
    DomOStream& operator<<( const bool& b )                   { checkOpen(); getCurrent()->set(b);    return *this; }
    DomOStream& operator<<( const char& ch )                  { checkOpen(); getCurrent()->set(ch);   return *this; }
    DomOStream& operator<<( const unsigned char& uch )        { checkOpen(); getCurrent()->set(uch);  return *this; }
    DomOStream& operator<<( const short& s )                  { checkOpen(); getCurrent()->set(s);    return *this; }
    DomOStream& operator<<( const unsigned short& us )        { checkOpen(); getCurrent()->set(us);   return *this; }
    DomOStream& operator<<( const long& l )                   { checkOpen(); getCurrent()->set(l);    return *this; }
    DomOStream& operator<<( const unsigned long& ul )         { checkOpen(); getCurrent()->set(ul);   return *this; }
    DomOStream& operator<<( const long long& ll )             { checkOpen(); getCurrent()->set(ll);   return *this; }
    DomOStream& operator<<( const unsigned long long& ull )   { checkOpen(); getCurrent()->set(ull);  return *this; }
    DomOStream& operator<<( const int& n )                    { checkOpen(); getCurrent()->set(n);    return *this; }
    DomOStream& operator<<( const unsigned int& un )          { checkOpen(); getCurrent()->set(un);   return *this; }
    DomOStream& operator<<( const double& d )                 { checkOpen(); getCurrent()->set(d);    return *this; }
    DomOStream& operator<<( const std::string& str )          { checkOpen(); getCurrent()->set(str);  return *this; }
    DomOStream& operator<<( const char* psz )                 { checkOpen(); getCurrent()->set(std::string(psz));  return *this; }
    DomOStream& operator<<( std::pair<const char*,size_t> binary ) { checkOpen(); getCurrent()->set((void*)binary.first,binary.second);  return *this; }
    template<class T> DomOStream& writeSeq( const T& seq ) {
      checkOpen();
      const char* pszName = getCurrent()->getName().c_str();
      if( seq.empty() )
        *this << domcancel();
      else {
        typename T::const_iterator it=seq.begin();
        *this << *it++;
        for( ;it!=seq.end(); it++ )
          *this << domclose() << domopen(pszName) << *it;
      }
      return *this;
    }
    template<class T> DomOStream& operator<<( const std::vector<T>& v)        { return writeSeq( v ); }
    template<class T> DomOStream& operator<<( const std::list<T>& v)          { return writeSeq( v ); }
    template<class T> DomOStream& operator<<( const std::deque<T>& v)         { return writeSeq( v ); }
    template<class T> DomOStream& operator<<( const boost::ptr_vector<T>& v ) { return writeSeq( v ); }
    template<class T> DomOStream& operator<<( const boost::ptr_list<T>& v )   { return writeSeq( v ); }
    template<class T> DomOStream& operator<<( const boost::optional<T>& ot)
    {
      if (!ot)
        *this << domcancel();
      else
        *this << ot.get();
      return *this;
    }
    bool data() const { return m_bData; }
    bool data(bool bData) { bool b=m_bData; m_bData = bData; return b; }
    bool showMissing() const { return m_bShowMissing; }
    bool showMissing(bool bShowMissing) { bool b=m_bShowMissing; m_bShowMissing = bShowMissing; return b; }
  protected:
    /** @brief Creates a new node and opens it.
     *  @details
     *  The new node will be appended to the current one's child list. This
     *  message is useful for O-Stream implementations to build the DOM.
     *  @param cCommand Command that was used to initiate this call
     */
    void newNode(const DomCommand& cCommand)
    {
      TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomOStream::newNode(): new node '" << getCurrent()->getPath() << "' + '" << cCommand.name() << "'" );
      // only OPEN, ATTRIBUTE or HIDDEN command should get here
      BOOST_ASSERT( OPEN == cCommand || ATTRIBUTE == cCommand );
      // create a new node with the custom factory
      DomNode *pNode = getRoot()->createNode(cCommand);
      // append the new node to this' children
      getCurrent()->push_back(pNode);
      // set the current node to the new node
      setCurrent(pNode);
      // store command code in state
      setState(cCommand);
    }
    void closeNode()
    {
      if( getState() == CANCEL )
      {
        DomNode* closed=getCurrent();
        DomStream::closeNode();
        getCurrent()->erase(closed);
      }
      else
        DomStream::closeNode();
    }
    void reopenNode()
    {
      // check if current node isn't empty
      if( !getCurrent()->empty() )
      {
        TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomOStream::reopenNode(): reopen node '" << getCurrent()->back()->getPath() << "'");
        // set the current node
        setCurrent(getCurrent()->back());
        // store open code in state
        setState(OPEN);
      }
      else
      {
        // missing children for this operation
        BOOST_ASSERT(0);
      }
    }
    void cancelNode()
    {
      if( m_bShowMissing )
      {
        getCurrent()->miss();
        *this << std::string();
      }
      else
      {
        // remove missing optional nodes in this mode
        setState(CANCEL);
      }
    }
  private:
    /// store user data
    bool m_bData;
    bool m_bShowMissing;
  };
  /** @brief This class is for deriving output streams.
   *  @ingroup DomStreamStreams
   *  @details
   *  To create a new output stream based on DomStream you must derive your
   *  class from DomOStream. You may add read and write methods, that parse or
   *  produce the protocol you like. For example have a look at XmlOStream and
   *  BinOStream
   */
  class DomIStream
    : public DomStream
  {
  public:
    /** @brief Constructor that gets the initial root node instance
     *  @param pRoot Node instance that will be used as root node and to create
     *         all sub nodes via pRoot->createNode().
     */
    DomIStream(DomNode *pRoot=new DomNode(ROOT)) : DomStream(pRoot) {}
    DomNode* detach()
    {
      while( !m_stckFakedOpens.empty() )
        m_stckFakedOpens.pop();
      return  DomStream::detach();
    }
    bool missing() const { return !m_stckFakedOpens.empty(); }
    /** @brief Stream operator that receives a DOM command,
     *  @param cCommand command to inject into the stream.
     *  @return This instance as reference
     *  @throw DomException If a node was not found.
     */
    DomIStream& operator>>( const DomCommand& cCommand )
    {
      switch( cCommand )
      {
      case ATTRIBUTE:
      case OPEN:  if( isAttr() ) closeNode(); openNode(cCommand);  break;
      case CLOSE: if( isAttr() ) closeNode(); closeNode(); break;
      case CANCEL: closeNode(); break;
      case USER_DATA: getCurrent()->data(cCommand.data()); break;
      default:
        // unknown command code
        BOOST_ASSERT(0);
      }
      return *this;
    }
    void rewind() { setCurrent(getRoot()); }
    DomIStream& operator>>( bool& b )                  { checkOpen(); getCurrent()->get(b);     return *this; }
    DomIStream& operator>>( char& rch )                { checkOpen(); getCurrent()->get(rch);   return *this; }
    DomIStream& operator>>( unsigned char& ruch )      { checkOpen(); getCurrent()->get(ruch);  return *this; }
    DomIStream& operator>>( short& rs )                { checkOpen(); getCurrent()->get(rs);    return *this; }
    DomIStream& operator>>( unsigned short& rus )      { checkOpen(); getCurrent()->get(rus);   return *this; }
    DomIStream& operator>>( long& rl )                 { checkOpen(); getCurrent()->get(rl);    return *this; }
    DomIStream& operator>>( unsigned long& rul )       { checkOpen(); getCurrent()->get(rul);   return *this; }
    DomIStream& operator>>( long long& rll )           { checkOpen(); getCurrent()->get(rll);   return *this; }
    DomIStream& operator>>( unsigned long long& rull ) { checkOpen(); getCurrent()->get(rull);  return *this; }
    DomIStream& operator>>( int& rn )                  { checkOpen(); getCurrent()->get(rn);    return *this; }
    DomIStream& operator>>( unsigned int& run )        { checkOpen(); getCurrent()->get(run);   return *this; }
    DomIStream& operator>>( double& rd )               { checkOpen(); getCurrent()->get(rd);    return *this; }
    DomIStream& operator>>( std::string& rstr )        { checkOpen(); getCurrent()->get(rstr);  return *this; }
    DomIStream& operator>>( std::pair<const char**,size_t*> binary ) { checkOpen(); getCurrent()->get((void*&)*binary.first,*binary.second); return *this; }
    template<class T> DomIStream& readSeq( T& seq )
    {
      //TBD_LOG("DomIStream& readSeq( DomIStream& dis, " << typeid(T) << "& seq )");
      seq.clear();
      if (!missing())
      {
        const std::string& name=getCurrent()->getName();
        DomNode* parent = getCurrent()->getParent();
        for( iterator it=parent->begin(); it!=parent->end(); it++ )
        {
          if( (*it)->getName() == name )
          {
            setCurrent(*it);
            seq.resize(seq.size()+1);
            *this >> seq.back();
          }
        }
      }
      return *this;
    }
    template<class T> DomIStream& operator>>( std::vector<T>& v)          { return readSeq( v ); }
    template<class T> DomIStream& operator>>( std::list<T>& v)            { return readSeq( v ); }
    template<class T> DomIStream& operator>>( boost::ptr_vector<T>& v )   { return readSeq( v ); }
    template<class T> DomIStream& operator>>( boost::ptr_list<T>& v )     { return readSeq( v ); }
    template<class T> DomIStream& operator>>( boost::optional<T>& ot )
    {
      if (missing())
        ot.reset();
      else
      {
        ot.reset(T());
        *this >> ot.get();
      }
      return *this;
    }
  protected:
    /** @brief Opens a child node.
     *  @details
     *  Depending on the DOM command this method opens the first respectively
     *  the next node with a given name.
     *  @param cCommand Command that was used to initiate this call
     *  @throw DomException If a node was not found.
     */
    void openNode( const DomCommand& cCommand )
    {
      // only OPEN, ATTRIBUTE or HIDDEN command should get here
      BOOST_ASSERT( OPEN == cCommand || ATTRIBUTE == cCommand );
      // iterator that the position of the node that will be opened
      iterator it;
      {
        // if the command transports a name
        if( NULL != cCommand.name() )
        {
          // does it provide an iterator?
          if( NULL != cCommand.getIt() )
            // find the next node with the given name
            it=getCurrent()->find(cCommand.name(),*cCommand.getIt());
          else
            // find the first node with the given name
            it=getCurrent()->find(cCommand.name());
        }
        else
        {
          // an iterator has to be in the command
          BOOST_ASSERT( NULL != cCommand.getIt() );
          // take the current node for result
          it = *cCommand.getIt();
        }
      }
      // if a node was found
      if( it != getCurrent()->end() )
      {
        TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomIStream::openNode(): open node '" << (*it)->getPath() << "'");
        // set the current node
        setCurrent(*it);
      }
      else
      {
        TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomIStream::openNode(): fake open node '" << getCurrent()->getPath() << "' + '" << cCommand.name() << "'");
        m_stckFakedOpens.push(cCommand);
      }
      // store command code in state
      setState(cCommand);
    }
    /** @brief Closes the current node,
     *  @details
     *  Makes the current's parent to be the current node
     */
    void closeNode()
    {
      if( missing() )
      {
        TBD_LOG_DOMSTREAM_OPERATIONS("tbd::DomIStream::closeNode(): fake close node '" << getCurrent()->getPath() << "' - '" << m_stckFakedOpens.top().name() << "'");
        m_stckFakedOpens.pop();
        setState(OPEN);
      }
      else
        DomStream::closeNode();
    }
    bool isAttr()
    {
      return (missing() && ATTRIBUTE == m_stckFakedOpens.top()) || DomStream::isAttr();
    }
  private:
    std::stack<DomCommand>  m_stckFakedOpens;
  };
}
#ifdef _MSC_VER
# pragma warning(default:4290)
#endif
#endif
