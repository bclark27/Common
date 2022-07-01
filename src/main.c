#include "Common.h"

int main()
{

  HashTable * ht = HashTable_init(sizeof(int), NULL);
  for (int i = 0; i < 100; i++)
  {
    HashTable_insert(ht, (unsigned char*)&i, sizeof(int), &i);
  }

  for (int i = 0; i < 100; i++)
  {
    int * a = HashTable_get(ht, (unsigned char*)&i, sizeof(int));
    printf("%d\n", *a);
  }

  HashTable_free(ht);
}
