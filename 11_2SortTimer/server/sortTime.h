#pragma once

#include<time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<iostream>

using namespace std;

constexpr int BUFFER_SIZE = 64;

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

class util_timer
{
public:
    util_timer():prev(nullptr),next(nullptr){}
public:
    time_t expire;
    void (*cb_func)(client_data *);
    client_data *user_data;
    util_timer *prev, *next;
};

class sort_timer_lst
{
public:
    sort_timer_lst():head(nullptr),tail(nullptr){}
    ~sort_timer_lst()
    {
        util_timer *tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    void add_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head = tail = timer;
            return;
        }
        if(timer->expire<head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }

    void addjust_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        util_timer *tmp = timer->next;
        if(!tmp||(timer->expire<tmp->expire))
        {
            return;
        }
        if(timer==head)
        {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        }
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    void del_timer(util_timer *timer)
    {
        if(!timer)
        {
            return;
        }
        if((timer==head)&&(timer==tail))
        {
            delete timer;
            head = nullptr;
            head->prev = nullptr;
            delete timer;
            return;
        }
        if(timer==head)
        {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        if(timer==tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }

        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    void tick()
    {
        if(!head)
            return;
        cout << "timer tick" << endl;

        time_t cur= time(nullptr);
        util_timer *tmp = head;
        while(tmp)
        {
            if(cur<tmp->expire)
                return;

            tmp->cb_func(tmp->user_data);
            head = tmp->next;
            if(head)
            {
                head->prev = nullptr;
            }
            delete tmp;
            tmp = head;
        }
    }

private:
    void add_timer(util_timer* timer,util_timer*lst_head)
    {
        util_timer *prev = lst_head, *tmp=prev->next;
        while(tmp)
        {
            if(timer->expire<tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev=tmp;
            tmp = tmp->next;
        }
        if(!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = nullptr;
            tail = timer;
        }
    }

private:
    util_timer *head, *tail;
};