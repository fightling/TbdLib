///////////////////////////////////////////////////////////////////////////////
/// @file arguments.h
/// @brief Command line argument parsing
/// @author Patrick Hoffmann
/// @date 05.11.2006
///////////////////////////////////////////////////////////////////////////////

#ifndef __TBD__ARGUMENTS_H
#define __TBD__ARGUMENTS_H

#include "exception.h"
#include "debug_assert.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

/// @defgroup arguments Arguments
/// @brief Command line argument parsing
/// @ingroup Process
/// @par Purpose 
/// Class for parsing command line arguments from argv[] and argc
/// @par Usage
/// @code
/// int main(int argc, char *argv[])
/// {
///   // Instance of Arguments
///   tbd::Arguments arg("MyProgram","This program is made by myself.");
///   {
///     // add first argument
///     arg.addArgument("param1","The first parameter");
///     // add second argument with a default "second"
///     arg.addArgument("param2","The second parameter", "second");
///
///     // add an option that can be activated by using "-o"
///     arg.addOption('o',"Simple option");
///     
///     // add an option that can be activated by using "-x"
///     arg.addOption('x',"Option with parameters");
///     {
///       // add an argument to option "x"
///       arg.addArgument('x',"xparam1", "First parameter of option x");
///       // add another argument to option "x" with default ""
///       arg.addArgument('x',"xparam2", "Second parameter of option x","");
///     }
///   }
///   if( !arg.parse(argc,argv) )
///     exit(-1);
/// 
///   std::cout << "param 1  : " << arg["param1"] << std::endl;
///   std::cout << "param 2  : " << arg["param2"] << std::endl;
///   std::cout << "option -o: " << arg['o'] << std::endl;
///   std::cout << "option -x: " << arg['x'] << std::endl;
///   std::cout << "param 1 of option -x: " << arg['x']["xparam1"] << std::endl;
///   std::cout << "param 1 of option -x: " << arg['x']["xparam2"] << std::endl;
/// }
/// @endcode
/// Called without parameters arg.parse() prints out an error and a short 
/// usage message:
/// @code 
/// MyProgram error: Too few arguments.
/// Usage: MyProgram [-o] [-x xparam1 xparam2] param1 [param2]
/// 
/// Options:
///   -o        Simple option
///   -x        Option with parameters
///   xparam1        = First parameter of option x
///   xparam2        = Second parameter of option x
/// 
/// Arguments:
///   param1  = The first parameter
///   param2  = The second parameter
///   (optional argument, default is 'second')
///
/// This program is made by myself.
/// @endcode
/// Called with "1 2 -o -x 3 4" the programm dumps it's parameters:
/// @code
/// param 1  : 1
/// param 2  : 2
/// option -o: 1
/// option -x: 1
/// param 1 of option -x: 3
/// param 1 of option -x: 4
/// @endcode
namespace tbd
{
  /// @brief Exception thrown by the Arguments class
  /// @ingroup Arguments
  class ArgumentsException
    : public Exception
  {
  public:
    enum ErrCode { Ok, UnknownArgumentName };
    /// @brief Constructor
    /// @param eErrCode Identifier for what has happened exception
    explicit ArgumentsException(ErrCode eErrCode) : m_eErrCode(eErrCode) {}
    ErrCode getErrCode() { return m_eErrCode; }
  private:
    ErrCode   m_eErrCode;
  };

  /// @brief Manages program arguments
  /// @ingroup Arguments
  class Arguments  
  {
  public:
    static std::string& unknown()
    {
      static std::string s_UnknownArgument("<UnKnOwN>");
      return s_UnknownArgument;
    }
    class Option;
    /// @brief One program argument
    /// @ingroup Arguments
    class Argument
    {
      friend class Arguments;
      friend class Option;
      std::string	m_strName;
      std::string	m_strDescription;
      std::string	m_strValue;
      std::string	m_strDefault;
      bool	      m_bOptional;
    public:
      Argument( std::string strName, std::string strDescription="", std::string strDefault="" )
        : m_strName( strName )
        , m_strDescription( strDescription )
        , m_strValue( strDefault )
        , m_strDefault( strDefault )
        , m_bOptional( !strDefault.empty() )
      {

      }
    };
    /// @brief One program option that might contain optional arguments
    /// @ingroup Arguments
    class Option
    {
      friend class Arguments;
      char			            m_chName;
      std::string		        m_strDescription;
      bool			            m_bSet;
      std::vector<Argument> m_aArguments;
    public:
      Option() {};
      ~Option() {};
      Option( char chName, std::string strDescription="" )
        : m_chName( chName )
        , m_strDescription( strDescription )
        , m_bSet( false )
      { }
      void addArgument( std::string strName, std::string strDescription="", std::string strDefault = "" )
      {
        m_aArguments.push_back( Argument( strName, strDescription, strDefault ) );
      }
      const std::string& operator[]( int n ) const
      {
        return m_aArguments[n].m_strValue;
      }
      const std::string& operator[]( const char *pszArgumentName ) const 
      { return operator[]( (std::string)pszArgumentName ); }
      const std::string &operator[]( const std::string& strArgumentName ) const
      {
        for( std::vector<Argument>::const_iterator it=m_aArguments.begin(); it!=m_aArguments.end(); it++ ) 
        {
          if( it->m_strName == strArgumentName )
            return it->m_strValue;
        }
        throw ArgumentsException(ArgumentsException::UnknownArgumentName);
        return Arguments::unknown();
      }
      operator bool() { return m_bSet; }
      void set( bool bSet = true ) { m_bSet = bSet; }
      std::string getName()
      {
        std::string str = " ";
        str[0] = m_chName;
        return str;
      }
    };
  public:
    Arguments( std::string strCommandName, std::string strDescription="", std::string strOptionmarkers="-" )
      : m_strOptionmarkers( strOptionmarkers )
      , m_strDescription( strDescription )
      , m_strCommandName( strCommandName ) 
    { }
    virtual ~Arguments() {};
    bool isOption( char chOptionName )
    {
      std::map<char,Option>::iterator it = m_mapOptions.find(chOptionName);
      return (it == m_mapOptions.end())? false : it->second.m_bSet;
    }
    const std::string& getCommandName() const
    {
      return m_strCommandName;
    }
    void usage( std::ostream& os=std::cout )
    {
      os << "Usage: " << m_strCommandName;
      for( std::map<char,Option>::iterator it = m_mapOptions.begin(); it != m_mapOptions.end(); it++ )
      {
        os << " [" << m_strOptionmarkers[0] << it->second.getName();
        for( std::vector<Argument>::iterator itArg = it->second.m_aArguments.begin(); itArg != it->second.m_aArguments.end(); itArg++ )
        {
          if( itArg->m_bOptional )
            os << " [" << itArg->m_strName << "]";
          else
            os << " " << itArg->m_strName;
        }
        os << "]";
      }
      for( std::vector<Argument>::iterator itArg = m_aArguments.begin(); itArg != m_aArguments.end(); itArg++ )
      {
        if( itArg->m_bOptional )
          os << " [" << itArg->m_strName << "]";
        else
          os << " " << itArg->m_strName;
      }
      os << std::endl;
      if( !m_mapOptions.empty() )
        os << std::endl << "Options:" << std::endl;
      for( std::map<char,Option>::iterator it = m_mapOptions.begin(); it != m_mapOptions.end(); it++ )
      {
        os << "\t-" << it->second.getName() << "\t  " << it->second.m_strDescription << std::endl;
        for( std::vector<Argument>::iterator itArg = it->second.m_aArguments.begin(); itArg != it->second.m_aArguments.end(); itArg++ )
        {
          os << "\t " << itArg->m_strName << "\t= " << itArg->m_strDescription << std::endl;
          if( itArg->m_bOptional )
            os << "\t\t  (optional argument, default is '" << itArg->m_strDefault << "')" << std::endl;
        }
      }
      if( !m_aArguments.empty() )
        os << std::endl << "Arguments:" << std::endl;
      for( std::vector<Argument>::iterator itArg = m_aArguments.begin(); itArg != m_aArguments.end(); itArg++ )
      {
        os << "\t" << itArg->m_strName << "\t= " << itArg->m_strDescription << std::endl;

        if( itArg->m_bOptional )
          os << "\t\t  (optional argument, default is '" << itArg->m_strDefault << "')" << std::endl;
      }
      os << std::endl;
      os << m_strDescription << std::endl;
    }
    void addOption( char chOption, std::string strDescription="" )
    { m_mapOptions.insert( std::pair<char,Option>(chOption,Option(chOption,strDescription)) ); }
    void addOption( Option &option )
    { m_mapOptions.insert( std::pair<char,Option>(option.m_chName,option) ); }
    void addArgument( std::string strName, std::string strDescription="", std::string strDefault="" )
    { m_aArguments.push_back( Argument( strName, strDescription, strDefault ) ); }
    void addArgument( char chOption, std::string strName, std::string strDescription="", std::string strDefault="" )
    {
      std::map<char,Option>::iterator it = m_mapOptions.find(chOption);
      if( it == m_mapOptions.end() )
        BOOST_ASSERT(0);
      else
        it->second.addArgument(strName,strDescription,strDefault);
    }
    bool parse(int argc, char* argv[], std::ostream& os=std::cout)
    {
      if( m_strCommandName.empty() )
        m_strCommandName = argv[0];

      unsigned int nArg = 0;

      for( int i=1; i<argc; i++ )
      {
        std::string strArgument = argv[i];
        // Option...?
        if( m_strOptionmarkers.find(strArgument.substr(0,1)) != std::string::npos )
        {
          char chOptionName = strArgument[1];
          std::map<char,Option>::iterator it = m_mapOptions.find(chOptionName);
          if( it == m_mapOptions.end() )
          {
            os << m_strCommandName << " error: Unknown option " << strArgument << "." << std::endl;
            usage(os);
            return false;
          }
          else
          {
            it->second.m_bSet = true;
            i++;
            { 
              unsigned int nNonOptionalArgs = 0;
              {
                for( std::vector<Argument>::iterator itOptArg = it->second.m_aArguments.begin(); itOptArg != it->second.m_aArguments.end(); itOptArg++ ) 
                {
                  if( !itOptArg->m_bOptional )
                    nNonOptionalArgs++;
                }
              }
              for(unsigned int nOptArg=0; nOptArg < it->second.m_aArguments.size(); i++, nOptArg++ )
              {
                if( i >= argc || m_strOptionmarkers.find(std::string(argv[i]).substr(0,1)) != std::string::npos )
                {
                  if( nOptArg < nNonOptionalArgs )
                  {
                    os << m_strCommandName << " error: Too few arguments for option " << strArgument << "." << std::endl;
                    usage(os);
                    return false;
                  }
                  else
                  {
                    break;
                  }
                }
                it->second.m_aArguments[nOptArg].m_strValue = argv[i];
              }
            }
            i--;
          }
        }
        else	// ...oder Argument
        {
          if( nArg >= m_aArguments.size() )
          {
            os << m_strCommandName << " error: Too much arguments. " << std::endl;
            usage(os);
            return false;
          }
          m_aArguments[nArg++].m_strValue = strArgument;
        }
      }
      {
        unsigned int nNonOptionalArgs = 0;
        {
          for( std::vector<Argument>::iterator it = m_aArguments.begin(); it != m_aArguments.end(); it++ ) 
          {
            if( !it->m_bOptional )
              nNonOptionalArgs++;
          }
        }
        if( nNonOptionalArgs > nArg )
        {
          os << m_strCommandName << " error: Too few arguments." << std::endl;
          usage(os);
          return false;
        }
      }
      return true;
    }
    std::string &operator[]( int n ) { return m_aArguments[n].m_strValue; }
    std::string &operator[]( std::string strArgumentName )
    {
      for( std::vector<Argument>::iterator it = m_aArguments.begin(); it != m_aArguments.end(); it++ ) 
      {
        if( it->m_strName == strArgumentName )
          return it->m_strValue;
      }
      throw ArgumentsException(ArgumentsException::UnknownArgumentName);
      return Arguments::unknown();
    }
    Option &operator[]( char chOptionName )
    {
      static Option  s_MissingOption;
      std::map<char,Option>::iterator it = m_mapOptions.find(chOptionName);
      return (it==m_mapOptions.end())?s_MissingOption:it->second;
    }
  private:
    std::string	          m_strOptionmarkers;
    std::string	          m_strDescription;
    std::string           m_strCommandName;
    std::map<char,Option> m_mapOptions;
    std::vector<Argument> m_aArguments;
  };
}

#endif 
