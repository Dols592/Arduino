/* 
*/
 
#include <stdint.h>
 
enum NSetConfigItemType
{
  NSIT_NONE = 0,
  NSIT_BOOL,
  NSIT_INT32,
  NSIT_UINT32,
  NSIT_STRING,
  NSIT_COUNT
};

struct SSoftConfigItem
{
  char* ItemName;
  NSetConfigItemType Type;
  void* pValue;
  bool Persistant;

  SSoftConfigItem()
  {
    ItemName = 0;
    Type = NSIT_NONE;
    pValue = 0;
    Persistant = true;
  }
};

class CSoftConfig
{
public: //Interface
  bool AddConfigItem(char ItemName, NSetConfigItemType Type, void* pValue, bool Persistant = true);
  
protected: //Help functions

protected: //Variables
};

