#include <opnet.h>
#include <router.hpp>
#include <float.h>
#include <iostream>
#include <limits.h>
#include <deque>
#include <log.h>

router::router()
{
};
router::router(tdma_ctrl *t)
{
    this->id = t->get_id();
    memset(&FWD, 0x00, sizeof(FwdTbl_t));
    memset(load_table, 0x00, sizeof(unsigned long long));
};

router::~router(){

};

int router::calc_weight(int src)
{
    int ret = 0;
    for (int i = 0; i < 5; i++)
    {
        if (topo[src][i])
        {
            ret++;
        }
    }
    return ret;
}

void router::calc_route()
{
    vector<node> N1;
    vector<int> N2;
    for (int i = 0; i < 5; i++)
    {
        if (topo[id-1][i])
        {
            node n;
            n.nid = i;
            n.weight = calc_weight(i);
            for (int j = 0; j < 5; j++)
            {
                if (topo[i][j] && j != id-1 && topo[id-1][j] == 0)
                {
                    N2.push_back(j);
                }
            }
        }
    }
    sort(N2.begin(), N2.end());
    vector<int>::iterator ite = unique(N2.begin(), N2.end());
    N2.erase(ite, N2.end());
    // sort one hop node

    // calc MPR
}

void router::Dijkstra(const node &from, int type)
{
    int N = vertice.size();
    LOG("vertice:%d", N);
    node **vs = new node *[N];
    bool *visited = new bool[N];
    int *prev = new int[N];
    double *dist = new double[N];
    int *hops = new int[N];
    double **weights = new double *[N];
    int *fwd = new int[N];
    bool *MPR = new bool[N];
    int *coverage = new int[N];

    for (int i = 0; i < N; i++)
    {
        weights[i] = new double[N];
        coverage[i] = 0;
    }

    int i = 0;
    int src = -1;
    for (auto v = vertice.begin(); v != vertice.end(); v++, i++)
    {
        vs[i] = (node *)&(*v);
        prev[i] = -1;
        dist[i] = DBL_MAX;
        visited[i] = false;
        hops[i] = 65535;
        MPR[i] = false;

        if (*v == from)
        {
            visited[i] = true;
            dist[i] = 0;
            hops[i] = 0;
            prev[i] = i;
            src = i;
        }
    }

    // printf("W = [\n");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (j == i)
            {
                weights[i][j] = 0;
            }
            else
            {
                switch (type)
                {
                case 0:
                    weights[i][j] = get_weight_all(*vs[i], *vs[j]);             
                    break;
                case 1:
                    weights[i][j] = get_weight_front(*vs[i], *vs[j]);
                    break;
                case 2:
                    weights[i][j] = get_weight_back(*vs[i], *vs[j]);
                    break;
                default:
                    break;
                }
            }
            printf("weights[%d][%d] = %3.2lf\n",vs[i]->get_id(),vs[j]->get_id(),weights[i][j]);
        }
        // printf("\n");
    }
    // printf("]\n");

    int cnt_visited = 0;
    int cnt_visited_tmp = cnt_visited;
    do
    {
        cnt_visited_tmp = cnt_visited;
        for (int n = 0; n < N; n++)
        {
            if (!visited[n])
                continue;

            for (int m = 0; m < N; m++)
            {
                if (weights[n][m] != DBL_MAX && !visited[m])
                {
                    if (dist[n] + weights[n][m] < dist[m])
                    {
                        prev[m] = n;
                        dist[m] = dist[n] + weights[n][m];
                    }

                    if (hops[n] + 1 < hops[m])
                    {

                        hops[m] = hops[n] + 1;
                    }

                    visited[m] = true;
                    cnt_visited++;
                }
            }
        }

    } while (cnt_visited != cnt_visited_tmp);

    //Update forwarding table
    switch (type)
    {
    case 0:
        forward_table_all.clear();
        break;
    case 1:
        forward_table_front.clear();
        break;
    case 2:
        forward_table_back.clear();
        break;        
    default:
        break;
    }
    

    //Initialize forward table
    for (int i = 0; i < N; i++)
    {
        fwd[i] = -1;
    }

    //Loop over nodes
    for (int i = 0; i < N; i++)
    {
        //Exclude unreachable nodes, source node & already computed nodes
        if (i == src || hops[i] == 65535 || fwd[i] != -1)
            continue;

        std::deque<int> q;
        int j = i;

        do
        {
            q.push_back(j);
            j = prev[j];
        } while (j != src);

        while (q.size() > 0)
        {
            fwd[q.front()] = q.back();
            q.pop_front();
        }
    }

    //Put the result into map
    for (int i = 0; i < N; i++)
    {
        if (i == src || fwd[i] == -1)
            continue;
        switch (type)
        {
        case 0:
        {
            forward_table_all.insert(std::make_pair(*vs[i], *vs[fwd[i]]));
            printf("ALL:[%d]->[%d] via:%d R = %lf H = %d\n", from.get_id(), vs[i]->get_id(), vs[fwd[i]]->get_id(), dist[i], hops[i]);
            break;
        }
        case 1:
        {
            forward_table_front.insert(std::make_pair(*vs[i], *vs[fwd[i]]));
            printf("front:[%d]->[%d] via:%d R = %lf H = %d\n", from.get_id(), vs[i]->get_id(), vs[fwd[i]]->get_id(), dist[i], hops[i]);
            break;
        }
        case 2:
        {
            forward_table_back.insert(std::make_pair(*vs[i], *vs[fwd[i]]));
            printf("back:[%d]->[%d] via:%d R = %lf H = %d\n", from.get_id(), vs[i]->get_id(), vs[fwd[i]]->get_id(), dist[i], hops[i]);
            break;
        }
        default:
            break;
        }
        //reset for MPR calc
        visited[i] = false;
    }

    // mpr_nodes.clear();

    // for (int i = 0; i < N; i++)
    //     visited[i] = false;

    // for (int i = 0; i < N; i++)
    // {
    //     if (hops[i] != 2)
    //         continue;

    //     int last_index = 0;
    //     int cover = 0;
    //     for (int j = 0; j < N; j++)
    //     {
    //         if (hops[j] != 1)
    //             continue;
    //         if (weights[j][i] < DBL_MAX)
    //         {
    //             cover++;
    //             last_index = j;
    //         }
    //     }

    //     if (cover == 1)
    //     {
    //         MPR[last_index] = true;
    //         for (int j = 0; j < N; j++)
    //         {
    //             if (hops[j] != 2)
    //                 continue;
    //             if (weights[last_index][j] < DBL_MAX)
    //             {
    //                 visited[j] = true;
    //             }
    //         }
    //     }
    // }

    // do
    // {

    //     std::priority_queue<std::pair<int, int>> q;
    //     for (int i = 0; i < N; i++)
    //     {
    //         coverage[i] = 0;
    //     }
    //     //Calculate MPR
    //     for (int i = 0; i < N; i++)
    //     {
    //         if (hops[i] != 1 || MPR[i])
    //             continue;

    //         for (int j = 0; j < N; j++)
    //         {
    //             if (hops[j] != 2)
    //                 continue;

    //             if (weights[i][j] < DBL_MAX && visited[j] == false)
    //             {
    //                 coverage[i]++;
    //             }
    //         }

    //         if (coverage[i] != 0)
    //         {
    //             q.push(std::pair<int, int>(i, coverage[i]));
    //         }
    //     }

    //     if (q.size() > 0)
    //     {
    //         MPR[q.top().first] = true;
    //         for (int i = 0; i < N; i++)
    //         {
    //             if (hops[i] != 2)
    //                 continue;
    //             if (weights[q.top().first][i] < DBL_MAX)
    //             {
    //                 visited[i] = true;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         break;
    //     }

    // } while (1);

    // for (int i = 0; i < N; i++)
    // {
    //     if (MPR[i])
    //     {
    //         mpr_nodes.insert(*vs[i]);
    //     }
    // }

    //Resource dealloc
    delete[] vs;
    delete[] prev;
    delete[] dist;
    delete[] visited;
    delete[] hops;
    delete[] weights;
    delete[] fwd;
    delete[] MPR;
}

FwdTblAll_t router::get_forward_table() 
{
    memset(&FWD, 0x00, sizeof(FWD));
    for (auto i = forward_table_front.begin(); i != forward_table_front.end(); i++)
    {
        if (i->first.get_id() == i->second.get_id())
        {
            FWD.table[0].link_table[i->first.get_id()-1].PRIMARY = 1;
        }
        FWD.table[0].link_table[i->first.get_id()-1].id = i->second.get_id();
    }
    for (auto i = forward_table_back.begin(); i != forward_table_back.end(); i++)
    {
        if (i->first.get_id() == i->second.get_id())
        {
            FWD.table[1].link_table[i->first.get_id()-1].PRIMARY = 1;
        }
        FWD.table[1].link_table[i->first.get_id()-1].id = i->second.get_id();
    }
    for (auto i = forward_table_all.begin(); i != forward_table_all.end(); i++)
    {
        if (i->first.get_id() == i->second.get_id())
        {
            FWD.table[2].link_table[i->first.get_id()-1].PRIMARY = 1;
        }
        FWD.table[2].link_table[i->first.get_id()-1].id = i->second.get_id();
    }
    return FWD;    
}

std::pair<node, bool> router::get_vertex(int id) const
{
    auto v = vertice.find(node(id));
    std::pair<node, bool> ret;
    if (v == vertice.end())
    {
        // printf("router:get_vertex(%d) = NOTFOUND!\n",id);
        for (auto u = vertice.begin(); u != vertice.end(); u++)
        {
            // printf("vertices = %d\n", u->get_id());
        }
        ret.second = false;
    }
    else
    {
        ret.second = true;
        // printf("router:get_vertex(%d) = FOUND!\n",id);
        ret.first = *v;
    }
    return ret;    
}

int router::get_vertice_num()
{
    return vertice.size();
}