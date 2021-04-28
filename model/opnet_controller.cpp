#include <string>
#include <iostream>
#include <opnet_controller.hpp>
#include <opnet.h>

using namespace std;
using namespace opnet;

void opnet_controller::recv_proc()
{
	mac_head mh;
	memcpy(&mh, rx_buf, sizeof(mac_head));
	int num = mh.num;
	int offset = sizeof(mac_head);
	while (num > 0)
	{
		sub_head sh;
		memcpy(&sh, rx_buf + offset, sizeof(mac_head));
		offset += sizeof(mac_head);
		pkt p;
		p.src = mh.src;
		p.dst = mh.dst;
		p.type = sh.type;
		tdma_ptr->type2prio(p);
		p.len = sh.len;
		p.p = (unsigned char *)malloc(sh.len);
		memcpy(p.p, rx_buf + offset, sh.len);
		offset += sh.len;
		if (p.type != ETHERTYPE_ARP)
		{
			if (p.dst == 0xf)
			{
				printf("[%d]recv_proc:BROADCAST PACKET\n", phy_ptr->get_id());
				tdma_ptr->proc_broadcast(p); // revise q, copy to broadcast, send to all neighbors except prev
			}
			else if (p.dst != phy_ptr->get_id())
			{
				tdma_ptr->proc_unicast(p);// q remain, copy to relay, if dst is myself, destroy copy
			}
		}

		if (p.dst == 0xf)
		{
			if (p.type == ETHERTYPE_ARP)
			{
				printf("[%d]recv ND to self\n", phy_ptr->get_id());
			}
			else
			{
				// parse broadcast head
				broadcast_head bh;
				memcpy(&bh, p.p, sizeof(broadcast_head));
				if (bh.seq <= tdma_ptr->seq_num[bh.src-1])
				{
					free(p.p);
					p.p = nullptr;
				}
			}
		}
		else
		{
			if (p.dst == phy_ptr->get_id())
			{
				printf("[%d]recv pkt to self\n", phy_ptr->get_id());
			}
			else
			{
				free(p.p);
				p.p = nullptr;
			}
		}
		
		if (p.p == nullptr)
		{
			continue;
		}
		switch (p.type)
		{
		case ETHERTYPE_ARP:
		{
			allocator_ptr->proc_nd(p, phy_ptr->get_direct());
		}
		break;
		case EHTERTYPE_POS:
		{
			tdma_ptr->set_position_recv(p);
		}
		break;
		case EHTERTYPE_BALLOC:
		{
			// send to allocator to allocate slots
			allocator_ptr->proc_balloc(p);
		}
		case EHTERTYPE_RINFO:
		{
			// send to rrc to update link
			allocator_ptr->proc_link_update(p);
		}
		case EHTERTYPE_CHANGE_B:
		{
			// send to rrc to update link
			allocator_ptr->proc_change_update(p);
		}
		break;
		case EHTERTYPE_URG_PRE:
		{
			printf("RECV:EHTERTYPE_URG_PRE\n");
			allocator_ptr->handle_urg_pre(p);
		}
		break;
		case EHTERTYPE_URG_END:
		{
			printf("RECV:EHTERTYPE_URG_END\n");
			allocator_ptr->handle_urg_end(p);
		}
		break;
		case EHTERTYPE_CHANGE:
		{
			printf("RECV:EHTERTYPE_CHANGE\n");
			tdma_ptr->update_change_table(p);
		}
		break;		
		default:
			break;
		}
		num--;
	}
}

opnet_controller::opnet_controller()
{
    // cout << "opnet_controller" << endl;
    phy_ptr = new phy_ctrl();
    tdma_ptr = new tdma_ctrl(phy_ptr);
	router_ptr = new router(phy_ptr->get_id());
	// allocator_ptr = new balloc::allocator(phy_ptr->get_id(), router_ptr);
	allocator_ptr = new balloc::allocator(phy_ptr->get_id(), tdma_ptr, router_ptr);
	app_ptr = new app_ns::app(phy_ptr->get_id());
}

void opnet_controller::init(void *table, void* fwd, void *ant)
{
	tdma_ptr->init(table, fwd, ant);
}

void opnet_controller::on_sim_start()
{
    // cout << "on_sim_start" << endl;

    // void *cm_ctrl = dma.register_map(0x00000000, 8192);
    // void *cm_tbl = dma.register_map(0x00004000, 8192);
    // void *tdma_creg = dma.register_map(0x00010000, 256);
    // void *tdma_hello = dma.register_map(0x00010100, 256);
    // void *tdma_table = dma.register_map(0x00010200, 512);
    // void *tdma_msg = dma.register_map(0x00010400, 1024);

    // cm.init(cm_ctrl, cm_tbl);
    // cm_hdl.init(cm_ctrl, cm_tbl);

    // tdma_hdl.init(tdma_creg, tdma_table, tdma_msg, tdma_hello);
    // map<unsigned int, unsigned int> table;
    // for (int i = 0; i < 20; i++)
    // {
    //     for (int j = 0; j < 50; j++)
    //     {
    //         table.insert(make_pair(j * 200000 + 50000000 * i, j + 1));
    //     }
    //     table.insert(make_pair(50000000 * i + 30000000, IRQ_FRAME));
    // }
    // table.insert(make_pair(990000000, IRQ_CYCLE));

    // //Register interrupt handler
    // irq.register_interrupt(0, bind(&cm_impl::schedule_interrupt, &cm_hdl, placeholders::_1));

    // for (int i = 0; i < 50; i++)
    // {
    //     irq.register_interrupt(i + 1, bind(&tdma_impl::handle_intrpt, &tdma_hdl, placeholders::_1));
    // }

    // //CM start
    // cm.start(1000000000, table, static_cast<unsigned int>(op_dist_uniform(1) * (0x01u << 31)));
    // cm_hdl.schedule_initial();
	op_intrpt_schedule_self(op_sim_time(), slot_ns::C); 
	op_intrpt_schedule_self(op_sim_time() + 5, slot_ns::B); 
	app_ptr->generate_pk();
}
void opnet_controller::on_irq(int id)
{
    // cout << "@" << op_sim_time() << " on_irq " << id << endl;
    // irq.handle_irq(id);
	switch (id & 0xff000000)
	{
	case app_ns::SEND:
	{
		bool urg = id & 0x00400000;
		int dst = (id & 0x003f0000) >> 16;
		if (urg)
		{
			tdma_ptr->proc_unicast(tdma_ptr->encap_mac(app_ptr->on_pk_send(id), dst, mac_ns::UDUCH, ETHERTYPE_IP));
		}
		else
		{
			tdma_ptr->proc_unicast(tdma_ptr->encap_mac(app_ptr->on_pk_send(id), dst, mac_ns::DUCH, ETHERTYPE_IP));
		}
	}
	break;
	default:
	{
		if (id == slot_ns::C)
		{
			// statistic txq and balloc
			allocator_ptr->req_ext_slot(tdma_ptr->get_txq());
		}
		
		if (id == slot_ns::B) // broadcast interrupt
		{
			tdma_ptr->proc_broadcast(tdma_ptr->encap_mac(tdma_ptr->encap_broadcast(allocator_ptr->send_link_broadcast()), 0x3f, mac_ns::CUCH, EHTERTYPE_RINFO));
			op_intrpt_schedule_self(op_sim_time() + 5, slot_ns::B);
		}
		else // slot control
		{
			tdma_ptr->slot_ctrl(id);	
		}
	}
	break;
	}
}
void opnet_controller::on_stream(int d)
{   
    Packet* p = phy_ptr->receive(d, tdma_ptr->get_status());
    // packets parse
    // add recv direction
    recv_proc(p);
}

void opnet_controller::on_stat(int id)
{
    cout << "on_stat " << id << endl;
    cout << "rcvd snr = " << op_stat_read(id) << endl;
}

void opnet_controller::on_sim_stop()
{
    cout << "on_sim_stop" << endl;
	// print beacon table
	tdma_ptr->print_beacon_table();
	// print forward table
	tdma_ptr->print_forward_table();
	LOG("[%d]vertice:%d", phy_ptr->get_id(), router_ptr->get_vertice_num());
}

opnet_controller::~opnet_controller()
{
    cout << "opnet_controller" << endl;
}