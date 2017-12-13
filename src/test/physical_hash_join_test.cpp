

#include "data_test.h"
#include "../data/physical/physical_scan.h"
#include "../data/physical/physical_hash_join.h"

namespace furious {

    class PhysicalHashJoinTest : public DataTest {
    };

    TEST_F(PhysicalHashJoinTest, PhysicalHashJoinTestEquivalentTables) {

      for(uint32_t i = 0; i < 10000; ++i) {
        tableA_->insert(i,i*2,i*1.0);
      }

      for(uint32_t i = 0; i < 10000; ++i) {
        tableB_->insert(i,i*4,i*3.0);
      }

      IPhysicalOperatorSPtr physical_scanA( new PhysicalScan(tableA_));
      IPhysicalOperatorSPtr physical_scanB( new PhysicalScan(tableB_));
      IPhysicalOperatorSPtr physical_hash_join( new PhysicalHashJoin(physical_scanA, physical_scanB) );

      physical_hash_join->open();
      BaseRow* joined_row = physical_hash_join->next();
      EntityId entity_id = 0;
      ASSERT_NE(joined_row, nullptr);
      while(joined_row != nullptr) {
        ASSERT_TRUE(joined_row->m_id == entity_id);
        ComponentA* componentA = reinterpret_cast<ComponentA*>(joined_row->column(0));
        ComponentB* componentB = reinterpret_cast<ComponentB*>(joined_row->column(1));
        ASSERT_EQ(componentA->field1_, entity_id*2);
        ASSERT_EQ(componentA->field2_, entity_id*1.0);
        ASSERT_EQ(componentB->field1_, entity_id*4);
        ASSERT_EQ(componentB->field2_, entity_id*3.0);
        joined_row = physical_hash_join->next();
        entity_id+=1;
      }
      physical_hash_join->close();
      tableA_->clear();
      tableB_->clear();
    }

    TEST_F(PhysicalHashJoinTest, PhysicalHashJoinTestOverlappTables) {

      for(uint32_t i = 0; i < 5000; ++i) {
        tableA_->insert(i,i*2,i*1.0);
      }

      for(uint32_t i = 2500; i < 7500; ++i) {
        tableB_->insert(i,i*4,i*3.0);
      }

      IPhysicalOperatorSPtr physical_scanA( new PhysicalScan(tableA_));
      IPhysicalOperatorSPtr physical_scanB( new PhysicalScan(tableB_));
      IPhysicalOperatorSPtr physical_hash_join( new PhysicalHashJoin(physical_scanA, physical_scanB) );

      physical_hash_join->open();
      BaseRow* joined_row = physical_hash_join->next();
      EntityId entity_id = 2500; 
      ASSERT_NE(joined_row, nullptr);
      while(joined_row != nullptr) {
        ASSERT_TRUE(joined_row->m_id == entity_id);
        ComponentA* componentA = reinterpret_cast<ComponentA*>(joined_row->column(0));
        ComponentB* componentB = reinterpret_cast<ComponentB*>(joined_row->column(1));
        ASSERT_EQ(componentA->field1_, entity_id*2);
        ASSERT_EQ(componentA->field2_, entity_id*1.0);
        ASSERT_EQ(componentB->field1_, entity_id*4);
        ASSERT_EQ(componentB->field2_, entity_id*3.0);
        joined_row = physical_hash_join->next();
        entity_id+=1;
      }
      ASSERT_EQ(entity_id,5000);
      physical_hash_join->close();
      tableA_->clear();
      tableB_->clear();
    }

    TEST_F(PhysicalHashJoinTest, PhysicalHashJoinTestDisjointTables) {

      for(uint32_t i = 0; i < 5000; ++i) {
        tableA_->insert(i,i*2,i*1.0);
      }

      for(uint32_t i = 5000; i < 10000; ++i) {
        tableB_->insert(i,i*4,i*3.0);
      }

      IPhysicalOperatorSPtr physical_scanA( new PhysicalScan(tableA_));
      IPhysicalOperatorSPtr physical_scanB( new PhysicalScan(tableB_));
      IPhysicalOperatorSPtr physical_hash_join( new PhysicalHashJoin(physical_scanA, physical_scanB) );

      physical_hash_join->open();
      BaseRow* joined_row = physical_hash_join->next();
      ASSERT_EQ(joined_row, nullptr);
      physical_hash_join->close();
      tableA_->clear();
      tableB_->clear();
    }

    struct ComponentC {
      uint32_t field1_;
      double field2_;
      ComponentC(uint32_t field1, double field2 ) : 
        field1_(field1),
        field2_(field2) 
      {}

      static std::string name() { return "ComponentC"; }

    };


    struct ComponentD {
      uint32_t field1_;
      double field2_;
      ComponentD(uint32_t field1, double field2 ) : 
        field1_(field1),
        field2_(field2) 
      {}

      static std::string name() { return "ComponentD"; }

    };

    template<>
      std::string type_name<ComponentC>() {
        return "ComponentC";
      }

    template<>
      std::string type_name<ComponentD>() {
        return "ComponentD";
      }

    TEST_F(PhysicalHashJoinTest, PhysicalHashJoinChain) {

      for(uint32_t i = 0; i < 10000; ++i) {
        tableA_->insert(i,i*2,i*1.0);
      }

      for(uint32_t i = 0; i < 10000; ++i) {
        tableB_->insert(i,i*4,i*3.0);
      }

      auto tableC_ = database_->create_table<ComponentC>();

      for(uint32_t i = 0; i < 10000; ++i) {
        tableC_->insert(i,i*8,i*6.0);
      }

      auto tableD_ = database_->create_table<ComponentD>();

      for(uint32_t i = 0; i < 10000; ++i) {
        tableD_->insert(i,i*16,i*9.0);
      }

      IPhysicalOperatorSPtr physical_scanA( new PhysicalScan(tableA_));
      IPhysicalOperatorSPtr physical_scanB( new PhysicalScan(tableB_));
      IPhysicalOperatorSPtr physical_scanC( new PhysicalScan(tableC_));
      IPhysicalOperatorSPtr physical_scanD( new PhysicalScan(tableD_));
      IPhysicalOperatorSPtr physical_hash_join1( new PhysicalHashJoin(physical_scanA, physical_scanB) );
      IPhysicalOperatorSPtr physical_hash_join2( new PhysicalHashJoin(physical_hash_join1, physical_scanC) );
      IPhysicalOperatorSPtr physical_hash_join3( new PhysicalHashJoin(physical_hash_join2, physical_scanD) );

      physical_hash_join3->open();
      BaseRow* joined_row = physical_hash_join3->next();
      EntityId entity_id = 0; 
      ASSERT_NE(joined_row, nullptr);
      while(joined_row != nullptr) {
        ASSERT_EQ(joined_row->m_id, entity_id);
        ASSERT_EQ(joined_row->num_columns(), 4);
        ASSERT_EQ(sizeof(ComponentA),joined_row->column_size(0));
        ASSERT_EQ(sizeof(ComponentB),joined_row->column_size(1));
        ASSERT_EQ(sizeof(ComponentC),joined_row->column_size(2));
        ASSERT_EQ(sizeof(ComponentD),joined_row->column_size(3));

        ComponentA* componentA = reinterpret_cast<ComponentA*>(joined_row->column(0));
        ASSERT_EQ(componentA->field1_, entity_id*2);
        ASSERT_EQ(componentA->field2_, entity_id*1.0);
        ComponentB* componentB = reinterpret_cast<ComponentB*>(joined_row->column(1));
        ASSERT_EQ(componentB->field1_, entity_id*4);
        ASSERT_EQ(componentB->field2_, entity_id*3.0);
        ComponentC* componentC = reinterpret_cast<ComponentC*>(joined_row->column(2));
        ASSERT_EQ(componentC->field1_, entity_id*8);
        ASSERT_EQ(componentC->field2_, entity_id*6.0);
        ComponentD* componentD = reinterpret_cast<ComponentD*>(joined_row->column(3));
        ASSERT_EQ(componentD->field1_, entity_id*16);
        ASSERT_EQ(componentD->field2_, entity_id*9.0);
        joined_row = physical_hash_join3->next();
        entity_id+=1;
      }
      ASSERT_EQ(entity_id,10000);
      physical_hash_join3->close();
      tableA_->clear();
      tableB_->clear();
      tableC_->clear();
      tableD_->clear();
      database_->drop_table<ComponentC>();
      database_->drop_table<ComponentD>();
    }
    
} /* furious */ 


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
