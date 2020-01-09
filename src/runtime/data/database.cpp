


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
  BTree<table_t*>::Iterator it_tables = m_tables.iterator();
  while (it_tables.has_next()) 
  {
    table_t* table =  *it_tables.next().p_value;
    table_destroy(table);
    delete table;
  }
  m_tables.clear();

  BTree<BitTable*>::Iterator it_tags = m_tags.iterator();
  while (it_tags.has_next()) 
  {
    delete *it_tags.next().p_value;
  }
  m_tags.clear();

  BTree<table_t*>::Iterator it_references = m_references.iterator();
  while (it_references.has_next()) 
  {
    table_t* table =  *it_references.next().p_value;
    table_destroy(table);
    delete table;
  }
  m_references.clear();

  BTree<table_t*>::Iterator it_temp_tables = m_temp_tables.iterator();
  while (it_temp_tables.has_next()) 
  {
    table_t* table = *it_temp_tables.next().p_value;
    table_destroy(table);
    delete table;
  }
  m_temp_tables.clear();

  BTree<GlobalInfo>::Iterator it_globals = m_globals.iterator();
  while (it_globals.has_next()) 
  {
    BTree<GlobalInfo>::Entry entry = it_globals.next();
    entry.p_value->m_destructor(entry.p_value->p_global);
    delete [] ((char*)entry.p_value->p_global);
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
  BTree<table_t*>::Iterator it = m_tables.iterator();
  while(it.has_next()) 
  {
    table_t* table = *it.next().p_value;
    table_dealloc_component(table, id);
  }
  release();
}

void Database::tag_entity(entity_id_t entity_id, 
                          const char* tag) 
{
  uint32_t hash_value = hash(tag);
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
                            const char* tag) 
{
  lock();
  uint32_t hash_value = hash(tag);
  BitTable** bit_table = m_tags.get(hash_value);
  if(bit_table != nullptr) 
  {
    (*bit_table)->remove(entity_id);
  }
  release();
}

BitTable* 
Database::get_tagged_entities(const char* tag) 
{
  uint32_t hash_value = hash(tag);
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
Database::find_or_create_ref_table(const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  lock();
  table_t* table_ptr = nullptr;
  table_t** ref_table = m_references.get(hash_value);
  if (ref_table == nullptr) 
  {
    table_ptr = new table_t;
    *table_ptr = table_create(ref_name, 
                              hash_value, 
                              sizeof(uint32_t), 
                              destructor<uint32_t>);
    m_references.insert_copy( hash_value, &table_ptr);
  } 
  else
  {
    table_ptr = *ref_table;
  }
  release();
  return TableView<uint32_t>(table_ptr);
}

TableView<uint32_t>
Database::find_ref_table(const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  lock();
  table_t* table_ptr = nullptr;
  table_t** ref_table = m_references.get(hash_value);
  if (ref_table != nullptr) 
  {
    table_ptr = *ref_table;
  }
  release();
  return TableView<uint32_t>(table_ptr);
}

TableView<uint32_t>
Database::create_ref_table(const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  lock();
  table_t* table_ptr = nullptr;
  table_t** ref_table = m_references.get(hash_value);
  if (ref_table == nullptr) 
  {
    table_ptr = new table_t();
    *table_ptr = table_create(ref_name, 
                              hash_value, 
                              sizeof(uint32_t), 
                              destructor<uint32_t>);
    
    m_references.insert_copy( hash_value, &table_ptr);
  } 
  release();
  return TableView<uint32_t>(table_ptr);
}

void 
Database::add_reference( const char* ref_name, 
                         entity_id_t tail, 
                         entity_id_t head) 
{
  uint32_t hash_value = hash(ref_name);
  lock();
  table_t* table_ptr = nullptr;
  table_t** ref_table = m_references.get(hash_value);
  if (ref_table == nullptr) 
  {
    table_ptr = new table_t();
    *table_ptr = table_create(ref_name, hash_value, sizeof(uint32_t), destructor<uint32_t>);
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
Database::remove_reference( const char* ref_name, 
                            entity_id_t tail) 
{
  uint32_t hash_value = hash(ref_name);
  lock();
  table_t** ref_table = m_references.get(hash_value);
  if (ref_table != nullptr) 
  {
    table_t* table_ptr = *ref_table;
    table_dealloc_component(table_ptr, tail);
  }
  release();
  return;
}

void
Database::start_webserver(const char* address, 
                          const char* port)
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
  BTree<table_t*>::Iterator it = m_tables.iterator();
  uint32_t count = 0; 
  while(it.has_next() && count < capacity)
  {
    table_t* table = *it.next().p_value;
    strncpy(&data[count].m_name[0], table->p_name, _FURIOUS_TABLE_INFO_MAX_NAME_LENGTH);
    data[count].m_size = table_size(table);
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

uint32_t
Database::get_table_id(const char* table_name)
{
  return hash(table_name);
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
  BTree<table_t*>::Iterator it_temp_tables = m_temp_tables.iterator();
  while (it_temp_tables.has_next()) 
  {
    table_t* table = *it_temp_tables.next().p_value;
    table_destroy(table);
    delete table; 
  }
  m_temp_tables.clear();
}


} /* furious */ 
