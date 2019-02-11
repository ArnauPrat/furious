


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

void Database::clear() {

  BTree<Table*>::Iterator it_tables = m_tables.iterator();
  while (it_tables.has_next()) 
  {
    delete *it_tables.next();
  }
  m_tables.clear();

  BTree<BitTable*>::Iterator it_tags = m_tags.iterator();
  while (it_tags.has_next()) 
  {
    delete *it_tags.next();
  }
  m_tags.clear();
}

uint32_t Database::get_next_entity_id() 
{
  uint32_t next_id = m_next_entity_id;
  m_next_entity_id++;
  return next_id;
}

void Database::clear_element(uint32_t id) 
{
  BTree<Table*>::Iterator it = m_tables.iterator();
  while(it.has_next()) 
  {
    Table* table = *it.next();
    table->remove_element(id);
  }
}

void Database::tag_entity(uint32_t entity_id, 
                          const std::string& tag) 
{
  uint32_t hash_value = hash(tag.c_str());
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
  bit_table_ptr->add(entity_id);
}

void Database::untag_entity(uint32_t entity_id, 
                          const std::string& tag) 
{
  uint32_t hash_value = hash(tag.c_str());
  BitTable** bit_table = m_tags.get(hash_value);
  if(bit_table != nullptr) 
  {
    (*bit_table)->remove(entity_id);
  }
}

BitTable* 
Database::get_tagged_entities(const std::string& tag) 
{
  uint32_t hash_value = hash(tag.c_str());
  BitTable** bit_table = m_tags.get(hash_value);
  if(bit_table != nullptr) 
  {
    return *bit_table;
  }
  BitTable* bit_table_ptr = new BitTable();
  m_tags.insert_copy(hash_value, &bit_table_ptr);
  return bit_table_ptr;
}

void 
Database::add_reference( const std::string& type, 
                         uint32_t tail, 
                         uint32_t head) 
{
  /*auto it = m_references.find(type);
  if (it == m_references.end()) {
    int64_t hash_value = get_table_id(type);
    auto table = new Table(type, hash_value, sizeof(int32_t), &destructor<int32_t>);
    it = m_references.insert( std::make_pair(type, table)).first;
  }

  it->second->insert_element<int32_t>(tail, head);
  */
}

void 
Database::remove_reference( const std::string& type, 
                            uint32_t tail, 
                            uint32_t head) 
{
  /*auto it = m_references.find(type);
  if (it != m_references.end()) {
    it->second->remove_element(tail);
  }*/
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
    p_webserver = nullptr;
  }
}

  
} /* furious */ 
