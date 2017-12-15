
#include <utility>

namespace furious {
extern Database* database;
extern Workload* workload;

template<typename TComponent>
  void register_component() {
    assert(database != nullptr);
    database->create_table<TComponent>();
  }

template<typename TComponent>
  void unregister_component() {
    assert(database != nullptr);
    database->remove_table<TComponent>();
  }

template<typename TComponent>
TableView<TComponent> get_table() {
  assert(database != nullptr);
  return database->find_table<TComponent>();
}

template<typename TSystem, typename...TArgs>
  void register_system(TArgs&&...args) {
    assert(database != nullptr);
    assert(workload != nullptr);
    workload->register_system<TSystem>(std::forward<TArgs>(args)...);
  }

template<typename TSystem>
  void unregister_system() {
    assert(database != nullptr);
    assert(workload != nullptr);
    workload->remove_system<TSystem>();
  }


} /* furious */ 
