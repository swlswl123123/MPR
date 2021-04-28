
#include <transfer_queue.hpp>

using namespace std;
transfer_queue::transfer_queue() : buffer_size(0) {}

transfer_queue::~transfer_queue() {}

void transfer_queue::push(Packet* p)
{
    priority_queue<Packet*>::push(p);
    buffer_size += op_pk_total_size_get(p);
}

void transfer_queue::pop()
{
    buffer_size -= op_pk_total_size_get(top());
    priority_queue<Packet*>::pop();
}

unsigned long transfer_queue::calc_size()
{
    return buffer_size;
}
