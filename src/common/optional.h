

#ifndef _FURIOUS_OPTIONAL_H_
#define _FURIOUS_OPTIONAL_H_ value

#include<boost/optional.hpp>

namespace furious {

template<typename T>
using optional = boost::optional<T>;
  
} /* furious */ 

#endif /* ifndef _FURIOUS_OPTIONAL_H_ */
