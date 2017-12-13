

#include <gtest/gtest.h>
#include "data_test.h"
#include "../data/physical/physical_scan.cpp"

namespace furious {

    class PhysicalScanTest : public DataTest {

    };

    TEST_F(PhysicalScanTest,PhysicalScanTest) {

      for(uint32_t i = 0; i < 1; ++i) {
        tableA_->insert(i,i*2,i*1.0);
      }

      PhysicalScan physical_scan(tableA_);
      physical_scan.open();
      
      uint32_t current_id = 0;
      BaseRow* row = physical_scan.next();
      ASSERT_TRUE(row != nullptr);
      while(row != nullptr) {
        ComponentA* component = reinterpret_cast<ComponentA*>(row->column(0));
        ASSERT_EQ(component->field1_,(current_id*2)); 
        ASSERT_EQ(component->field2_,(current_id*1.0));
        row = physical_scan.next();
        current_id+=1;
      }
      physical_scan.close();

    }
  
} /* furious */ 


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
