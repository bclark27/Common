#include "EntityProperty.h"

#include "ArrayList.h"

///////////////
//  DEFINES  //
///////////////

#define INIT_DEST_COUNT     32

/////////////
//  TYPES  //
/////////////

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

void OnSettingValue(EntityProperty * self, void * newValue, ArrayList * propagationPath, bool isOrigin);

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

EntityProperty * EntityProperty_init(void * owner, char * name)
{
    EntityProperty * ep = calloc(1, sizeof(EntityProperty));
    EntityProperty_initInPlace(ep, owner, name);
    return ep;
}

void EntityProperty_initInPlace(EntityProperty * ep, void * owner, char * name)
{
    if (name == NULL)
        return;

    if (name != NULL)
    {
        ep->name = malloc(strlen(name) + 1);
        strcpy(ep->name, name);
    }

    ep->owner = owner;

    ep->destProps = List_listInit(sizeof(EntityProperty*), false, NULL, NULL);
}

void EntityProperty_free(EntityProperty * ep)
{
    if (ep == NULL)
        return;

    EntityProperty_freeInPlace(ep);

    free(ep);
}

void EntityProperty_freeInPlace(EntityProperty * ep)
{
    if (ep == NULL)
        return;

    free(ep->name);
    EntityProperty_UnLinkSource(ep);
    EntityProperty_UnlinkDestinations(ep);
    free(ep->destProps);

    ep->owner = NULL;
    ep->onValueChanged = NULL;
}

void EntityProperty_RefreshValue(EntityProperty * self)
{
    if (self == NULL)
        return;

    if (self->srcProp != NULL)
    {
        OnSettingValue(self, self->srcProp, NULL, true);
    }
    else
    {
        OnSettingValue(self, self->value, NULL, true);
    }
}

void EntityProperty_SetValue(EntityProperty * self, void * value)
{
    OnSettingValue(self, value, NULL, true);
}

// -1: a is src, 1: b is src, 0 no linkage, 2 is double linkage
U4 EntityProperty_AreLinked(EntityProperty * a, EntityProperty * b)
{
    if (a == NULL || b == NULL) return 0;

    bool aIsSrc = false;
    bool bIsSrc = false;

    // check first if a is src and b is dest
    // check if b is a member of the dest list for a
    bool bIsDestOfA = false;

    for (Link * curr = a->destProps->head; curr; curr = curr->next)
    {
        if (curr->data == b)
        {
            bIsDestOfA = true;
            break;
        }
    }

    // check if the src of b is a  
    aIsSrc = b->srcProp == a && bIsDestOfA;


    // check if b is src and a is dest
    // check if a is a member of the dest list for b
    bool aIsDestOfB = false;
    for (Link * curr = b->destProps->head; curr; curr = curr->next)
    {
        if (curr->data == a)
        {
            aIsDestOfB = true;
            break;
        }
    }


    // check if the src of a is b
    bIsSrc = a->srcProp == b && aIsDestOfB;

    if (aIsSrc && bIsSrc) return 2;
    if (aIsSrc) return -1;
    if (bIsSrc) return 1;

    // neither is the case
    return 0;
}

void EntityProperty_Link(EntityProperty * src, EntityProperty * dest)
{
    if (src == NULL || dest == NULL) return;
    if (EntityProperty_AreLinked(src, dest) == -1) return;

    bool destAlreadyExists = false;
    for (Link * curr = src->destProps->head; curr; curr = curr->next)
    {
        if (curr->data == dest)
        {
            destAlreadyExists = true;
            break;
        }
    }

    if (!destAlreadyExists)
        List_queue(src->destProps, dest);
    
    dest->srcProp = src;
}


void EntityProperty_UnLink(EntityProperty * src, EntityProperty * dest)
{
    if (src == NULL || dest == NULL) return;

    U4 linkage = EntityProperty_AreLinked(src, dest);
    if (linkage != -1 && linkage != 2) return;

    dest->srcProp = NULL;

    Link * curr = src->destProps->head;
    while (curr)
    {
        EntityProperty * p = curr->data;
        if (p == dest)
        {
            List_destroyLink(src->destProps, curr);
            break;
        }
        curr = curr->next;
    }
}

void EntityProperty_UnLinkSource(EntityProperty * self)
{
    if (self == NULL || self->srcProp == NULL) return;
    EntityProperty_UnLink(self->srcProp, self);
    self->srcProp = NULL;
}


void EntityProperty_UnlinkDestinations(EntityProperty * src)
{
    if (src == NULL || src->destProps == NULL)
        return;

    Link * curr = src->destProps->head;
    while (curr)
    {
        Link * next = curr->next;
        EntityProperty_UnLink(src, curr->data);
        curr = next;
    }
    List_clearList(src->destProps);
}

void EntityProperty_print(EntityProperty * self)
{
    if (self == NULL) return;
    printf("EntityProperty:\nName: %s\nValue: %p\nSource: ", self->name, self->value);
    printf("%s\n", self->srcProp == NULL ? "NULL" : self->srcProp->name);
    printf("Destinations (%d):\n", self->destProps->len);

    Link * curr = self->destProps->head;
    while (curr)
    {
        printf("\t%s\n", ((EntityProperty*)curr->data)->name);
        curr = curr->next;
    }
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

void OnSettingValue(EntityProperty * self, void * newValue, ArrayList * propagationPath, bool isOrigin)
{
    // null checks
    if (!self)
        return;

    if (isOrigin)
        propagationPath = ArrayList_init(sizeof(EntityProperty*), 20);

    // check if i am in the propigation path list
    bool cycleDetected = false;
    for (int i = 0; i < propagationPath->elementCount; i++)
    {
        EntityProperty * prop = ((EntityProperty**)propagationPath->data)[i];
        
        if (self == prop)
        {
            cycleDetected = true;
            break;
        }
    }

    // if i am, then return
    if (cycleDetected)
        return;

    // set my value to that of my source
    self->value = newValue;
    if (self->onValueChanged)
        self->onValueChanged(self, propagationPath);


    // notify all my destination properties
    ArrayList_append(propagationPath, &self);
    for (Link * curr = self->destProps->head; curr; curr = curr->next)
    {
        EntityProperty * dest = curr->data;
        OnSettingValue(dest, self->value, propagationPath, false);
    }

    if (isOrigin)
        ArrayList_freeList(propagationPath);
}