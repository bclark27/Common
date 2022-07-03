#include "Common.h"
#include "TypedHashTable.h"
#include "HashSet.h"

typedef struct
{
  U4 n;
  U4 m;
} gridTravlerKey;

U8 gridTravelerHelper(U4 n, U4 m, TypedHashTable * ht)
{
  if (n == 1 || m == 1) return 1;
  if (n == 0 || m == 0) return 0;

  gridTravlerKey key = {n, m};
  U8 * cached = TypedHashTable_getRef(ht, &key);

  if (cached)
  {
    return *cached;
  }

  U8 ans = gridTravelerHelper(n - 1, m, ht) + gridTravelerHelper(n, m - 1, ht);

  TypedHashTable_insert(ht, &key, &ans);

  return ans;
}

U8 gridTraveler(U4 n, U4 m)
{
  if (n == 1 || m == 1) return 1;
  if (n == 0 || m == 0) return 0;

  TypedHashTable * ht = TypedHashTable_init(sizeof(gridTravlerKey), sizeof(U8), true, NULL);

  U8 ans = gridTravelerHelper(n - 1, m, ht) + gridTravelerHelper(n, m - 1, ht);

  TypedHashTable_free(ht);

  return ans;
}

int main()
{
  TypedHashTable * h = TypedHashTable_init(sizeof(U4), sizeof(U4), true, NULL);

  for (U4 i = 0; i < 100; i++)
  {
    TypedHashTable_insert(h, &i, &i);
  }

  for (U4 i = 0; i < 100; i++)
  {
    printf("%d\n", *(U4 *)TypedHashTable_getRef(h, &i));
  }

  TypedHashTable_free(h);
}
