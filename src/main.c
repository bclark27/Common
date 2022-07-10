#include <stdio.h>
#include "Common.h"
#include "HashSet.h"

int main()
{
  mem_init();

  HashSet * hs = HashSet_init(4, true, NULL);
  int * a = malloc(4);

  for (int i = 0; i < 1; i++)
  {
    HashSet_insert(hs, &i);
  }

  free(a);free(a);
  mem_print();
  HashSet_free(hs);
  mem_freeTracker();
}
