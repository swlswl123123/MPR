/**
 * @file opnet_controller.hpp
 * @author Victor LI (leevick@tsinghua.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2021-01-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <tdma.hpp>
#include <router.hpp>
#include <app.hpp>
#include <log.h>

namespace opnet
{
    class opnet_controller
    {
    private:
        tdma_ctrl *tdma_ptr;
        router* router_ptr;
        app_ns::app* app_ptr;

    public:
        opnet_controller();
        void recv_proc();
        void on_sim_start();
        void on_irq(int id);
        void on_stream(int id);
        void on_stat(int id);
        void on_sim_stop();
        ~opnet_controller();
    };
}; // namespace opnet
