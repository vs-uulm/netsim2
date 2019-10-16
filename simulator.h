//
// Created by moedinger on 01.03.19.
//

#include <cstdint>
#include <queue>
#include <functional>

#ifndef NETSIM2_SIMULATOR_H
#define NETSIM2_SIMULATOR_H

/**
 * Event: function 'action' is called by the simulator when timestamp 'timestamp' is reached in the simulation
 */
struct event {
    uint64_t timestamp;
    std::function<void()> action;
    event(decltype(action), decltype(timestamp));
};

/**
 *
 */
const auto eventComp = [](const event& a, const event& b) {return a.timestamp > b.timestamp; };

/**
 * Simulator: Manages a clock and an eventque.
 * Allows scheduling of events and manages logging.
 */
class simulator {
    // simulation
    uint64_t clock = 0;
    std::priority_queue<event,std::vector<event>, decltype(eventComp)> eventqueue;

public:
    // logging streams
    bool log_verbosity = true;
    std::ostream& log_stream;
    bool csv_verbosity = true;
    std::ostream& csv_stream;

    simulator();

    void addEvent(event);
    void addEventIn(std::function<void()>, uint64_t delta);
    void addEventAt(std::function<void()>, uint64_t time);

    uint64_t now();

    void next();
    bool empty();

    void log(std::string);
    void csv(std::string);
};


#endif //NETSIM2_SIMULATOR_H
