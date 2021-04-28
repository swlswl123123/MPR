#pragma once
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <tdma.hpp>

const int topo[5][5] = {
        {0, 1, 5, 9, 13},
        {3, 0, 9, 13, 17},
        {7, 11, 0, 17, 21},
        {11, 15, 19, 0, 25},
        {15, 19, 23, 27, 0}
        };

typedef struct
{
    int nid;
    int weight;
}node;

class router
{
public:
    router();
    router(tdma_ctrl* t);
    void send_hello();
    void calc_route();
    void handle_hello(Packet *p);
    int calc_weight(int src);
    ~router();
private:
    int id;
    FwdTbl_t FWD;
    unsigned long long load_table[50];
    vector<int> MPR_selector;
    vector<int> MPR;
};
