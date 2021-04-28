#pragma once
#include <queue>
#include <vector>
#include <algorithm>
#include <opnet.h>

using namespace std;
class transfer_queue:public priority_queue<Packet*>
{
private:
    unsigned long buffer_size;
public:
    transfer_queue(/* args */);
    ~transfer_queue();
    void push(Packet *p);
    void pop();
    unsigned long calc_size();
};
