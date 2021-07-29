#include "tracking.hpp"
#include "image_size.h"

static inline int distance_l1(int x1, int x2, int y1, int y2)
{
    return (abs(x1 - x2) + abs(y1 - y2));
}

ObjectList::ObjectList()
{
    printf("init tracking...\n");
    count = 0;
    ObjectNode *node = new ObjectNode;
    head = node;
    head->ob = NULL;
    head->next = NULL;
    p = head;
    tail = head;
}

ObjectList::~ObjectList()
{
    printf("destroy tracking...");
    count_and_delete_every_objects();
}

ObjectNode *ObjectList::get_head_node()
{
    return head;
}

int ObjectList::get_count()
{
    return count;
}

void ObjectList::reset_count()
{
    printf("manually reset count to 0\n");
    count = 0;
}

bool ObjectList::append_object(HumanObject *ob)
{
    printf("insert Object\n");
    ObjectNode *node = new ObjectNode;
    if (node == NULL)
    {
        printf("fatal! malloc object node failed\n");
        ob->~HumanObject();
        return 1;
    }
    node->ob = ob;
    node->next = NULL;
    tail->next = node;
    tail = node;
    return 0;
}

bool ObjectList::delete_object_by_label(int label)
{
    printf("try deleting object %d\n", label);
    p = head;
    while (p->next)
    {
        if (p->next->ob->get_index() == label)
        {
            break;
        }
        p = p->next;
    }
    if (p->next == NULL)
    {
        printf("Object %d not found\n", label);
        return 1;
    }

    ObjectNode *to_delete = p->next;
    p->next = p->next->next;
    /* move tail pointer if the tail node is deleted*/
    if (p->next == NULL)
    {
        tail = p;
    }
    /* delete node */
    to_delete->ob->~HumanObject();
    delete to_delete;
    return 0;
}

bool ObjectList::count_and_delete_every_objects()
{
    p = head->next;
    head->next = NULL;
    tail = head;
    while (p)
    {
        ObjectNode *to_delete = p;
        p = p->next;
        count += to_delete->ob->counting();
        if (to_delete->ob)
        {
            to_delete->ob->~HumanObject();
        }
        delete to_delete;
    }
    return 0;
}

int ObjectList::match_centroid(HumanObject *ob, Blob *blob_list, int n_blobs)
{
    if (ob == NULL)
    {
        printf("error: null object pointer\n");
        return 1;
    }
    int ob_x, ob_y;
    ob->predict(&ob_x, &ob_y);
    for (int i = 1; i <= n_blobs; i++)
    {
        Blob blob = blob_list[i - 1];
        int blob_x = blob.centroid_index % IM_W;
        int blob_y = blob.centroid_index / IM_W;
        int distance = distance_l1(blob_x, ob_x, blob_y, ob_y);
        if (distance <= 25)
        {
            printf("match object %d with blob %d centroid\n", ob->get_index(), i);
            ob->update(blob_x, blob_y, blob.size);
            return i;
        }
    }
    return 0;
}

int ObjectList::match_centrals(HumanObject *ob, Blob *blob_list, int n_blobs)
{
    if (ob == NULL)
    {
        printf("error: null object pointer\n");
        return 1;
    }
    printf("try match with centrals\n");
    int ob_x, ob_y;
    ob->predict(&ob_x, &ob_y);
    for (int n = 1; n <= n_blobs; n++)
    {
        printf("central of blob %d\n", n);
        Blob blob = blob_list[n - 1];
        if ((blob.central_distance_list == NULL) || (blob.central_index_list == NULL))
        {
            printf("no central points available\n");
            continue;
        }
        int16_t *central_x = (int16_t *)malloc(sizeof(int16_t) * blob.n_central_points);
        int16_t *central_y = (int16_t *)malloc(sizeof(int16_t) * blob.n_central_points);
        for (int i = 0; i < blob.n_central_points; i++)
        {
            central_x[i] = blob.central_index_list[i] % IM_W;
            central_y[i] = blob.central_index_list[i] / IM_W;
        }
        int min_loc = 0;
        int dist_min = distance_l1(central_x[0], ob_x, central_y[0], ob_y);
        for (int i = 1; i < blob.n_central_points; i++)
        {
            int dist_i = distance_l1(central_x[i], ob_x, central_y[i], ob_y);
            if (dist_i < dist_min)
            {
                min_loc = i;
                dist_min = dist_i;
            }
        }
        printf("dist min %d, %d, loc %d %d\n", dist_min, min_loc, central_x[min_loc], central_y[min_loc]);
        if (dist_min < 15)
        {
            printf("match ob %d with blob %d at central %d %d\n", ob->get_index(), n, central_x[min_loc], central_y[min_loc]);
            ob->update(central_x[min_loc], central_y[min_loc], 8 * blob.central_distance_list[min_loc]);
            free(central_x);
            free(central_y);
            return n;
        }
        else
        {
            free(central_x);
            free(central_y);
        }
    }
    return 0;
}

void ObjectList::matching(Blob *blob_list, int n_blobs)
{
    if ((n_blobs == 0) || (blob_list == NULL))
    {
        printf("detected 0 blob, delete every object\n");
        count_and_delete_every_objects();
        return;
    }
    bool *matched_flag = (bool *)malloc(sizeof(bool) * n_blobs);
    for (int i = 0; i < n_blobs; i++)
    {
        matched_flag[i] = false;
    }
    p = head->next;
    while (p)
    {
        int matched_blob_num = 0;
        matched_blob_num = match_centroid(p->ob, blob_list, n_blobs);
        if (matched_blob_num)
        {
            matched_flag[matched_blob_num - 1] = true;
            p = p->next;
            continue;
        }
        matched_blob_num = match_centrals(p->ob, blob_list, n_blobs);
        if (matched_blob_num)
        {
            matched_flag[matched_blob_num - 1] = true;
            p = p->next;
            continue;
        }

        /* if an object is not matched with any blob then delete it*/
        int relative_count = p->ob->counting();
        printf("count %2d\n", relative_count);
        count += relative_count;
        int label = p->ob->get_index();
        delete_object_by_label(label);
        p = p->next;
    }

    /* check if small blobs could be a fraction from a large blob in previous frame,
        if so, regester it to a existing object*/
    for (int i = 0; i < n_blobs; i++)
    {
        if (matched_flag[i])
        {
            continue;
        }
        uint32_t blob_loc = blob_list[i].centroid_index;
        int blob_x = blob_loc % IM_W;
        int blob_y = blob_loc / IM_W;
        int blob_sz = blob_list[i].size;
        p = head->next;
        while (p)
        {
            ObjectNode *node = p;
            p = p->next;
            int ob_x, ob_y;
            node->ob->predict(&ob_x, &ob_y);
            int distance = distance_l1(ob_x, blob_x, ob_y, blob_y);
            int ob_sz, last_sz;
            node->ob->get_size(&ob_sz, &last_sz);
            int total_size = (blob_sz + ob_sz);
            int size_score = last_sz - total_size;
            if ((distance < 60) && (size_score < 300) && (size_score > 0))
            {
                printf("regester blob %d at (%d %d) with size %d to object %d\n", i + 1, blob_x, blob_y, blob_sz, node->ob->get_index());
                float blob_weight = (float)blob_sz / total_size;
                float ob_weight = (float)ob_sz / total_size;
                printf("debug blob w %f, ob w%f\n", blob_weight, ob_weight);
                int merged_pos_x = blob_weight * blob_x + ob_weight * ob_x;
                int merged_pos_y = blob_weight * blob_y + ob_weight * ob_y;
                node->ob->update(merged_pos_x, merged_pos_y, total_size);
                matched_flag[i] = true;
                break;
            }
        }
    }

    /* spawn new object from unmatched blobs*/
    for (int i = 0; i < n_blobs; i++)
    {
        if (matched_flag[i])
        {
            continue;
        }
        Blob blob = blob_list[i];
        int pos_x, pos_y;
        pos_x = blob.centroid_index % IM_W;
        pos_y = blob.centroid_index / IM_W;
        HumanObject *ob = new HumanObject(i + 1, pos_x, pos_y, blob.size);
        append_object(ob);
    }
    free(matched_flag);
}
