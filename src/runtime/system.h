

#ifndef _FURIOus_SYSTEM_H
#define _FURIOus_SYSTEM_H value

#include "../data/table.h"
#include "../data/context.h"
#include "../common/reflection.h"

#include <vector>
#include <memory>

using SystemId = uint32_t;

namespace furious {


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
   * @param context The context of the execution
   * @param block_start The starting id of the block
   * @param components The set of blocks with the components 
   */
  virtual void apply_block( Context* context, uint32_t block_start, const std::vector<void*>& components_blocks ) = 0;

  /**
   * @brief Applies the system over a set of components
   *
   * @param context The context of the execution
   * @param id The entity id 
   * @param components The components to apply the system to 
   */
  virtual void apply( Context* context, uint32_t id, const std::vector<void*>& components ) = 0;

  
  /**
   * @brief Gets the names of the components this system is for
   *
   * @return A vector with the names of the components this system is for
   */
  virtual std::vector<SysComDescriptor> components() const = 0;

};

} /* furious */ 



#endif
