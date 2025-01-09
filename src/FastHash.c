#include "FastHash.h"

/////////////////////////////
//  FUNDTION DECLERATIONS  //
/////////////////////////////

static void updateHashState(U4 * state);

///////////////////////
//  PUBLIC FUNDTION  //
///////////////////////

U4 FastHash_hash(void * key, U4 keyLen)
{
  if (key == NULL || keyLen == 0) return 0;

  U4 state = HASH_SEED;
  U4 temp;
  U1 * charKey = (U1 *)key;
  for (int i = 0; i < keyLen; i++)
  {
    // update the hash state
    temp = ((U4)charKey[i]) << 24;
    state = ((state ^ temp) & 0xff000000) | (state & 0x00ffffff);
    updateHashState(&state);
  }

  return state;
}


/////////////////////////
//  PRIVATE FUNDTIONS  //
/////////////////////////

static void updateHashState(U4 * state)
{
  // update the state (default is 8 iterations)
  for (U4 i = 0; i < HASH_ROUNDS; i++)
  {
    // if the lowest bit is 1, shift and xor with feedback
    if (*state & 1)
    {
      *state = (*state >> 1) ^ FEEDBACK_VAL;
    }
    // else just shift
    else
    {
      *state = (*state >> 1);
    }
  }
}
