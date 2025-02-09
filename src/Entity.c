#include "Entity.h"

///////////////
//  DEFINES  //
///////////////

/////////////
//  TYPES  //
/////////////

/////////////////////////////
//  FUNCTION DECLERATIONS  //
/////////////////////////////

EntityProperty * GetProperty(Entity * e, char * propName);
void OnPropertyChanged(EntityProperty * prop, ArrayList * propagationPath);

////////////////////////
//  PUBLIC FUNCTIONS  //
//////////////////////// 


Entity * Entity_init(char * name)
{
    if (name == NULL)
        return NULL;

    Entity * e = calloc(1, sizeof(Entity));
    if (name != NULL)
    {
        e->name = malloc(strlen(name) + 1);
        strcpy(e->name, name);
    }

    e->properties = List_listInit(sizeof(EntityProperty*), false, NULL, NULL);

    e->childrenArrLen = 16;
    e->children = calloc(sizeof(Entity*), e->childrenArrLen);

    return e;
}
void Entity_free(Entity * e)
{
    if (e == NULL)
        return;
    
    Entity_RemoveAllProperties(e);
    free(e->properties);
    
    free(e->name);
    free(e);
}

bool Entity_PropertyExists(Entity * e, char * propName)
{
    if (e == NULL || propName == NULL)
        return false;

    for (Link * curr = e->properties->head; curr; curr = curr->next)
    {
        EntityProperty * prop = curr->data;
        if (!strcmp(propName, prop->name))
            return true;
    }

    return false;
}

bool Entity_AddProperty(Entity * e, char * propName)
{
    if (e == NULL || propName == NULL || Entity_NameExists(e, propName))
        return false;
    
    // stuff to add here
    EntityProperty * prop = EntityProperty_init(e, propName);
    prop->onValueChanged = (CallbackFunction)OnPropertyChanged;
    List_queue(e->properties, prop);
    return true;
}

void Entity_RemoveProperty(Entity * e, char * propName)
{
    if (e == NULL || propName == NULL)
        return;

    for (Link * curr = e->properties->head; curr; curr = curr->next)
    {
        EntityProperty * prop = curr->data;
        if (!strcmp(propName, prop->name))
        {
            EntityProperty_free(prop);
            List_destroyLink(e->properties, curr);
            return;
        }
    }
}

void Entity_RemoveAllProperties(Entity * e)
{
    if (e == NULL)
        return;
    Link * curr = e->properties->head;
    while (curr)
    {
        Link * next = curr->next;
        EntityProperty * prop = curr->data;
        EntityProperty_free(prop);
        curr = next;
    }
    List_clearList(e->properties);
}

void * Entity_GetPropertyValue(Entity * e, char * propName)
{
    EntityProperty * prop = GetProperty(e, propName);

    if (prop)
        return prop->value;
    return NULL;
}

void Entity_SetPropertyValue(Entity * e, char * propName, void * value)
{
    EntityProperty * prop = GetProperty(e, propName);
    if (prop)
    {
        EntityProperty_SetValue(prop, value);
    }
}

void Entity_LinkProperties(Entity * src_e, char * src_p, Entity * dest_e, char * dest_p)
{
    if (src_e == NULL ||
        src_p == NULL ||
        dest_e == NULL ||
        dest_p == NULL)
        return;
    
    EntityProperty * s = GetProperty(src_e, src_p);
    EntityProperty * d = GetProperty(dest_e, dest_p);

    if (s == NULL || d == NULL)
        return;
    
    EntityProperty_Link(s, d);
}

bool Entity_NameExists(Entity * e, char * name)
{
    if (Entity_PropertyExists(e, name))
        return true;

    for (int i = 0; i < e->childrenCount; i++)
    {
        Entity * c = e->children[i];
        if (!strcmp(c->name, e->name))
            return true;
    }

    return false;
}

bool Entity_AddChildEntity(Entity * p, Entity * c)
{
    if (p == c || !p || !c || Entity_NameExists(p, c->name))
        return false;

    if (p->childrenCount == p->childrenArrLen)
    {
        Entity** newChildrenList = calloc(sizeof(Entity*), p->childrenArrLen * 2);
        memcpy(newChildrenList, p->children, p->childrenArrLen * sizeof(Entity*));
        free(p->children);
        p->children = newChildrenList;
        p->childrenArrLen *= 2;
    }
    
    Entity_RemoveChildEntity(c->parent, c);
    c->parent = p;
    p->children[p->childrenCount++] = c;
    return true;
}

bool Entity_RemoveChildEntity(Entity * p, Entity * c)
{
    if (p == c || !p || !c)
        return false;

    bool found = false;
    int i = 0;
    for (; i < p->childrenCount; i++)
    {
        if (p->children[i] == c)
        {
            found = true;
            break;   
        }
    }

    if (!found)
        return false;

    c->parent = NULL;

    for (; i < p->childrenCount - 1; i++)
        p->children[i] = p->children[i + 1];
    p->childrenCount--;

    return true;
}

/////////////////////////
//  PRIVATE FUNCTIONS  //
/////////////////////////

EntityProperty * GetProperty(Entity * e, char * propName)
{
    if (e == NULL || propName == NULL)
        return NULL;

    for (Link * curr = e->properties->head; curr; curr = curr->next)
    {
        EntityProperty * prop = curr->data;
        if (!strcmp(propName, prop->name))
        {
            return prop;
        }
    }

    return NULL;
}

void OnPropertyChanged(EntityProperty * prop, ArrayList * propagationPath)
{
    Entity * self = prop->owner;

    printf("%s: %s\n", self->name, prop->name);
}