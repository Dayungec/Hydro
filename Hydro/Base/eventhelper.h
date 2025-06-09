#ifndef EVENTHELPER_H
#define EVENTHELPER_H
#include "autoptr.h"
#include "semaphora.h"
#include "variant.h"
namespace SGIS{

class EventArgs;
class Event;
class EventHelper;

typedef void* Sender;
typedef void* Receiver;

typedef void(*EVENTFUN) (Event&);

class SGIS_EXPORT EventArgs{
public:
    EventArgs(const map<string,Variant>&parameters={});
    EventArgs(const EventArgs&other);
    virtual~EventArgs();
    EventArgs&operator=(const EventArgs&other);
    Variant operator[](string key);
protected:
    map<string,Variant>parameters;
};

class SGIS_EXPORT Event
{
public:
    Event();
    Event(Sender sender,EventArgs args);
    ~Event();
    void RaiseEvent(Sender sender,const EventArgs&args);
    void RaiseEvent(Sender sender);
    Sender GetSender();
    Receiver GetReceiver();
    EventArgs&GetEventArgs();
    Variant operator[](string key);
protected:
    void Remove(EventHelper*listener);
private:
    vector<EventHelper*>listeners;
    vector<EVENTFUN>funcs;
    vector<void*>owners;
    friend class EventHelper;
protected:
    Sender sender;
    Receiver receiver;
    EventArgs args;
    //AutoPtr<Mutex>mutex;
};

class SGIS_EXPORT EventHelper
{
public:
    EventHelper();
    virtual~EventHelper();
    void Add(Event*event,EVENTFUN fun,void*owner=nullptr);
    void Remove(Event*event);
    void RemoveBySender(Sender sender);
    void RemoveByReceiver(Receiver receiver);
protected:
    vector<Event*>events;
    friend class Event;
    AutoPtr<Mutex>mutex;
};

}
#endif // EVENTHELPER_H
