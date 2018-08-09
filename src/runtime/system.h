

#ifndef _FURIOus_SYSTEM_H
#define _FURIOus_SYSTEM_H value

#include "../data/table.h"
#include "../common/traits.h"

#include <vector>
#include <memory>

using SystemId = int32_t;

namespace furious {

class Context;

struct SysComDescriptor {
  std::string   m_name;
  ComAccessType m_access_type;
  bool          m_is_reference;
  const char*   m_ref_name;
};

class System {
public:

  System(int32_t id);
  virtual ~System() = default;

  /**
   * @brief Applies the system over the set of blocks of components
   *
   * @param context The context of the execution
   * @param block_start The starting id of the block
   * @param components The set of blocks with the components 
   */
  virtual void apply_block( Context* context, const std::vector<TBlock*>& components_blocks ) = 0;

  /**
   * @brief Applies the system over a set of components
   *
   * @param context The context of the execution
   * @param id The entity id 
   * @param components The components to apply the system to 
   */
  virtual void apply( Context* context, int32_t id, const std::vector<void*>& components ) = 0;

  
  /**
   * @brief Gets the names of the components this system is for
   *
   * @return A vector with the names of the components this system is for
   */
  virtual std::vector<SysComDescriptor> components() const = 0;

  /**
   * @brief Gets the name of the system
   *
   * @return Returns the name of the system
   */
  virtual std::string name() const = 0;

  /**
   * @brief Gets the id of the system
   *
   * @return  Returns the id of the system
   */
  int32_t get_id();

private:

  int32_t m_id;

};

} /* furious */ 



#endif
