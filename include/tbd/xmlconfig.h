#ifndef _XMLCONFIG_H__E43399A8_CF59_4184_984E_FAB9FC0DF52B__INCLUDED_
#define _XMLCONFIG_H__E43399A8_CF59_4184_984E_FAB9FC0DF52B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <fstream>
#include "tbd/xmlstream.h"

namespace tbd
{
  class XmlConfigException
    : public Exception
  {
  public:
    enum ErrCode 
    { 
      NotTested = -2,
      NotImplemented = -1,
      Ok = 0,
      CantReadFile,
      XmlSyntaxError,
      XmlStructureError,
      WrongFileFormat,
      ConfigClassExpected,
      WrongConfigClass,
      CantWriteToFile,
      OtherError,
      /// for derives to continue error list
      LastError
    };
    /// @brief Create a XML config exception
    /// @param eErrCode Identifier for what has happened exception
    /// @param strFileName source file name
    explicit XmlConfigException(ErrCode eErrCode, const std::string& strFileName ) 
      : m_eErrCode(eErrCode) 
      , m_strFileName(strFileName)
    {}
    virtual ~XmlConfigException() throw() {}
    ErrCode getErrCode() const { return m_eErrCode; }
    const std::string& getFileName() { return m_strFileName; }
    virtual void explain(std::stringstream& ss) const 
    { 
      switch( m_eErrCode )
      {
      case Ok:                  break;
      case NotTested:           ss << "code not tested (please contact manufacturer!)"; break;
      case NotImplemented:      ss << "code not implemented (please contact manufacturer!)"; break;
      case CantReadFile:        ss << "cannot open config file for reading"; break;
      case XmlSyntaxError:      ss << "XML syntax error"; break;
      case XmlStructureError:   ss << "XML structure error (e.g. missing tags)"; break;
      case WrongFileFormat:     ss << "wrong file format (XML expected)"; break;
      case ConfigClassExpected: ss << "expecting XML tag that defines config class"; break;
      case WrongConfigClass:    ss << "bad config class"; break;
      case CantWriteToFile:     ss << "cannot write to file"; break;
      case OtherError:          ss << "(other error)"; break;
      default:                  ss << "(unkown error)";
      }
    }
  private:
    ErrCode     m_eErrCode;
    std::string m_strFileName;
  };

  /// base class for XML configurations
  class XmlConfig
  {
  public:
    /// Constructor
    /// @param pszClassName Name of the class of this config. This name must
    ///        match the one in the config file to read!
    XmlConfig( const char *pszClassName )
      : m_strClassName(pszClassName)
    {
    }
    /// Destructor
    virtual ~XmlConfig()
    {
    }
    /// read a configuration file
    void readFile()
    {
      std::string strErrorMessage;
      readFile(m_strFileName,strErrorMessage);
    }
    /// read a configuration file and catch any known exception into a error code
    /// and error message
    /// @param rstrErrorMessage returned error message (if any)
    XmlConfigException::ErrCode readFile( std::string& rstrErrorMessage )
    {
      return readFile(m_strFileName,rstrErrorMessage);
    }
    XmlConfigException::ErrCode readFile( const std::string& strFileName, std::string& rstrErrorMessage )
    {
      std::stringstream ss;
      XmlConfigException::ErrCode err=XmlConfigException::Ok;
      {
        try
        {
          readFile( strFileName );
        }
        catch (XmlConfigException& e)
        {
          ss << "while trying to read file '" << e.getFileName() << "': " << e.what();
          // return error
          err=e.getErrCode();
        }
        // catch XML and exceptions
        catch (XmlParseException& e)
        {
          ss << "while reading file '" << strFileName << "': " << e.what();
          // return error
          err=XmlConfigException::XmlSyntaxError;
        }
        catch (DomException& e)
        {
          ss << "while parsing file '" << strFileName << "': " << e.what();
          err=XmlConfigException::XmlStructureError;
        }
      }
      rstrErrorMessage = ss.str();
      return err;
    }
    /// read a config file and DON'T catch any exception
    /// @param strFileName file path and name of the file to read.
    void readFile( const std::string& strFileName )
    {
      // set the path to the path to the file ;)
      setFileName(strFileName);
      // open file
      std::ifstream  ifs(strFileName.c_str());
      // if it isn't open
      if( !ifs.is_open() )
        // return error
        throw XmlConfigException(XmlConfigException::CantReadFile,m_strFileName);
      // read xml file to DOM
      DomIStream dis;
      xml::read(ifs,dis);
      // check if there is a config node at root
      if( !dis.exists("config") )
        // if not, return error
        throw XmlConfigException(XmlConfigException::WrongFileFormat,m_strFileName);
      // open the config node
      dis >> domopen("config");
      {
        // check if there is a class specified
        if( !dis.exists("class") )
          // if not, return error
          throw XmlConfigException(XmlConfigException::ConfigClassExpected,m_strFileName);
        // read class name
        std::string strClass;
        {
          dis >> domattr("class") >> strClass;
          // compare with class name of this instance
          if( strClass != m_strClassName )
            // return error, if it doesn't match
            throw XmlConfigException(XmlConfigException::WrongConfigClass,m_strFileName);
        }
        // read specific configuration
        read(dis);
      }
      dis >> domclose();
    }
    void writeFile()
    {
      std::string strErrorMessage;
      writeFile(m_strFileName,strErrorMessage);
    }
    XmlConfigException::ErrCode writeFile( std::string& rstrErrorMessage )
    {
      return writeFile(m_strFileName,rstrErrorMessage);
    }
    XmlConfigException::ErrCode writeFile( const std::string& strFileName, std::string& rstrErrorMessage )
    {
      std::stringstream ss;
      XmlConfigException::ErrCode err=XmlConfigException::Ok;
      {
        try
        {
          writeFile(strFileName);
        }
        catch (XmlConfigException& e)
        {
          ss << "XmlParseException: ErrCode=#" << e.getErrCode() << " in file '" << e.getFileName() << "'";
          // return error
          err=e.getErrCode();
        }
      }
      rstrErrorMessage = ss.str();
      return err;
    }
    void writeFile( const std::string& strFileName )
    {
      // set the path to the path to the file ;)
      setFileName(strFileName);
      // open file
      std::ofstream  ofs(strFileName.c_str());
      // if it isn't open
      if( !ofs.is_open() )
        // return error
        throw XmlConfigException(XmlConfigException::CantWriteToFile,m_strFileName);
      // read xml file to DOM
      DomOStream dos;
      // create the config node
      dos << domopen("config");
      {
        dos << domattr("class") << m_strClassName;
        // read specific configuration
        write(dos);
      }
      dos << domclose();
      xml::write(ofs,dos);
    }

    std::string getPath( std::string strFileNameOrPath ) const
    {
      if( strFileNameOrPath.size() >= 2 && (strFileNameOrPath[0] == '\\' || strFileNameOrPath[0] == '/' || strFileNameOrPath[1] == ':') )
        return strFileNameOrPath;
      else
        return m_strPath + strFileNameOrPath;
    }
    void setFileName(const std::string& strFileName)
    {
      m_strFileName = strFileName;
      int nPosSlash=-1;
      for( int nPos=0; strFileName[nPos] != '\0'; nPos++ )
      {
        if( strFileName[nPos] == '/' || strFileName[nPos] == '\\' )
          nPosSlash = nPos;
      }
      if( nPosSlash!=-1 )
        m_strPath = strFileName.substr(0,nPosSlash+1);
      else
        m_strPath = "";
    }
    const std::string& getFileName() const
    {
      return m_strFileName;
    }
    // helpers
  public:
    /// this method splits IP and port by a separator
    /// @param strIpAndPort IP and port in one string (e.g. "123.123.123.123:123")
    /// @param strIp (OUT) gets the IP only
    /// @param usPort (OUT) gets the port only
    /// @param chSeparator separator character of IP and port
    static bool splitIpPort(const std::string& strIpAndPort, std::string& strIp, unsigned short& usPort, char chSeparator=':' )
    {
      std::string::size_type pos=strIpAndPort.find(chSeparator);
      if( std::string::npos == pos )
        return true;
      strIp = strIpAndPort.substr(0,pos);
      usPort = (unsigned short)atoi(strIpAndPort.substr(pos+1).c_str());
      return false;
    }
    static std::string joinIpPort(const std::string& strIp, unsigned short usPort, char chSeparator=':' )
    {
      std::stringstream s;
      s << strIp << chSeparator << usPort;
      return s.str();
    }
    static std::string makeHex( unsigned int uiValue, const std::string& strPrefix="0x" )
    {
      static const char* sHexTable[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
      std::stringstream s;
      s << strPrefix;
      for( int i=sizeof(uiValue); i>0; i-- )
      {
        unsigned char ch = (unsigned char)(uiValue >> ((i-1)*8));
        s << sHexTable[(ch & 0xF0) >> 4] << sHexTable[ch & 0xF];
      }
      return s.str();
    }

    template<class K,class V> static void readMap( DomIStream& dis, std::map<K,V>& m, const char* pszKeyName, const char* pszValName, const char* pszAttrName=NULL )
    {
      for( DomIStream::iterator it=dis.begin(); it!=dis.end(); it++ )
      {
        K k;
        V v;
        dis >> domopen(pszValName,it) >> domattr(pszKeyName) >> k;
        if( NULL != pszAttrName )
          dis >> domattr(pszAttrName) >> v;
        else
          dis >> v;
        dis >> domclose();
        m.insert(std::pair<K,V>(k,v));
      }
    }
    template<class K,class V> static void writeMap( DomOStream& dos, const std::map<K,V>& m, const char* pszKeyName, const char* pszValName, const char* pszAttrName=NULL )
    {
      for( typename std::map<K,V>::const_iterator it=m.begin(); it!=m.end(); it++ )
      {
        dos << domopen(pszValName) << domattr(pszKeyName) << it->first;
        if( NULL != pszAttrName )
          dos << domattr(pszAttrName) << it->second;
        else
          dos << it->second;
        dos << domclose();
      }
    }
    template<class V> static void readVector( DomIStream& dis, std::vector<V>& vec, char* pszValName, const char* pszAttrName=NULL )
    {
      for( DomIStream::iterator it=dis.begin(); it!=dis.end(); it++ )
      {
        V v;
        dis >> domopen(pszValName,it);
        if( pszAttrName != NULL )
          dis >> domattr(pszAttrName) >> v;
        else
          dis >> v;
        dis >> domclose();
        vec.push_back(v);
      }
    }
    template<class V> static void writeVector( DomOStream& dos, const std::vector<V>& vec, const char* pszValName, const char* pszAttrName=NULL )
    {
      for( typename std::vector<V>::const_iterator it=vec.begin(); it!=vec.end(); it++ )
      {
        dos << domopen(pszValName);
        if( pszAttrName != NULL )
          dos << domattr(pszAttrName)<< *it;
        else
          dos << *it;
        dos << domclose();
      }
    }

    // overrides
  protected:
    /// override for specific configuration to read
    /// @param dis XML stream to read your configuration from.
    virtual void read( DomIStream& dis ) = 0;
    virtual void write( DomOStream& dos ) const = 0;
  private:
    /// name of the config class - provided by the derived classes through 
    /// the constructor 
    std::string m_strClassName;
    std::string m_strFileName;
    std::string m_strPath;
  };
}

#endif // #ifndef __TBD__XMLCONFIG_H
