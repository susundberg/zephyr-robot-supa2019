#include "change_filter.h"
#include "../main.h"

bool change_filtered( s32_t new_value, ChangeFilter* filter )
{
   if ( ABS( new_value  - filter->value_last ) > filter->value_th )
   {
       filter->value_last = new_value;
       return true;
   }
   return false;
}

