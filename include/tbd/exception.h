///////////////////////////////////////////////////////////////////////////////
/// @file exception.h
/// @brief Exception implementation
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__EXCEPTION_H
#define __TBD__EXCEPTION_H
#ifdef _MSC_VER
# pragma warning(disable:4290)
#endif

/// @defgroup exceptions Exceptions
/// @brief Base class of all exceptions thrown by tbd
/// @ingroup tbd
/// @par Purpose
/// The class Exception builds an easy to use exception base for TBD. It focuses
/// on debugging, parameter tracking and unique error message formatting.
/// @par Usage
/// Just derive your exceptions from tbd::Exception and you will benefit it's
/// features.
/// @code
/// class MyException
///   : public tbd::Exception
/// {
///   std::string m_strReason;
/// public:
///   MyException( const std::string& strReason ) : m_strReason(strReason) {}
///   virtual void explain(std::stringstream& ss) const { ss << m_strReason; }
/// };
/// @endcode
/// @code
/// // External message formatting
/// std::ostream& operator<<(std::ostream& os, const tbd::Exception& e )
/// {
///   if( e.is<MyException>() )
///     os << "Exception '" << e->get<MyException>()->what() << "'catched." << std::endl;
///   return os;
/// }
/// void main()
/// {
///   try
///   {
///     throw MyException("reason");
///   }
///   catch( tbd::Exception& e )
///   {
///     std::cout << e;   // message output: Exception 'reason' catched. delete
///     exit(0);
///   }
/// }
/// @endcode
#include <assert.h>
#include <string>
#include <sstream>
#include <exception>

namespace tbd
{
  struct SourceLine
  {
    SourceLine( const char* pszFile, int nLine ) : m_pszFile(pszFile), m_nLine(nLine) {}
    const char* m_pszFile;
    int         m_nLine;
  };

  /// @todo derive from boost::exception and use standard diagnostic functions
  class Exception
    : public std::exception
  {
    static const size_t MAX_FILENAME_LENGTH=40;
  public:
    Exception() : m_cSourceLine(0,0) {}
    Exception( const std::exception& e ) : std::exception(e), m_cSourceLine(0,0) {}
    virtual ~Exception() throw() {}
    template<class T> const T* get() const
    {
      BOOST_ASSERT( NULL != dynamic_cast<const T*>(this) );
      return dynamic_cast<const T*>(this);
    }
    template<class T> bool is() const
    {
      return NULL != get<T>();
    }
    virtual const char* what() const throw() { std::stringstream ss; explain(ss); static std::string str; str=ss.str(); return str.c_str(); }
    virtual void explain(std::stringstream& ss) const { ss << " in source file " << shortenAtLeft(m_cSourceLine.m_pszFile,MAX_FILENAME_LENGTH) << ":" << m_cSourceLine.m_nLine; }
    std::string shortenAtLeft( const std::string& str, size_t size ) const
    {
      if( str.size() > size )
        return std::string("...")+str.substr(str.size()-size+3);
      else
        return str;
    }
    template<class E> friend E operator+( const tbd::SourceLine& source, const E& e) { E r=e; r.m_cSourceLine = source; return r; }
    void source( const tbd::SourceLine& source) { m_cSourceLine = source; }
  protected:
    SourceLine  m_cSourceLine;
  };
}

#define TBD_THROW(x) throw tbd::SourceLine(__FILE__,__LINE__) + x

#ifdef _MSC_VER
# pragma warning(default:4290)
#endif

#endif
