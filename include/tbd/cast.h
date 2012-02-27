#ifndef __TBD__CAST_H
#define __TBD__CAST_H

#include "tbd/exception.h"
#include <boost/numeric/conversion/cast.hpp>

namespace tbd
{
  template<class T, class S> T numeric_cast( const S& s )
  {
    try
    {
      return boost::numeric_cast<T>(s);
    }
    catch( boost::bad_numeric_cast& e )
    {
      TBD_THROW(Exception(e));
    }
  }
}

#endif
