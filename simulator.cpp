//
// Created by moedinger on 01.03.19.
//
#include <iostream>

#include "simulator.h"

// ---------------------- event -----------------------
event::event(decltype(action) a, decltype(timestamp) t) {
    timestamp = t;
    action = a;
}


// ---------------------- simulator -----------------------
simulator::simulator() : eventqueue(eventComp), log_stream(std::cout), csv_stream(std::cout) {}

void simulator::addEvent(event e) {
    eventqueue.push(e);
}

void simulator::addEventIn(std::function<void()> a, uint64_t delta) {
    addEvent(event(a,clock+delta));
}

void simulator::addEventAt(std::function<void()> a, uint64_t time) {
    addEvent(event(a,std::max(time,clock)));
}

uint64_t simulator::now() {
    return clock;
}

void simulator::next() {
    // get event
    auto ev = eventqueue.top();

    // set time to time of event
    // events have timestamps which are increasing monotonicaly
    // (as they are pulled from a priority queue sorted by timestamp)
    clock = ev.timestamp;
    // process event
    ev.action();

    // Remove event
    eventqueue.pop();
}

bool simulator::empty() {
    return eventqueue.empty();
}

void simulator::log(std::string entry) {
    if(log_verbosity)
        log_stream << clock << ": " << entry << std::endl;

}

void simulator::csv(std::string entry) {
    if(csv_verbosity)
        csv_stream << clock << "," << entry << std::endl;
}
