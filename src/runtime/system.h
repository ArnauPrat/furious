

#ifndef _FURIOus_SYSTEM_H
#define _FURIOus_SYSTEM_H value

#include "../data/table.h"

#include <vector>
#include <memory>

using SystemId = uint32_t;

namespace furious {

enum class ComAccessType : uint8_t {
  E_READ,
  E_WRITE
};

struct SysComDescriptor {
  std::string   m_name;
  ComAccessType m_access_type;
};

class System {
public:

  System() = default;
  virtual ~System() = default;

  /**
   * @brief Applies the system over the set of blocks of components
   *
   * @param components The set of blocks with the components 
   */
  virtual void apply_block( const std::vector<void*>& components_blocks ) = 0;

  /**
   * @brief Applies the system over a set of components
   *
   * @param components The components to apply the system to 
   */
  virtual void apply( const std::vector<void*>& components ) = 0;

  
  /**
   * @brief Gets the names of the components this system is for
   *
   * @return A vector with the names of the components this system is for
   */
  virtual std::vector<SysComDescriptor> components() const = 0;

};

} /* furious */ 



#endif
