#pragma once
// #include <deque>
#include <queue>
#include <vector>
#include <antenna.hpp>
#include <types.hpp>
#include <transfer_queue.hpp>

#define scan_round 10
#define FRAME_INTERVAL 50e-3
#define D_INTERVAL 1e-6
#define BIT_PER_UNIT 15000
#define PI 3.1415926535


enum prio_t
{
DBCH,  //Data broadcast channel
CBCH,  //Control broadcast channel
DUCH,  //Data unicast channel
UDBCH, //Urgent data broadcast channel
UCBCH, //Urgent control broadcast channel
UDUCH, //Urgent data unicast channel
CUCH,  //Control unicast channel
UCUCH  //Urgent control unicast channel
};

typedef enum intrpt
{
    C,
    B,
    D = 0x400
}intrpt_t;

typedef enum
{
  tx,
  rx
}port;

typedef enum ptype
{
  ETHERTYPE_IP,
  ETHERTYPE_ARP,
  ETHERTYPE_IPV6,
  EHTERTYPE_BALLOC, //Beacon allocation message type
  EHTERTYPE_RINFO,   //Link info message type
  EHTERTYPE_URG_PRE,
  EHTERTYPE_URG_END,  
  EHTERTYPE_POS,
  EHTERTYPE_CHANGE,
  EHTERTYPE_CHANGE_B
} ptype_t;

// implement struct

class tdma_ctrl
{
private:
    int id;
    Objid obj;
    FwdTbl_t forward_table;      
    transfer_queue txq;
    unsigned long long my_seq_num;  // My broadcasting seq nu
public:
    unsigned long long seq_num[50]; 
    tdma_ctrl(/* args */);
    ~tdma_ctrl();
    void slot_ctrl(int type);
    void update_forward_table(FwdTbl_t F);
    int proc_broadcast(Packet* p);
    void proc_unicast(Packet* p);
    void send_proc();
    Packet* encap_mac(Packet *p, int dst, int prio, int type);
    Packet *encap_broadcast(Packet *p);
    unsigned long long get_txq();
    int get_id();
};


