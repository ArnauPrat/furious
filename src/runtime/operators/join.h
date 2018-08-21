
#ifndef _FURIOUS_JOIN_H_
#define _FURIOUS_JOIN_H_ value

#include <map>
#include <bitset>

namespace furious {

template<typename...TTables>
std::map<int32_t, std::bitset<TABLE_BLOCK_SIZE>> 
join(TTables* tables...) {

}
  
} /* furious */ 

#endif /* ifndef _FURIOUS_JOIN_H_ */
