///////////////////////////////////////////////////////////////////////////////
/// @file filestream.h
/// @brief File I/O streams based on basic stream intefaces
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__FILESTREAM_H
#define __TBD__FILESTREAM_H

#include "stream.h"
#include "bit.h"

#include <errno.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <share.h>
# include <sys/types.h>
#else
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <stdint.h>
#endif

/// @defgroup FileStreams File Streams
/// @brief Streaming classes for handling (large) files
/// @ingroup Streams
namespace tbd
{
#if defined( _WIN32 )
  typedef _int64 huge_streampos;
#else
  typedef off64_t huge_streampos;
#endif
  /** @brief This base class for all file stream classes handles access to large files (>4GB)
   *  @ingroup FileStreams
   */
  class FileStreamBase
    : public Stream<huge_streampos>
  {
  public:
    enum OpenMode
    {
      OM_CLOSED     = -1,
#if defined( _WIN32 )
      OM_READONLY   = _O_SEQUENTIAL|_O_BINARY|_O_RDONLY,
      OM_WRITEONLY  = _O_SEQUENTIAL|_O_BINARY|_O_CREAT|_O_WRONLY|_O_TRUNC,
      OM_APPEND     = _O_SEQUENTIAL|_O_BINARY|_O_CREAT|_O_WRONLY|_O_APPEND,
      OM_READWRITE  = _O_SEQUENTIAL|_O_BINARY|_O_RDWR,
      OM_RWCREATE   = _O_SEQUENTIAL|_O_BINARY|_O_CREAT|_O_RDWR
#else
      /// @brief open an existing file for read only and seek to begin of file
      OM_READONLY   = O_RDONLY,
      /// @brief create a new or open an existing  file for write only and truncate it to 0 bytes
      OM_WRITEONLY  = O_RDWR  |O_CREAT|O_TRUNC,
      /// @brief create a new or open an existing file for write only and seek to end of file
      OM_APPEND     = O_RDWR  |O_CREAT|O_APPEND,
      /// @brief open an existing file for read and write and seek to begin of file
      OM_READWRITE  = O_RDWR,
      /// @brief create new or open an existing file for read and write and seek to begin of file
      OM_RWCREATE   = O_RDWR  |O_CREAT
#endif
    };
    enum Origin
    {
      ORG_BEG = SEEK_SET,
      ORG_CUR = SEEK_CUR,
      ORG_END = SEEK_END
    };
    enum State
    {
      ok, failed
    };
  protected:
    /// @brief Constructor
    FileStreamBase() : m_hFile(-1), m_eState(ok), m_eOpenMode(OM_CLOSED) {}
    /// @brief Destructor
    virtual ~FileStreamBase() { close(); }
    void open( const std::string& rcFilename, OpenMode eOpenMode, int iPermMode=0777 )
    {
      m_eOpenMode = eOpenMode;
      if( rcFilename.empty() || 0 <= m_hFile )
      {
        // file already opened
        BOOST_ASSERT(0);
        return;
      }
#if defined( _WIN32 )
      m_strFileName = rcFilename;
# if( _MSC_VER >= 1400 )
      if( 0 !=  ::_sopen_s( &m_hFile, rcFilename.c_str(), eOpenMode, _SH_DENYWR, iPermMode ) )
        m_hFile = -1;
# else
      m_hFile = ::_sopen( rcFilename.c_str(), eOpenMode, _SH_DENYWR, iPermMode );
# endif
#elif defined(__USE_LARGEFILE64)
      m_hFile = open64( rcFilename.c_str(), eOpenMode, iPermMode );
#else
      m_hFile = open( rcFilename.c_str(), eOpenMode, iPermMode );
#endif
    }
    void close()
    {
      if( is_open() )
      {
        /// @neverdoc
#if defined( _WIN32 )
      ::_close( m_hFile );
      m_strFileName = "";
#else
      ::close( m_hFile );
#endif
        m_hFile = -1;
        m_eOpenMode = OM_CLOSED;
      }
    }
    bool is_open() const { return m_hFile >= 0; }
    /** @brief Read exactly size from this stream to pch
     *  @param pch Pointer to a piece of memory of at least size bytes
     *  length.
     *  @param size Number of bytes to read.
     */
    void read( char* pch, size_t size )
    {
      // paranoia checks
      if( !is_open() || 0==size )
      {
        BOOST_ASSERT(0);
        m_eState = failed;
        m_nGCount = 0;
        return;
      }
      // init
      m_nGCount = 0;

      while( 0 != size )
      {
        int nRet;
        // try to read from file
#if defined( _WIN32 )
        nRet = ::_read( m_hFile, (void*)(pch + m_nGCount), (unsigned int)size );
#else
        nRet = ::read( m_hFile, (void*)(pch + m_nGCount), size );
#endif
        // return if error occurred
        if( 0 > nRet )
        {
          BOOST_ASSERT(0);
          m_eState = failed;
          return;
        }
        // check if we reached end of file
        else if( 0 == nRet )
        {
          m_eState = (0 == m_nGCount)?failed:ok;
          return;
        }
        size -= nRet;
        m_nGCount += nRet;
      }
    }
    /** @brief Write size bytes from pch into this stream.
     *  @param pch Pointer to the content to write.
     *  @param size Size of the content.
     */
    void write( const char* pch, size_t size )
    {
      // paranoia checks
      if( !is_open() )
      {
        BOOST_ASSERT(0);
        m_eState = failed;
        return;
      }
      if( 0==size )
        return;
      int nRet;
#if defined( _WIN32 )
      nRet = ::_write( m_hFile, (void*)pch, (unsigned int)size );
#else
      nRet = ::write( m_hFile, (void*)pch, size );
#endif
      // return if error occurred
      if( 0 > nRet )
      {
        BOOST_ASSERT(0);
        m_eState = failed;
        return;
      }
    }
    /// @brief Flushes the stream to memory.
    void flush()
    {
#if defined( _WIN32 )
      _commit(m_hFile);
#else
      ::fsync(m_hFile);
#endif
    }
    void unget()
    {
      seek(-1,ORG_CUR);
    }
    int peek()
	  {
	    char ch;
      // paranoia checks
      if( !is_open() )
        return -1;
      int nRet;
      // try to read from file
#if defined( _WIN32 )
      nRet = ::_read( m_hFile, (void*)&ch, 1 );
#else
      nRet = ::read( m_hFile, (void*)&ch, 1 );
#endif
      // return if error occurred
      if( 0 > nRet )
        return -1;
      // check if we reached end of file
      else if( 0 == nRet )
      {
        m_eState = (0 == m_nGCount)?failed:ok;
        return -1;
      }
      seek(-1,ORG_CUR);
      return ch;
	  }
    /// @brief Returns the current read position.
    streampos tell() const
    {
      if( !is_open() )
        return bit::bitmask<streampos>();
      streampos nG;
#if defined( _WIN32 )
      nG = _telli64( m_hFile);
#else
      nG = lseek64( m_hFile, 0, SEEK_CUR );
#endif
      // and return
      return nG;
    }
    /// @brief Sets the current read position.
    void seek(const streampos& g, Origin eOrigin )
    {
      if( !is_open() )
      {
        BOOST_ASSERT(0);
        return;
      }
      streampos nNewG;
#if defined( _WIN32 )
      nNewG = _lseeki64( m_hFile, g, eOrigin );
#else
      nNewG = lseek64( m_hFile, g, eOrigin );
#endif
      m_eState = (nNewG<0)?failed:ok;
    }
    streampos gcount() const { return m_nGCount; }
    bool fail() const { return m_eState == failed; }
    OpenMode mode() const { return m_eOpenMode; }
    bool canWrite() const { return m_eOpenMode == OM_WRITEONLY || m_eOpenMode == OM_APPEND || m_eOpenMode == OM_READWRITE  || m_eOpenMode == OM_RWCREATE; }
    int err_no() const { return errno; }
    void error( std::ostream& os ) const
    {
      if( m_hFile < 0 )
      {
        os << "FileStreamBase ERROR: ";
        switch(errno)
        {
        case EACCES:  os << "The file does not exist and the parent directory denies write permission of the file to be created. A component of the path name prefix denies search permission. The file exists and the permissions specified by oflag are denied. O_TRUNC is specified and write permission is denied." << std::endl; break;
        case EEXIST:  os << "O_CREAT and O_EXCL are set, and the named file exists." << std::endl; break;
        case EFAULT:  os << "The pathname parameter is not a valid pointer." << std::endl; break;
        case EINTR:   os << "The call was interrupted by a signal." << std::endl; break;
        case EINVAL: os << "The oflag parameter contains invalid flags, or flags specified in oflag are invalid for pathname." << std::endl; break;
        case EISDIR: os << "The oflag parameter specifies O_WRONLY or O_RDWR and the named file is a directory." << std::endl; break;
        case EMFILE: os << "This process is currently using too many open file descriptors." << std::endl; break;
        case ENAMETOOLONG: os << "The pathname parameter string length exceeds PATH_MAX or a path name component is longer than NAME_MAX." << std::endl; break;
        case ENFILE: os << "Too many files are currently open within the system." << std::endl; break;
        case ENOENT: os << "O_CREAT is not set and the named file does not exist. O_CREAT is set and either the pathname prefix does not exist or the pathname parameter points to an empty string." << std::endl; break;
        case ENOMEM: os << "The system is unable to allocate memory for the descriptor." << std::endl; break;
        case ENOSPC: os << "O_CREAT is set, and the directory that would contain the file cannot be extended." << std::endl; break;
        case ENOTDIR: os << "A component of the pathname prefix is not a directory." << std::endl; break;
        case ENXIO: os << "O_NONBLOCK is set, the named file is a FIFO, O_WRONLY is set, and no process has the file open for reading." << std::endl; break;
#if !defined( _WIN32 )
        case EOVERFLOW: os << "The named file is a regular file and the size of the file cannot be represented correctly in an object of type off_t." << std::endl; break;
#endif
        case EROFS: os << "The named file resides on a read-only file system; if the file does not exist, either O_WRONLY, O_RDWR, O_CREAT, or O_TRUNC is set in the oflag parameter." << std::endl; break;
        default:
          ;
        }
      }
    }
#if defined( _WIN32 )
    struct _stat64 stat() const {
      struct _stat64 filestat;
      _stat64(m_strFileName.c_str(),&filestat);
      return filestat;
    }
#elif defined(__USE_LARGEFILE64)
    struct stat64 stat() const {
      struct stat64 filestat;
      fstat64(m_hFile,&filestat);
      return filestat;
    }
#else
    struct stat stat() const {
      struct stat filestat;
      fstat(m_hFile,&filestat);
      return filestat;
    }
#endif
  private:
    int         m_hFile;
    State       m_eState;
    streampos   m_nGCount;
    OpenMode    m_eOpenMode;
#if defined( _WIN32 )
    std::string m_strFileName;
#endif
  };
  /// @ingroup FileStreams
  class FileIStream
    : virtual public FileStreamBase
    , public IStream<huge_streampos>
  {
    friend class FileStream;
    IMPLEMENT_ISTREAM_OPERATORS(FileIStream,huge_streampos);
#if !defined( _WIN32 )
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
#endif
  public:
    typedef huge_streampos streampos;
    /// @brief Constructor
    FileIStream() {}
    /// @brief Constructor opening a file
    FileIStream( const std::string& rcFilename ) { open(rcFilename); }
    /// @brief Destructor
    ~FileIStream() {}
    void open( const std::string& rcFilename )
    {
      FileStreamBase::open(rcFilename,OM_READONLY,_S_IREAD);
    }
    virtual bool is_open() const { return FileStreamBase::is_open(); }
    virtual void read( char* pch, size_t size ) { FileStreamBase::read(pch,size); }
    /// @brief Returns the current read position.
    virtual streampos tellg() const { return FileStreamBase::tell(); }
    /// @brief Sets the current read position.
    virtual void seekg(const streampos& g) { FileStreamBase::seek(g,ORG_BEG); }
    virtual void seekg2end() { FileStreamBase::seek(0,ORG_END); }
    virtual streampos gcount() const { return FileStreamBase::gcount(); }
    virtual bool fail() const { return FileStreamBase::fail(); }
    virtual int peek() { return FileStreamBase::peek(); }
    virtual void unget() { return FileStreamBase::unget(); }
    OpenMode mode() const { return FileStreamBase::mode(); }
    bool canWrite() const { return false; }
    int err_no() const { return FileStreamBase::err_no(); }
    void error( std::ostream& os ) const { return FileStreamBase::error(os); }
#if defined( _WIN32 )
    struct _stat64 stat() const { return FileStreamBase::stat(); }
#elif defined(__USE_LARGEFILE64)
    struct stat64 stat() const { return FileStreamBase::stat(); }
#else
    struct stat stat() const { return FileStreamBase::stat(); }
#endif
  };
  /// @ingroup FileStreams
  class FileOStream
    : virtual public FileStreamBase
    , public OStream<huge_streampos>
  {
    friend class FileStream;
    IMPLEMENT_OSTREAM_OPERATORS(FileOStream,huge_streampos);
  public:
    typedef huge_streampos streampos;
    /// @brief Constructor
    FileOStream() { }
    /// @brief Constructor opening a file
    FileOStream( const std::string& rcFilename, bool bAppend=false )
    {
      FileStreamBase::open(rcFilename,bAppend?OM_APPEND:OM_WRITEONLY,_S_IWRITE);
    }
    /// @brief Destructor
    ~FileOStream() { if( is_open() ) close(); }
    void open( const std::string& rcFilename )
    {
      FileStreamBase::open(rcFilename,OM_WRITEONLY,_S_IWRITE);
    }
    virtual bool is_open() const { return FileStreamBase::is_open(); }
    virtual void write( const char* pch, size_t size ) { FileStreamBase::write(pch,size); }
    /// @brief Returns the current read position.
    virtual streampos tellp() const { return FileStreamBase::tell(); }
    /// @brief Sets the current read position.
    virtual void seekp(const streampos& p) { FileStreamBase::seek(p,ORG_BEG); }
    virtual void seekp2end() { FileStreamBase::seek(0,ORG_END); }
    virtual void flush() { FileStreamBase::flush(); }
    OpenMode mode() const { return FileStreamBase::mode(); }
    bool canWrite() const { return true; }
    int err_no() const { return FileStreamBase::err_no(); }
    void error( std::ostream& os ) const { return FileStreamBase::error(os); }
#if defined( _WIN32 )
    struct _stat64 stat() const { return FileStreamBase::stat(); }
#elif defined(__USE_LARGEFILE64)
    struct stat64 stat() const { return FileStreamBase::stat(); }
#else
    struct stat stat() const { return FileStreamBase::stat(); }
#endif
  };
  /// @ingroup FileStreams
  class FileStream
    : public FileIStream
    , public FileOStream
  {
    IMPLEMENT_ISTREAM_OPERATORS(FileStream,huge_streampos);
    IMPLEMENT_OSTREAM_OPERATORS(FileStream,huge_streampos);
  public:
    typedef huge_streampos streampos;
    /// @brief Constructor
    FileStream() { }
    FileStream( const FileStream& src ) : FileStreamBase(src) { }
    /// @brief Constructor opening a file
    FileStream( const std::string& rcFilename ) { open(rcFilename); }
    /// @brief Destructor
    void open( const std::string& rcFilename, OpenMode eOpenMode=OM_READWRITE, int iPermMode=_S_IWRITE|_S_IREAD )
    {
      FileStreamBase::open(rcFilename,eOpenMode,iPermMode);
    }
    virtual bool is_open() const { return FileStreamBase::is_open(); }
    virtual void close() { FileStreamBase::close(); }
    virtual void read( char* pch, size_t size ) { FileStreamBase::read(pch,size); }
    virtual streampos tellg() const { return FileIStream::tellg(); }
    virtual void seekg(const streampos& g) { FileIStream::seekg(g); }
    virtual streampos tellp() const { return FileOStream::tellp(); }
    virtual void seekp(const streampos& g) { FileOStream::seekp(g); }
    virtual void seekp2end() { FileOStream::seekp2end(); }
    virtual streampos gcount() const { return FileStreamBase::gcount(); }
    virtual bool fail() const { return FileStreamBase::fail(); }
    virtual void write( const char* pch, size_t size ) { FileStreamBase::write(pch,size); }
    virtual void flush() { FileStreamBase::flush(); }
    virtual int peek() { return FileStreamBase::peek(); }
    virtual void unget() { return FileStreamBase::unget(); }
    OpenMode mode() const { return FileStreamBase::mode(); }
    bool canWrite() const { return FileStreamBase::canWrite(); }
    int err_no() const { return FileStreamBase::err_no(); }
    void error( std::ostream& os ) const { return FileStreamBase::error(os); }
    std::string error() const { std::stringstream ss; FileStreamBase::error(ss); return ss.str(); }
#if defined( _WIN32 )
    struct _stat64 stat() const { return FileStreamBase::stat(); }
#elif defined(__USE_LARGEFILE64)
    struct stat64 stat() const { return FileStreamBase::stat(); }
#else
    struct stat stat() const { return FileStreamBase::stat(); }
#endif
    operator const FileOStream&() const { return *this; }
    operator const FileIStream&() const { return *this; }
    operator FileOStream&() { return *this; }
    operator FileIStream&() { return *this; }
  };
}
#endif
