#include <app.hpp>
#include <log.h>

using namespace app_ns;

traffic::traffic()
{
}

traffic::~traffic()
{
}

app::app(int id)
{
    this->id = id;
}

app::~app()
{
}

bool traffic::operator==(const traffic_t &t) const
{
    return this->time == t.time;
}
bool traffic::operator>(const traffic_t &t) const
{
    return this->time < t.time;
}

bool traffic::operator<(const traffic_t &t) const
{
    return this->time > t.time;
}

void app::on_pk_recv(Packet* p)
{
    double delay = op_sim_time() - op_pk_creation_time_get(p);
    int pkid = 0;
    op_pk_fd_get_int32(p, 0, &pkid);
    LOG("[%d]TRAFFIC:[%d] DELAY = %lf", id, pkid, delay);
    op_pk_destroy(p);    
}

Packet* app::on_pk_send(int inter_vector)
{
    int index = inter_vector & 0x00ffffff;
    bool urg = inter_vector & 0x00400000;
    int dst = (inter_vector & 0x003f0000) >> 16;
    int pkid = (inter_vector & 0x0000ffff);
    Packet *p = op_pk_create(12000);
    op_pk_fd_set_int32(p, 0, pkid, 32);
    LOG("[%d]TRAFFIC:SEND[%d] to [%d] URG %d", id, pkid, dst,urg);
    return p;    
}

void app::generate_pk()
{
    FILE *p = fopen("traffic.csv", "r");
    traffic_t t;
    bool flag = 1;
    while (fscanf(p, "%d,%d,%d,%lf,%d,%d", &t.id, &t.src, &t.dst, &t.time, &t.urg, &t.size) == 6)
    {
        if (t.src != this->id)
            continue;
        this->traffics.push_back(t);
        // generate urgent packets pre information
        if(t.urg == 1 && flag)
        {
            op_intrpt_schedule_self(t.time - 0.2, URG_PRE | (t.dst) << 16);
            flag = 0;
        }
        // the end of urgent packets
        if (t.size == 0)
        {
            op_intrpt_schedule_self(t.time + 0.05, URG_END | (t.dst) << 16);
            // flag = 1;
        }
        op_intrpt_schedule_self(t.time, SEND | (t.dst) << 16 | t.id | t.urg << 22);
    }
    fclose(p);    
}

// void app::proc(int cond)
// {
//     switch (cond)
//     {
//     case ON_RECV:
//     {
//         Packet *p = op_pk_get(op_intrpt_strm());
//         if (p == OPC_NIL)
//             break;
//         double delay = op_sim_time() - op_pk_creation_time_get(p);
//         int pkid = 0;
//         op_pk_fd_get_int32(p, 0, &pkid);
//         LOG("[%d]TRAFFIC:[%d] DELAY = %lf", id, pkid, delay);
//         op_pk_destroy(p);
//     }
//     break;
//     case ON_SEND:
//     {
//         int index = op_intrpt_code() & 0x00ffffff;
//         bool urg = op_intrpt_code() & 0x00400000;
//         int dst = (op_intrpt_code() & 0x003f0000) >> 16;
//         int pkid = (op_intrpt_code() & 0x0000ffff);

//         Packet *p = op_pk_create(12000);
//         op_pk_fd_set_int32(p, 0, pkid, 32);
//         Packet *q = op_pk_create(0);
//         op_pk_fd_set_int32(q, 0, dst, 8);
//         op_pk_fd_set_pkt(q, 1, p, op_pk_total_size_get(p));
//         if (urg)
//         {
//             LOG("[%d]TRAFFIC:SEND[%d] to [%d] URG %d", id, pkid, dst,urg);
//             op_pk_send(q, DATA_URG_DL);
//         }
//         else
//         {
//             LOG("[%d]TRAFFIC:SEND[%d] to [%d] URG %d", id, pkid, dst,urg);
//             op_pk_send(q, DATA_DL);
//         }
//     }
//     break;
//     case ON_URG_PRE:
//     {
//         LOG("ON_URG_PRE");
//         int dst = (op_intrpt_code() & 0x003f0000) >> 16;
//         Packet *q = op_pk_create(0);
//         op_pk_fd_set_int32(q, 0, dst, 8);
//         op_pk_fd_set_int32(q, 1, 0, 8);
//         op_pk_fd_set_int32(q, 2, 0, 8);
//         op_pk_send(q, DATA_URG_PRE);
//     }
//     break;

//     case ON_URG_END:
//     {
//         int dst = (op_intrpt_code() & 0x003f0000) >> 16;
//         Packet *q = op_pk_create(0);
//         op_pk_fd_set_int32(q, 0, dst, 8);
//         op_pk_fd_set_int32(q, 1, 0, 8);
//         op_pk_fd_set_int32(q, 2, 1, 8);
//         op_pk_send(q, DATA_URG_PRE);
//     }
//     break;
//     case ON_BEGSIM:
//     {
//         FILE *p = fopen("traffic.csv", "r");

//         traffic_t t;
//         bool flag = 1;
//         while (fscanf(p, "%d,%d,%d,%lf,%d,%d", &t.id, &t.src, &t.dst, &t.time, &t.urg, &t.size) == 6)
//         {
//             if (t.src != this->id)
//                 continue;
//             this->traffics.push_back(t);

//             if(t.urg == 1 && flag)
//             {
//                 op_intrpt_schedule_self(t.time - 0.2, URG_PRE | (t.dst) << 16);
//                 flag = 0;
//             }
//             if (t.size == 0)
//             {
//                 op_intrpt_schedule_self(t.time + 0.05, URG_END | (t.dst) << 16);
//                 // flag = 1;
//             }
//             op_intrpt_schedule_self(t.time, SEND | (t.dst) << 16 | t.id | t.urg << 22);
//         }
//         fclose(p);
//     }
//     break;
//     case ON_ENDSIM:
//         break;
//     }
// }