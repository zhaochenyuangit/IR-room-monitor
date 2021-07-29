#pragma once
#include "human_object.hpp"
extern "C"
{
#include "feature_extraction.h"
}

struct ObjectNode
{
    HumanObject *ob;
    ObjectNode *next;
};

class ObjectList
{
private:
    ObjectNode *head;
    ObjectNode *tail;
    ObjectNode *p;
    int count;

private:
    int match_centroid(HumanObject *ob, Blob *blob_list, int n_blobs);
    int virtual_match_centroid(HumanObject *ob, Blob *blob_list, int n_blobs);
    int match_centrals(HumanObject *ob, Blob *blob_list, int n_blobs);

    bool append_object(HumanObject *ob);
    bool delete_object_by_label(int label);
    bool blob_registration(Blob *blob);

public:
    ObjectList();
    ~ObjectList();

    ObjectNode *get_head_node();
    int get_count();
    void reset_count();

    void matching(Blob *blob_list, int n_blobs);
    bool count_and_delete_every_objects();
};

typedef int (ObjectList::*match_fun)(HumanObject *, Blob *, int);