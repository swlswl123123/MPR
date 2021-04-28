#include "antenna.hpp"
// #include <opnet.h>
// #include <vos.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

antenna::antenna()
{
    char name[256];
    node_ant = op_id_parent(op_id_self());
    op_ima_obj_attr_get_int32(node_ant, "user id", &id);
    std::cout << "debug_id" << id << std::endl;
    op_ima_obj_attr_get_dbl(node_ant, "DMAC.init_head", &head);
    std::cout << "debug_head" << head << std::endl;
    for (int i = 0; i < op_topo_child_count(node_ant, OPC_OBJTYPE_ANT); i++)
    {
        int arid = 0;
        op_ima_obj_attr_get_str(op_topo_child(node_ant, OPC_OBJTYPE_ANT, i), "name", 256, name);
        sscanf(name, "a_%d", &arid);
        ants[arid] = op_topo_child(node_ant, OPC_OBJTYPE_ANT, i);
    }
    memset(pointing, 0x00, sizeof(pointing));
}
antenna::~antenna() {}
geo_t antenna::calc_pointing(geo_t p, angle_t t)
{
    double gamma = t.gamma / 180 * M_PI;
    double alpha = t.alpha / 180 * M_PI;
    double D = t.D;
    double lambda = p.lon / 180 * M_PI;
    double phi = p.lat / 180 * M_PI;
    double h = p.alt;

    double y1 = 0;
    double y2 = 0;
    double y3 = 0;

    y1 = cos(lambda) * cos(phi) * (RE + h) + D * cos(lambda) * cos(phi) * sin(alpha) - D * cos(alpha) * sin(gamma) * sin(lambda) - D * cos(alpha) * cos(gamma) * cos(lambda) * sin(phi);
    y2 = cos(phi) * sin(lambda) * (RE + h) + D * cos(alpha) * cos(lambda) * sin(gamma) + D * cos(phi) * sin(alpha) * sin(lambda) - D * cos(alpha) * cos(gamma) * sin(lambda) * sin(phi);
    y3 = sin(phi) * (RE + h) + D * sin(alpha) * sin(phi) + D * cos(alpha) * cos(gamma) * cos(phi);
    double y = sqrt(y1 * y1 + y2 * y2 + y3 * y3);

    geo_t ret;
    double alt = y - RE;
    double lat = asin(y3 / y);
    double lon = 0;

    if (y1 == 0 && y2 == 0)
    {
        lon = 0;
    }
    else
    {
        if (y2 > 0)
        {
            lon = acos(y1 / (y * cos(lat)));
        }
        else
        {
            lon = -acos(y1 / (y * cos(lat)));
        }
    }

    ret.lat = lat / M_PI * 180;
    ret.lon = lon / M_PI * 180;
    ret.alt = alt;
    // printf("y = (%lf,%lf,%lf)\n",y1,y2,y3);
    // printf("geo = (%lf,%lf,%lf)\n",ret.lat,ret.lon,ret.alt);
    return ret;
}

void antenna::execute_pointing(angle_t t, int n)
{
    // LOG("[%d]exe_pointing[%d]:%f %f",id,n,t.gamma,t.alpha);    
    if (op_ima_obj_attr_exists(ants[n], "target altitude") && op_ima_obj_attr_exists(ants[n], "target latitude") && op_ima_obj_attr_exists(ants[n], "target longitude"))
    {
        op_ima_obj_attr_set_dbl(ants[n], "target latitude", t.alpha);
        op_ima_obj_attr_set_dbl(ants[n], "target longitude", t.gamma - 180);//orientation angle -180°-180°
        op_ima_obj_attr_set_dbl(ants[n], "target altitude", 0);
        pointing[n].alpha = t.alpha;
        pointing[n].gamma = t.gamma;
        pointing[n].D = t.D;
        // LOG("[%d]ant[%d]:%f",id,n,t.gamma);
    }
    else
    {
        LOG("attributes not exits!!");
    }
}
double antenna::get_head()
{
    return head;
}
void antenna::set_head(double h)
{
    head=h;
}
double antenna::get_gamma(int s)
{
    return pointing[s].gamma;
}
