#include "Net/EventLoop.h"

namespace Net {

EventLoop::EventLoop() 
    : m_threadId(std::this_thread::get_id()), 
      m_running(false) {

}

EventLoop::~EventLoop() {

}

bool EventLoop::loop() {
    return true;
}

bool EventLoop::quit() {
    return true;
}

bool EventLoop::wakeup() {
    return true;
}

bool EventLoop::updateChannel(ChannelPtr channel) {
    
    return true;
}

bool EventLoop::removeChannel(ChannelPtr channel) {
    return true;
}

} // namespace Net