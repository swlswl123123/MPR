#pragma once
#include <opnet.h>
#include <log.h>
#define RE 6378137.0

typedef struct
{
    double gamma;
    double alpha;
    double D;
} angle_t;

typedef struct
{
    double gmma;
    double alpha;
    int valid;
    int spare;//预留，方便下发
} geo_t;

struct nb_pos
{
    geo_t pos[10];
};

typedef class antenna antenna_t;
class antenna
{

private:
    int id;
    Objid node_ant, ants[4];
    double head;

public:
    angle_t pointing[4];
    static geo_t calc_pointing(geo_t p, angle_t t);
    antenna();
    ~antenna();
    void execute_pointing(angle_t t, int n);
    double get_head();
    void set_head(double h);
    double get_gamma(int s);
};
