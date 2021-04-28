#include<phy.hpp>

phy_ctrl::phy_ctrl()
{
    recv_direct = 0;
    obj = op_id_parent(op_id_self());
    op_ima_obj_attr_get_int32(obj, "user id", &id);
}

phy_ctrl::~phy_ctrl()
{
}

void phy_ctrl::send(int port, Packet* p)
{
    if (p != OPC_NIL)
    {
        // LOG("[%d]PHY SEND:pkid=%d",id,op_pk_tree_id(pac_to_send));
        switch (port)
        {
        case phy_ns::N:
            op_send_pk(p, phy_ns::tx_N);
            LOG("[%d]SEND N",id);
            break;
        case phy_ns::S:
            op_send_pk(p, phy_ns::tx_S);
            LOG("[%d]SEND S",id);
            break;
        case phy_ns::E:
            op_send_pk(p, phy_ns::tx_E);
            LOG("[%d]SEND E",id);
            break;
        case phy_ns::W:
            op_send_pk(p, phy_ns::tx_W);
            LOG("[%d]SEND W",id);
            break;
        default:
            break;
        }
    }
}

Packet* phy_ctrl::receive(int port, int mode) 
{
    Packet* pac_recv = op_pk_get(port);
    if (pac_recv != OPC_NIL)
    {
        // LOG("[%d]PHY RECV:pkid=%d",id,op_pk_tree_id(pac_recv));
        switch (port)
        {
        case phy_ns::rx_N:
            if (mode != 0)
            {
                pac_recv = OPC_NIL;
                recv_direct = -1;
                LOG("[%d]discard_N", id);
            }
            else
            {
                recv_direct = phy_ns::N;
                LOG("[%d]RECV N", id);
            }
            break;
        case phy_ns::rx_S:
            if (mode != 0)
            {
                pac_recv = OPC_NIL;
                recv_direct = -1;
                LOG("[%d]discard_S", id);
            }
            else
            {
                recv_direct = phy_ns::S;
                LOG("[%d]RECV S", id);
            }
            break;
        case phy_ns::rx_E:
            if (mode != 1)
            {
                pac_recv = OPC_NIL;
                recv_direct = -1;
                LOG("[%d]discard_E", id);
            }
            else
            {
                recv_direct = phy_ns::E;
                LOG("[%d]RECV E", id);
            }
            break;
        case phy_ns::rx_W:
            if (mode != 1)
            {
                pac_recv = OPC_NIL;
                recv_direct = -1;
                LOG("[%d]discard_W", id);
            }
            else
            {
                recv_direct = phy_ns::W;
                LOG("[%d]RECV W", id);
            }
            break;
        default:
            break;
        }
    }
    return pac_recv;
}

int phy_ctrl::get_id()
{
    return id;
}

int phy_ctrl::get_direct()
{
    return recv_direct;
}