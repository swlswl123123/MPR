#pragma once
#include <vector>
#include <opnet.h>

namespace app_ns
{

typedef enum strm
{
    DATA_UL,
    DATA_DL,
    DATA_URG_UL,
    DATA_URG_DL,
    DATA_URG_PRE
} strm_t;

typedef enum intrpt
{
    ON_RECV,
    ON_SEND,
    ON_URG_PRE,
    ON_URG_END,
    ON_BEGSIM,
    ON_ENDSIM
} intrpt_t;

typedef enum code
{
    SEND = 0x01000000,
    URG_PRE = 0x02000000,
    URG_END = 0x04000000
} code_t;

typedef class traffic traffic_t;
class traffic
{
public:
    int id;
    int src;
    int dst;
    double time;
    int size;
    traffic();
    ~traffic();
    bool urg;
    bool operator==(const traffic_t &t) const;
    bool operator>(const traffic_t &t) const;
    bool operator<(const traffic_t &t) const;
};

class app
{

private:
    std::vector<traffic> traffics;
    int id;
public:
    app(int id);
    ~app();
    // void proc(int proc);
    void on_pk_recv(Packet* p);
    Packet* on_pk_send(int inter_vector);
    void generate_pk();
};

} // namespace app_ns
