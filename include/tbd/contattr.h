/*
 * contattr.h
 *
 *  Created on: 22 Sep 2011
 *      Author: pat
 */

#ifndef TBD_CONTATTR_H_
#define TBD_CONTATTR_H_

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <tbd/domstream.h>

namespace tbd
{
  namespace intern
  {
    // imports from boost
    using boost::token_compress_on;
    using boost::split;
    using boost::join;
    using boost::is_any_of;
    using boost::lexical_cast;
    using boost::optional;
    using boost::copy;
    // imports from STL
    using std::vector;
    using std::string;
    using std::inserter;
    using std::back_inserter;
    using boost::adaptors::transformed;
    using boost::sort;

    /// simple comparison functor
    template<class T> struct compare_ftr
    {
      bool operator()(typename T::const_reference i, typename T::const_reference j) const
      {
        return i < j;
      }
    };
    /// lexical cast functor converts from U to T
    template<class T, class U> struct lexical_cast_ftr
    {
      typedef T result_type;
      result_type operator()(const U& u) const
      {
        return lexical_cast<T>(u);
      }
    };

    /** @brief The purpose of the container attribute is to implement DOM attributes which
     *  are based on containers. For example one can have a vector of strings which are joined
     *  to a single string (separated by a white space character) when streaming it into
     *  a DOM stream.
     *  @tparam CONT complete container type (e.g. std::vector<std::string>)
     *  @tparam LEX_PARSE conversion functor from string into CONT::value_type. The default
     *          is a functor which leads to lexical_cast
     *  @tparam LEX_GENERATE conversion functor from CONT::value_type into string. The default
     *          is a functor which leads to lexical_cast
     *  @tparam COMPARE comparison functor which is used to sort the items when generating
     *          the string
     */
    template<
      class CONT,
      char SEP = ' ',
      typename LEX_PARSE = lexical_cast_ftr<typename CONT::value_type, string>,
      typename LEX_GENERATE = lexical_cast_ftr<string, typename CONT::value_type>,
      typename COMPARE = compare_ftr<CONT>
    >
    class ContAttr: public CONT
    {
    public:
      /// substitute the container's value type
      typedef typename CONT::value_type ValueT;
      typedef CONT                      ContainerT;
      static const char                 sep_ = SEP;
      typedef LEX_PARSE                 LexParseF;
      typedef LEX_GENERATE              LexGenerateF;
      typedef COMPARE                   CompareF;
      ContAttr()
      {
      }
      /** @brief copy constructor
       *  @param src source instance
       */
      ContAttr(const ContainerT& src)
          : ContainerT(src)
      {
      }
      /** @brief parse from string list
       *  @param src source string
       */
      ContAttr(const string& src)
      {
        parse(src);
      }
      /** @brief generic output stream operator
       *  @details joins the contained items (using SEP as separator) to a string
       *           and put it into an output stream
       *  @tparam OS type of the output stream
       *  @param os the output stream
       *  @param ca the ContAttr instance
       *  @return the output stream
       */
      template<class OS> friend OS& operator<<(OS& os, const ContAttr& ca)
      {
        return os << ca.generate();
      }
      /** @brief read a string
       *  @details separates the string into items (using SEP as separator)
       *           and overwrite the content of ca with these values
       *  @tparam IS type of the input stream
       *  @param is the input stream
       *  @param ca the ContAttr instance to overwrite
       *  @return the input stream
       */
      template<class IS> friend IS& operator>>(IS& is, ContAttr& ca)
      {
        // read a string
        string str;
        is >> str;
        // clear container
        ca.CONT::clear();
        // parse the string into ca
        ca.parse(str);
        return is;
      }
      /** @brief input stream operator for optional container attribute
       *  @details if ca is valid this method works like the generic output stream
       *           operator. Otherwise it does nothing. If ca is valid but the
       *           container is empty the method will act like ca is invalid.
       *  @tparam IS type of the input stream
       *  @param os the output stream
       *  @param oca the ContAttr instance
       *  @return the output stream
       */
      template<class OS> friend OS& operator<<(OS& os, const optional<ContAttr>& oca)
      {
        // is ca present and not empty?
        if (oca && !oca->empty())
          // generate string from container and write it to the output
          return os << oca->generate();
        else
          // keep calm
          return os;
      }
      /** @brief output DOM stream operator for optional container attributes
       *  @details specialized output operator for DomOStream which removes the
       *           current DOM node when ca is not present or empty
       *  @param os the output DOM stream
       *  @param oca the optional ContAttr instance
       *  @return the output DOM stream
       */
      friend DomOStream& operator<<(DomOStream& os, const optional<ContAttr>& oca)
      {
        // is oca present and not empty?
        if (oca && !oca->empty())
          // generate string from container and write it to the output
          return os << oca->generate();
        else
          // cancel (and remove) current node
          return os << domcancel();
      }
      /** @brief input DOM stream operator for optional container attributes
       *  @details specialized input operator for DomIStream which cancels
       *           the current DOM node if it is missing. If the node value
       *           is present but empty the optional ca will get absent
       *  @param is the output DOM stream
       *  @param oca the optional ContAttr instance
       *  @return the input DOM stream
       */
      friend DomIStream& operator>>(DomIStream& is, optional<ContAttr>& oca)
      {
        // if current node is missing in the DOM
        if (is.missing())
        {
          // cancel current node
          is >> domcancel();
          // reset oca to be absent
          oca.reset();
        }
        else
        {
          // read string from input
          string str;
          is >> str;
          // if string is empty
          if (str.empty())
            // reset oca to be absent
            oca.reset();
          else
          {
            // initialize oca with an empty value
            oca.reset(ContAttr());
            // parse the string into oca
            oca->parse(str);
          }
        }
        return is;
      }
      /// @brief cast to string
      operator string() const
      {
        return generate();
      }
      /** @brief return a container appended by a single value
       *  @param left container
       *  @param right value to append
       *  @return resulting container
       */
      friend ContAttr operator|(const ContAttr& left, const ValueT& right)
      {
        ContAttr result = left;
        inserter(result, result.ContainerT::end()) = right;
        return result;
      }
      /** @brief return the merge result of two containers
       *  @param left first container
       *  @param right second container
       *  @return resulting container
       */
      friend ContAttr operator|(const ContAttr& left, const ContAttr& right)
      {
        ContAttr result = left;
        copy(right,inserter(result, result.ContainerT::end()));
        return result;
      }
      /** @brief generate a string from the container content
       *  @details The container content will be sorted using the COMPARE functor
       *           and then each container value gets converted into a string by
       *           using the LEX_GENERATE functor. The resulting vector will be
       *           joined to a single string separated by SEP.
       *  @return generated string
       */
      string generate() const
      {
        static const char psz[] = { sep_, '\0' };
        vector<ValueT> v;
        copy(*this,back_inserter(v));
        return join(sort(v,CompareF()) | transformed(LexGenerateF()), psz);
      }
      /** @brief parses items separated with SEP from a string
       *  @details of course the items will be stored in the container
       *  @param str input string
       */
      void parse(const string& str)
      {
        static const char psz[] = { sep_, ' ', '\0' };
        vector<string> s;
        split(s, str, is_any_of(psz), token_compress_on);
        boost::copy(s | transformed(LexParseF()),inserter(*this, ContainerT::end()));
      }
      /** @brief set container from external container
       *  @param src source container
       *  @return lvalue
       */
      const ContAttr& operator=(const ContainerT& src)
      {
        ContainerT::clear();
        copy(src.begin(), src.end(), inserter(*this, ContainerT::end()));
        return *this;
      }
      /** @brief set container from external instance
       *  @param src source instance
       *  @return lvalue
       */
      const ContAttr& operator=(const ContAttr& src)
      {
        ContainerT::operator=(src);
        return *this;
      }
      /** @brief inserts a new element to the container
       *  @details inserts to the end of the container
       *  @param v value
       *  @return l-value
       */
      const ContAttr& operator+=(const ValueT& v)
      {
        inserter(*this, ContainerT::end()) = v;
        return *this;
      }
      const ContAttr& operator+=(const ContAttr& src)
      {
        copy(src,inserter(*this, ContainerT::end()));
        return *this;
      }
    };
  }
  /// substitute from internal namespace
  using intern::ContAttr;
}

#endif
