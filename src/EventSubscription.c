#include "EventSubscription.h"

#include "Common.h"

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static bool subscriberAlreadyExists(SubscriptionChain * subChain, void * subscriber);
static void freeSubscription(void * s);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

//public function to create a new event subscription chain
SubscriptionChain * subChain_subscriptionChainInit()
{
  return List_listInit(sizeof(Subscription), true, NULL, freeSubscription);
}

//public function too completely free up an event subscription chain
void subChain_freeSubscriptionChain(SubscriptionChain * sc)
{
  List_destroyList(sc);
}

char subChain_addSubscription(SubscriptionChain * subChain, void * subscriber, EventHandle handle)
{
  if(subChain && !subscriberAlreadyExists(subChain, subscriber))
  {
    Subscription sub = {.subscriber = subscriber, .eventHandle = handle};
    List_queue(subChain, &sub);
    return true;
  }

  return false;
}

void subChain_removeSubscription(SubscriptionChain * subChain, void * subscriber)
{
  if(!subChain || subChain->len <= 0) return;

  ListItr itr = List_getItr(subChain);
  Subscription * sub = List_getNextRef(&itr);
  U4 i;

  for (i = 0; sub; i++, sub = List_getNextRef(&itr))
  {
    if (sub->subscriber == subscriber) break;
  }

  if (!sub) return;

  List_deleteItem(subChain, i);
}

void subChain_eventTrigger(SubscriptionChain * subChain, void * args)
{
  if(!subChain) return;

  ListItr itr = List_getItr(subChain);
  Subscription * sub = List_getNextRef(&itr);

  while (sub)
  {
    sub->eventHandle(sub->subscriber, args);
    sub = List_getNextRef(&itr);
  }
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static bool subscriberAlreadyExists(SubscriptionChain * subChain, void * subscriber)
{
  for(Link * link = subChain->head; link; link = link->next)
  {
    Subscription * subscription = (Subscription *)(link->data);
    void * sub = subscription->subscriber;
    if(sub == subscriber)
    {
      return true;
    }
  }
  
  return false;
}

static void freeSubscription(void * s){
  free((Subscription *) s);
}
