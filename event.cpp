#include <sstream>
#include "class.h"

void cl_eventQueue::insertEvent(cl_event event)
{
    this->queue.insert(event);
}

void cl_eventQueue::deleteEvent()
{
    this->queue.erase(this->queue.begin());
}

cl_event cl_eventQueue::getFirst()
{
return *(this->queue.begin());
}
