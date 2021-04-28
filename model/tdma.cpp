#include <tdma.hpp>
#include <log.h>
#include <string.h>
#include <opnet.h>
#include <math.h>

tdma_ctrl::tdma_ctrl(/* args */)
{
    obj = op_id_parent(op_id_self());
    op_ima_obj_attr_get_int32(obj, "user id", &id);
    // memset(load_table, 0x00, sizeof(unsigned long long));
    memset(&forward_table, 0x00, sizeof(FwdTblAll_t));
    memset(&txq, 0x00, sizeof(transfer_queue));
}


tdma_ctrl::~tdma_ctrl()
{
}

void tdma_ctrl::slot_ctrl(int type)
{
    switch (type & 0xf00)
    {
    case C:
    {
        op_intrpt_schedule_self(op_sim_time() + FRAME_INTERVAL, C);
        for (int i = 0; i < 50; i++)
        {
            op_intrpt_schedule_self(op_sim_time() + i*D_INTERVAL, D|i);
        }
    }
    break;
    case D & 0xf00:
    {
        int d = type & 0x0ff;
        if (d == id - 1)
        {
           send_proc(); 
        }
    }
    break;
    default:
        break;
    }
}

void tdma_ctrl::send_proc()
{
    LOG("[%d]sending", id);
    Packet *p = op_pk_create(0);
    unsigned int count = 32;
    int i =  1;
    op_pk_fd_set_int32(p, 0, 0, 32);
    while (txq.size() > 0)
    {
        Packet *q = txq.top();
        if ((count += op_pk_total_size_get(q)) > BIT_PER_UNIT)
        {
            break;
        }
        int src, dst, next_hop, type;
        op_pk_fd_get_int32(q, 0, &dst);
        op_pk_fd_get_int32(q, 1, &src);
        op_pk_fd_get_int32(q, 2, &next_hop);
        op_pk_fd_get_int32(q, 4, &type);
        txq.pop();
        int pkid = 0;
        if (type == ETHERTYPE_IP)
        {
            Packet *r = OPC_NIL;
            op_pk_fd_get_pkt(q, 5, &r);
            op_pk_fd_get_int32(r, 0, &pkid);
            LOG("[%d]TX[%d]:[%d]->[%d] via [%d]", id, pkid, src, dst, next_hop);
            op_pk_fd_set_pkt(q, 5, r, op_pk_total_size_get(r));
        }
        else
        {
            LOG("[%d]TX:[%d]->[%d] via [%d] type:%x", id, src, dst, next_hop, type);
        }
        i++;
    }
    if (i > 1)
    {
        op_pk_fd_set_int32(p, 0, i - 1, 32);
        op_send_pk(p, tx);
        LOG("[%d]SEND", id);
    }
}

int tdma_ctrl::proc_broadcast(Packet* p)
{
    int src, dst, type, htl, ctrl, prev, prio;
    unsigned long long seq;
    Packet *q = OPC_NIL, *r = OPC_NIL, *s = OPC_NIL;
    if (p == OPC_NIL)
        return;
    // LOG("on_proc_broadcast");
    op_pk_fd_get_int32(p, 0, &dst);
    op_pk_fd_get_int32(p, 1, &src);
    op_pk_fd_get_int32(p, 3, &prio);
    op_pk_fd_get_int32(p, 4, &type);
    op_pk_fd_get_pkt(p, 5, &q);
    op_pk_fd_get_int32(q, 0, &prev);
    op_pk_fd_get_int32(q, 1, &htl);
    op_pk_fd_get_int64(q, 2, (long long *)&seq);

    LOG("[%d]proc_broadcast:dst = %d src = %d prev = %d type = %d ctrl = %d htl = %d seq = 0x%llx seq_num[src - 1]=0x%lx", id, dst, src, prev, type, ctrl, htl, seq, seq_num[src - 1]);
    if (htl == 0 || seq <= seq_num[src - 1])
    {
        op_pk_fd_set_pkt(p, 5, q, op_pk_total_size_get(q));
        LOG("[%d]broadcast seq timeout",id);
        return;
    }

    seq_num[src - 1] = seq;
    LOG("[%d]proc_broadcast: seq_num[%d]update to 0x%llx", id, src-1, seq_num[src - 1]);


    op_pk_fd_set_int32(q, 1, htl - 1, 32);

    for (int i = 0; i < 5; i++)
	{
		LOG("[%d]forward_table[%d].PRIMARY = %d id:%d", id, i, forward_table.link_table[i].PRIMARY, forward_table.link_table[i].id);
		if (!forward_table.link_table[i].PRIMARY || i == prev - 1)
			continue;

		op_pk_fd_set_int32(q, 0, id, 32);
		r = op_pk_copy(q);
		s = op_pk_copy(p);
		op_pk_fd_set_pkt(s, 5, r, op_pk_total_size_get(q));
        txq.push(s);
		LOG("[%d]BROADCAST PUSH", id);
	}
	op_pk_fd_set_pkt(p, 5, q, op_pk_total_size_get(q));
}

void tdma_ctrl::proc_unicast(Packet* q)
{
    int src, dst, next_hop;
    ptype_t type;
	Packet *p = OPC_NIL, *r = OPC_NIL, *s = OPC_NIL;
	if (q == OPC_NIL)
		return;
	p = op_pk_copy(q);
	op_pk_fd_get_int32(p, 0, &dst);
	op_pk_fd_get_int32(p, 1, &src);
	op_pk_fd_get_int32(p, 2, &next_hop);
	op_pk_fd_get_int32(p, 4, (int *)&type);

    for (int i = 0; i < 5; i++)
    {
        LOG("[%d]fwd[%d] = %d", id, i,forward_table.link_table[i].id); // to calculate route number
    }
    // // LOG("[%d]debug:%d", id, dst);

	unsigned int fwd = forward_table.link_table[dst - 1].id;
    LOG("[%d]:pkt from [%d] to [%d],fwd=%d,type=0x%x", id, src, dst, fwd, type);
	
   if (fwd != 0 && type == ETHERTYPE_IP)
	{
		int pkid = 0;
		op_pk_fd_get_pkt(p, 5, &r);
		op_pk_fd_get_int32(r, 0, &pkid);
		op_pk_fd_set_pkt(p, 5, r, op_pk_total_size_get(r));
        op_pk_fd_set_int32(p, 2, fwd, 32);
		LOG("[%d][%d]RELAY:[%d]->[%d] via [%d]", id, pkid, src, dst, fwd);
	}
    else if (type == ETHERTYPE_IP)
	{
		int pkid = 0;
        op_pk_nfd_get_pkt(p, "data", &r);
		op_pk_fd_get_int32(r, 0, &pkid);
		LOG("[%d][%d]RELAY FAILED:[%d]->[%d] via [%d]", id, pkid, src, dst, fwd);
	}

	if (fwd != 0)
	{
        txq.push(p);
	}
	else
	{
		op_pk_destroy(p);
		LOG("DEBUG:DESTORY");
	}
}

Packet *tdma_ctrl::encap_mac(Packet *p, int dst, int prio, int type)
{
    if (p == OPC_NIL)
    {
        return p;
    }
    
    Packet *r = op_pk_create(0);
    op_pk_fd_set_int32(r, 0, dst, 32);
    op_pk_fd_set_int32(r, 1, id, 32);
    op_pk_fd_set_int32(r, 3, prio, 32);
    op_pk_fd_set_int32(r, 4, type, 32);
    op_pk_fd_set_pkt(r, 5, p, op_pk_total_size_get(p));
    return r;
}

Packet *tdma_ctrl::encap_broadcast(Packet *p)
{
    my_seq_num++; // increase sequence number
    Packet *q = op_pk_create(0);
    op_pk_fd_set_int32(q, 0, id, 32);
    op_pk_fd_set_int32(q, 1, 12, 32);
    op_pk_fd_set_int64(q, 2, my_seq_num, 64);
    op_pk_fd_set_pkt(q, 3, p, op_pk_total_size_get(p));
    LOG("ON_encap_broadcast");
    return q;
}

void tdma_ctrl::update_forward_table(FwdTbl_t F)
{
    forward_table = F;
}

unsigned long long tdma_ctrl::get_txq()
{
    return txq.calc_size();
}

int tdma_ctrl::get_id()
{
    return id;
}
