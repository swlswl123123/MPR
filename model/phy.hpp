#pragma once
#include<log.h>
#include<opnet.h>

namespace phy_ns
{
typedef enum port
{
    tx_N,
    tx_E,
    tx_S,
    tx_W,
    rx_N,
    rx_E,
    rx_S,
    rx_W
}port_t;

typedef enum direct
{
    N,
    S,
    E,
    W
}direct_t;

};

class phy_ctrl
{
private:
    Objid obj;
    int id;
    int recv_direct;
public:
    phy_ctrl(/* args */);
    ~phy_ctrl();
    void send(int port, Packet* p);  // mode 0:NS 1:EW
    Packet* receive(int port, int mode);
    int get_id();
    int get_direct();
};

