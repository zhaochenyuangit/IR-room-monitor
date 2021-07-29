#pragma once
extern "C"
{
#include "stdbool.h"
#include "stdio.h"
#include <string.h>
}

class HumanObject
{
private:
    static int serial_num_;
    int label_;
    struct state_
    {
        int size;
        int pos_x;
        int pos_y;
        int vel_x;
        int vel_y;
    } last_, now_;
    int first_x_;
    int first_y_;

public:
    HumanObject(int label, int pos_x, int pos_y, int size);
    ~HumanObject();
    void update(int pos_x, int pos_y, int size);
    void ab_filter(int pos_x, int pos_y);
    void predict(int *ppos_x, int *ppos_y);
    int get_index();
    void get_size(int *size, int *last_size);
    void get_last_pos(int *lpos_x, int *lpos_y);
    void get_now_pos(int *npos_x, int *npos_y);
    int counting();
};
