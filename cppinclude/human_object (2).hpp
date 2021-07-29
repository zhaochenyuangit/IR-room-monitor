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
    } last_, now_, vrtl_;
    int virtual_age_;
    int first_x_;
    int first_y_;
    
public:
    HumanObject(int label, int pos_x, int pos_y, int size);
    ~HumanObject();
    void update(int pos_x, int pos_y, int size);
    void virtual_update(int pos_x, int pos_y, int size);
    void virtual_propagation();
    void get_virtual_pos(int *vrtl_predict_pos_x, int *vrtl_predict_pos_y);
    int get_virtual_age();
    void ab_filter(int pos_x, int pos_y);
    void predict(int *predict_pos_x, int *predict_pos_y);
    void get_last_predict(int *last_pred_pos_x, int *last_pred_pos_y);
    int get_index();
    void get_size(int *size, int *last_size);
    void get_last_pos(int *last_pos_x, int *last_pos_y);
    void get_now_pos(int *now_pos_x, int *now_pos_y);
    int counting();
};