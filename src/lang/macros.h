
#ifndef _FURIOUS_LANG_MACROS_H_
#define _FURIOUS_LANG_MACROS_H_


#define FURIOUS_ENABLE_COMPONENT(context, Component, id)\
  context->enable_component<Component>(#Component, id)

#define FURIOUS_DISABLE_COMPONENT(context, Component, id)\
  context->disable_component<Component>(#Component, id)
    

#endif
