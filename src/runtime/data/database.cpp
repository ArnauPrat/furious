


#include "../../common/utils.h"
#include "bit_table.h"
#include "database.h"
#include "webserver/webserver.h"

namespace furious 
{

Database::Database() : 
m_next_entity_id(0),
p_webserver(nullptr)
{
}

Database::~Database() 
{
  stop_webserver();
  clear();
}

void Database::clear() 
{
  lock();
  BTree<Table*>::Iterator it_tables = m_tables.iterator();
  while (it_tables.has_next()) 
  {
    delete *it_tables.next().p_value;
  }
  m_tables.clear();

  BTree<BitTable*>::Iterator it_tags = m_tags.iterator();
  while (it_tags.has_next()) 
  {
    delete *it_tags.next().p_value;
  }
  m_tags.clear();

  BTree<Table*>::Iterator it_references = m_references.iterator();
  while (it_references.has_next()) 
  {
    delete *it_references.next().p_value;
  }
  m_references.clear();

  BTree<Table*>::Iterator it_temp_tables = m_temp_tables.iterator();
  while (it_temp_tables.has_next()) 
  {
    delete *it_temp_tables.next().p_value;
  }
  m_temp_tables.clear();

  BTree<GlobalInfo>::Iterator it_globals = m_globals.iterator();
  while (it_globals.has_next()) 
  {
    BTree<GlobalInfo>::Entry entry = it_globals.next();
    entry.p_value->m_destructor(entry.p_value->p_global);
  }
  m_globals.clear();
  release();
}

entity_id_t 
Database::get_next_entity_id() 
{
  lock();
  entity_id_t next_id = m_next_entity_id;
  m_next_entity_id++;
  release();
  return next_id;
}

void 
Database::clear_entity(entity_id_t id) 
{
  lock();
  BTree<Table*>::Iterator it = m_tables.iterator();
  while(it.has_next()) 
  {
    Table* table = *it.next().p_value;
    table->dealloc_and_destroy_component(id);
  }
  release();
}

void Database::tag_entity(entity_id_t entity_id, 
                          const std::string& tag) 
{
  uint32_t hash_value = hash(tag.c_str());
  lock();
  BitTable** bit_table = m_tags.get(hash_value);
  BitTable* bit_table_ptr = nullptr;
  if(bit_table == nullptr) 
  {
    bit_table_ptr = new BitTable();
    m_tags.insert_copy(hash_value, &bit_table_ptr);
  }
  else
  {
    bit_table_ptr = *bit_table;
  }
  release();
  bit_table_ptr->add(entity_id);
}

void Database::untag_entity(entity_id_t entity_id, 
                          const std::string& tag) 
{
  lock();
  uint32_t hash_value = hash(tag.c_str());
  BitTable** bit_table = m_tags.get(hash_value);
  if(bit_table != nullptr) 
  {
    (*bit_table)->remove(entity_id);
  }
  release();
}

BitTable* 
Database::get_tagged_entities(const std::string& tag) 
{
  uint32_t hash_value = hash(tag.c_str());
  lock();
  BitTable** bit_table = m_tags.get(hash_value);
  if(bit_table != nullptr) 
  {
    release();
    return *bit_table;
  }
  BitTable* bit_table_ptr = new BitTable();
  m_tags.insert_copy(hash_value, &bit_table_ptr);
  release();
  return bit_table_ptr;
}

TableView<uint32_t>
Database::get_references(const std::string& ref_name)
{
  uint32_t hash_value = hash(ref_name.c_str());
  lock();
  Table* table_ptr = nullptr;
  Table** ref_table = m_references.get(hash_value);
  if (ref_table == nullptr) 
  {
    table_ptr = new Table(ref_name, hash_value, sizeof(uint32_t), &destructor<uint32_t>);
    m_references.insert_copy( hash_value, &table_ptr);
  } 
  else
  {
    table_ptr = *ref_table;
  }
  release();
  return TableView<uint32_t>(table_ptr);
}

void 
Database::add_reference( const std::string& ref_name, 
                         entity_id_t tail, 
                         entity_id_t head) 
{
  uint32_t hash_value = hash(ref_name.c_str());
  lock();
  Table* table_ptr = nullptr;
  Table** ref_table = m_references.get(hash_value);
  if (ref_table == nullptr) 
  {
    table_ptr = new Table(ref_name, hash_value, sizeof(uint32_t), &destructor<uint32_t>);
    m_references.insert_copy( hash_value, &table_ptr);
  } 
  else
  {
    table_ptr = *ref_table;
  }
  TableView<uint32_t> t_view(table_ptr);
  t_view.insert_component(tail, head);
  release();
  return;
}

void 
Database::remove_reference( const std::string& ref_name, 
                            entity_id_t tail) 
{
  uint32_t hash_value = hash(ref_name.c_str());
  lock();
  Table** ref_table = m_references.get(hash_value);
  if (ref_table != nullptr) 
  {
    Table* table_ptr = *ref_table;
    table_ptr->dealloc_and_destroy_component(tail);
  }
  release();
  return;
}

void
Database::start_webserver(const std::string& address, 
                          const std::string& port)
{
  p_webserver = new WebServer();
  p_webserver->start(this,
                     address,
                     port);
}

void
Database::stop_webserver()
{
  if(p_webserver != nullptr)
  {
    p_webserver->stop();
    delete p_webserver;
    p_webserver = nullptr;
  }
}

size_t
Database::num_tables() const
{
  lock();
  size_t size = m_tables.size();
  release();
  return size;
}

size_t 
Database::meta_data(TableInfo* data, uint32_t capacity)
{
  lock();
  BTree<Table*>::Iterator it = m_tables.iterator();
  uint32_t count = 0; 
  while(it.has_next() && count < capacity)
  {
    Table* table = *it.next().p_value;
    strncpy(&data[count].m_name[0], table->name().c_str(), _FURIOUS_TABLE_INFO_MAX_NAME_LENGTH);
    data[count].m_size = table->size();
    count++;
  }
  release();
  return count;
}

void
Database::lock() const
{
  m_mutex.lock();
}

void
Database::release() const
{
  m_mutex.unlock();
}

void
Database::remove_temp_tables()
{
  lock();
  remove_temp_tables_no_lock();
  release();
}

void
Database::remove_temp_tables_no_lock()
{
  BTree<Table*>::Iterator it_temp_tables = m_temp_tables.iterator();
  while (it_temp_tables.has_next()) 
  {
    delete *it_temp_tables.next().p_value;
  }
  m_temp_tables.clear();
}

  
} /* furious */ 
