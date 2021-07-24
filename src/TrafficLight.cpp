#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
  	std::unique_lock<std::mutex> uLcK(_mtx);
  	_condition.wait(uLcK, [this]{return !_queue.empty();});
    // to wait for and receive new messages and pull them from the queue using move semantics.
  	T NewMessage = std::move(_queue.back());
    // The received object should then be returned by the receive function.
  	_queue.pop_back();
  	return NewMessage;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> lck(_mtx);
  	_queue.emplace_back(std::move(msg));
  	_condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
  	while(true){
    // runs and repeatedly calls the receive function on the message queue.
      TrafficLightPhase phase = _phasesQueue.receive();
    // Once it receives TrafficLightPhase::green, the method returns.
      if (phase == TrafficLightPhase::green){
      	break;
      }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  std::random_device rd;
  //time move
  std::uniform_int_distribution<int> distribution (4000,6000);
  long cycle_duration= distribution(rd);
  // start time
  lastUpdate = std::chrono::system_clock::now();
  
  //while loop 
  while(true){
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    long timeUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
    if(timeUpdate >= cycle_duration){
      _currentPhase = (_currentPhase == TrafficLightPhase::red )? TrafficLightPhase::green : TrafficLightPhase::red;
      _phasesQueue.send(std::move(_currentPhase));
      lastUpdate = std::chrono::system_clock::now();
      cycle_duration = distribution(rd);
    }
  }
}

