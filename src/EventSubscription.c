#include "EventSubscription.h"

#include "Standard.h"

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

static char subscriberAlreadyExists(SubscriptionChain * subChain, void * subscriber);
static void freeSubscription(void * s);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

//public function to create a new event subscription chain
SubscriptionChain * subChain_subscriptionChainInit()
{
  return List_listInit();
}

//public function too completely free up an event subscription chain
void subChain_freeSubscriptionChain(SubscriptionChain * sc)
{
  List_destroyList(sc, freeSubscription);
}

char subChain_addSubscription(SubscriptionChain * subChain, void * subscriber, EventHandle handle)
{
  if(subChain && !subscriberAlreadyExists(subChain, subscriber))
  {
    Subscription * sub = (Subscription *)malloc(sizeof(Subscription));
    if(!sub)
    {
      return 0;
    }
    sub->subscriber = subscriber;
    sub->eventHandle = handle;
    return List_queue(subChain, sub);
  }
  return 0;
}

void subChain_removeSubscription(SubscriptionChain * subChain, void * subscriber)
{
  if(subChain && subChain->len > 0)
  {
    for(Link * link = subChain->head; link; link = link->next)
    {
      Subscription * sub = (Subscription *)(link->data);
      if(sub->subscriber == subscriber)
      {
        //bug here??? idk maybe seg fault i cant see yet. second line down is origonal
        _destroyLink(subChain, link, freeSubscription);
        //deleteItem(subChain, sub, freeSubscription);
        return;
      }
    }
  }
}

void subChain_eventTrigger(SubscriptionChain * subChain, void * args)
{
  if(subChain)
  {
    for(Link * link = subChain->head; link; link = link->next)
    {
      Subscription * sub = (Subscription *)(link->data);
      sub->eventHandle(sub->subscriber, args);
    }
  }
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

static char subscriberAlreadyExists(SubscriptionChain * subChain, void * subscriber)
{
  for(Link * link = subChain->head; link; link = link->next)
  {
    Subscription * subscription = (Subscription *)(link->data);
    void * sub = subscription->subscriber;
    if(sub == subscriber)
    {
      return 1;
    }
  }
  return 0;
}

static void freeSubscription(void * s){
  free((Subscription *) s);
}
