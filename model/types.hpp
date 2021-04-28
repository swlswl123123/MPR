#pragma once
#include <iostream>
namespace balloc
{
typedef enum phase
{
    HELLO,
    REQ,
    ACK,
} phase_t;

typedef enum mode
{
    INVALID,
    RECV,
    SEND
} mode_t;

typedef enum beacon_type
{
    VACCANT,     //Indicate current beacon is vaccant
    PREORDAINED, //Indicate current beacon is pre-ordained
    EXTENDED,    //Indicate current beacon is occupied as extended beacon i.e. can be removed as required
    PRIMARY ,    //Indicate current beacon is occupied as primary beacon
    URGENT ,      //Indicate current beacon is occupied as urgent beacon i.e. can only be released by the allocator
    DENIELED
} beacon_type_t;

typedef enum session_type
{
    PRIMARY_BALLOC,
    EXTENDED_BALLOC,
    URGENT_BALLOC    
} session_type;

typedef enum cmd1
{
    MAINTAIN,
    ALLOCATE,
    RELEASE,
} cmd1_t;

typedef enum ack
{
    NOP,
    APPROVE,
    DENIEL
} ack_t;

typedef struct beacon
{
    union {
        cmd1_t command : 3;
        ack_t ack : 3;
    };
    mode_t mode : 3;
    beacon_type_t type : 4;
    unsigned int id : 6;
} beacon_t;

typedef struct beacon_info
{
    beacon_t links[96];
} beacon_info_t;

typedef struct beacon_info_all
{
    beacon_info_t info_sub[4];  // 0:N 1:S 2:E 3:W
}beacon_info_all_t;
};

typedef struct
{
    bool PRIMARY : 1;
    bool MPR : 1;
    unsigned short id : 6;
} link_t;//实现时采用char+char+short

    /* forward table*/
struct FwdTbl_t
{
    link_t link_table[50];
};

struct FwdTblAll_t
{
    FwdTbl_t table[3];
};

enum change_type
{
  NE,
  NW,
  EN,
  ES,
  SE,
  SW,
  WS,
  WN,
  UNVALID,
  single,
  dual
};

typedef struct change_table
{
    int id : 4;
    change_type type : 4;
    int time_cnt : 4;   // every 1s will discount one
    int time_send : 6;
    int time_recv : 6;
} change_table_t;

typedef struct
{
    unsigned int i : 8;
    unsigned int j : 8;
    balloc::beacon_t link;
} hello_struct;

typedef struct
{
    unsigned int dst : 4;
    unsigned int src : 4;
    unsigned int num : 8;
} mac_head;

typedef struct
{
    unsigned int len : 32;
    unsigned char type : 8;
} sub_head;

typedef struct
{
    unsigned char src : 4;
    unsigned char pre : 4;
    unsigned char HTL : 8;
    unsigned long long seq : 64;
} broadcast_head;