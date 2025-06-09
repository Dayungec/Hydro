#include "eventhelper.h"
namespace SGIS{

EventArgs::EventArgs(const map<string,Variant>&parameters){
    this->parameters=parameters;
}
EventArgs::EventArgs(const EventArgs&other){
    parameters=other.parameters;
}
EventArgs::~EventArgs(){

}
EventArgs&EventArgs::operator=(const EventArgs&other){
    parameters=other.parameters;
    return *this;
}
Variant EventArgs::operator[](string key){
    return parameters[key];
}

Event::Event(){
    //mutex=new Mutex();
    sender=nullptr;
    receiver=nullptr;
}
Event::Event(Sender sender,EventArgs args){
    //mutex=new Mutex();
    this->sender=sender;
    this->args=args;
    receiver=nullptr;
}
Event::~Event(){
   for(int k=listeners.size()-1;k>=0;k--){
       listeners[k]->Remove(this);
   }
   listeners.clear();
}

void Event::RaiseEvent(Sender sender){
    //mutex->Lock();
    this->sender=sender;
    this->args=EventArgs();
    for(int k=0;k<funcs.size();k++){
        this->receiver=owners[k];
        funcs[k](*this);
    }
    this->args={};
    //mutex->Unlock();
}
void Event::RaiseEvent(Sender sender,const EventArgs&args){
    //mutex->Lock();
    this->sender=sender;
    this->args=args;
    for(int k=0;k<funcs.size();k++){
        this->receiver=owners[k];
        funcs[k](*this);
    }
    this->args={};
    //mutex->Unlock();
}

Sender Event::GetSender(){
    return sender;
}

Receiver Event::GetReceiver(){
    return receiver;
}

EventArgs&Event::GetEventArgs(){
    return args;
}

Variant Event::operator[](string key){
    return args[key];
}

void Event::Remove(EventHelper*listener){
    //mutex->Lock();
    for(int k=listeners.size()-1;k>=0;k--){
        if(listeners[k]==listener){
            listeners.erase(begin(listeners)+k);
            funcs.erase(begin(funcs)+k);
            owners.erase(begin(owners)+k);
        }
    }
    //mutex->Unlock();
}

EventHelper::EventHelper(){
    mutex=new Mutex();
}

EventHelper::~EventHelper(){
   for(int k=events.size()-1;k>=0;k--){
       events[k]->Remove(this);
   }
   events.clear();
}
void EventHelper::Add(Event*event,EVENTFUN fun,void*owner){
   mutex->Lock();
   for(int k=event->listeners.size()-1;k>=0;k--){
       if((event->listeners[k]==this)&&(event->funcs[k]==fun)){
           mutex->Unlock();
           return;
       }
   }
   event->listeners.push_back(this);
   event->funcs.push_back(fun);
   event->owners.push_back(owner);
   this->events.push_back(event);
   mutex->Unlock();
}

void EventHelper::Remove(Event*event){
    mutex->Lock();
    for(int k=events.size()-1;k>=0;k--){
       if(events[k]==event){
           events.erase(begin(events)+k);
       }
    }
    mutex->Unlock();
}

void EventHelper::RemoveBySender(Sender sender){
    mutex->Lock();
    for(int k=events.size()-1;k>=0;k--){
       if(events[k]->GetSender()==sender){
           events.erase(begin(events)+k);
       }
    }
    mutex->Unlock();
}


void EventHelper::RemoveByReceiver(Receiver receiver){
    mutex->Lock();
    for(int k=events.size()-1;k>=0;k--){
       if(events[k]->GetReceiver()==receiver){
           events.erase(begin(events)+k);
       }
    }
    mutex->Unlock();
}

}
