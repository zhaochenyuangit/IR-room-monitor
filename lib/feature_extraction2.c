#include "feature_extraction.h"
#include "image_size.h"

static uint32_t calc_centroid(uint8_t *mask, int width, int height, uint32_t *centroid_index);
static void chamfer_distance_transform(uint8_t *mask, int width, int height, int a, int b);
static int calc_centrals(uint8_t *submask, int width, int height, uint32_t **location_list_ptr, uint8_t **distance_list_ptr);

void print_blob_info(Blob *blob_list, int n_blobs)
{
    for (int i = 0; i < n_blobs; i++)
    {
        printf("blob num %d: ", i + 1);
        printf("size %d centroid %d\n", blob_list[i].size, blob_list[i].centroid_index);
        printf("central number %d\n", blob_list[i].n_central_points);
    }
}

void print_blob_mask(Blob *blob_list, int n_blobs, short *holder, int w, int h)
{
    for (int n = 0; n < n_blobs; n++)
    {
        for (int i = 0; i < (w * h); i++)
        {
            holder[i] = 0;
        }
        printf("blob number %d: \n", n + 1);
        for (int i = 0; i < blob_list[n].n_central_points; i++)
        {
            int index = blob_list[n].central_index_list[i];
            holder[index] = blob_list[n].central_distance_list[i];
        }
        for (int r = 0; r < h; r++)
        {
            for (int c = 0; c < w; c++)
            {
                printf("%d ", holder[r * w + c]);
            }
            printf("\n");
        }
    }
}

struct Blob *extract_feature(uint8_t *labeled, int n_blobs, int img_w, int img_h)
{
    if (n_blobs == 0)
    {
        return NULL;
    }
    struct Blob *blob_list = malloc(sizeof(struct Blob) * n_blobs);
    if (blob_list == NULL)
    {
        printf("extract_feature blob list malloc failed\n");
        return NULL;
    }
    for (int num = 1; num <= n_blobs; num++)
    {
        static uint8_t submask[IM_LEN];
        for (int i = 0; i < IM_LEN; i++)
        {
            submask[i] = (labeled[i] == num) ? 1 : 0;
        }
        blob_list[num - 1].size = calc_centroid(submask, img_w, img_h, &blob_list[num - 1].centroid_index);
        blob_list[num - 1].n_central_points = calc_centrals(submask, img_w, img_h, &blob_list[num - 1].central_index_list, &blob_list[num - 1].central_distance_list);
    }
    return blob_list;
}

/** @brief calculate central points of a single blob
 * @param submask,width,height the binary mask of a single blob and its dimensions
 * @param location_list_ptr a pointer to a list storing the linear index of central points
 * @param distance_list_ptr a pointer to a list storing the distance value of central points 
 * @return length of the list, -1 when error
 */
static int calc_centrals(uint8_t *submask, int width, int height, uint32_t **location_list_ptr, uint8_t **distance_list_ptr)
{
    const float percentage = 0.76;
    //even if the blob occpies the whole FOV it can only have so many central points
    enum
    {
        max_pixels = (int)((IM_H * (1 - percentage) + 2) * (IM_W * (1 - percentage) + 2)),
    };
    static uint32_t location_list_largest[max_pixels];
    static uint8_t distance_list_largest[max_pixels];
    chamfer_dt_city(submask, width, height);
    //calculate max distance
    int max_loc = 0;
    for (int i = 1; i < (width * height); i++)
    {
        if (submask[i] > submask[max_loc])
        {
            max_loc = i;
        }
    }
    int threshold = submask[max_loc] * percentage;
    //calculate number of central points
    int n_points = 0;
    for (int i = 0; i < (width * height); i++)
    {
        if (n_points >= max_pixels)
        {
            printf("more pixels than expected\n");
            break;
        }
        if (submask[i] > threshold)
        {
            location_list_largest[n_points] = i;
            distance_list_largest[n_points] = submask[i];
            n_points++;
        }
    }
    //if blob is too small, do not calculate central points
    if (n_points < 10)
    {
        *location_list_ptr = NULL;
        *distance_list_ptr = NULL;
        return n_points;
    }
    *location_list_ptr = malloc(sizeof(uint32_t) * n_points);
    if (*location_list_ptr == NULL)
    {
        printf("calc_centrals malloc failed!\n");
        return (-1);
    }
    *distance_list_ptr = malloc(sizeof(uint8_t) * n_points);
    if (*distance_list_ptr == NULL)
    {
        printf("calc_centrals malloc failed!\n");
        free(*location_list_ptr);
        return (-1);
    }
    for (int i = 0; i < n_points; i++)
    {
        (*location_list_ptr)[i] = location_list_largest[i];
        (*distance_list_ptr)[i] = distance_list_largest[i];
    }
    return n_points;
}

void chamfer_dt_city(uint8_t *mask, int width, int height)
{
    chamfer_distance_transform(mask, width, height, 1, 2);
}

/** @brief chamfer distance transform of a single blob
 * @param mask,width,height the binary mask to be transformed and its width, height
 * @param a,b the distance definition, a: distance to neighbour-4 pixels, b: distance to corner-neighbour pixels 
*/
static void chamfer_distance_transform(uint8_t *mask, int width, int height, int a, int b)
{
    if ((width >= 255) || (height >= 255))
    {
        printf("WARN: distance is hold in a uint8_t variable, which may overflow \n \
        for large image when object dimension could be greater than 255\n");
    }

    //first pass
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            if (mask[index])
            {
                uint32_t candidates[4];
                int n_candidates = 0;
                if (r >= 1)
                {
                    candidates[n_candidates++] = mask[index - width] + a; //north
                    if (c >= 1)
                    {
                        candidates[n_candidates++] = mask[index - width - 1] + b; //north west
                    }
                    if (c < (width - 1))
                    {
                        candidates[n_candidates++] = mask[index - width + 1] + b; //north east
                    }
                }
                if (c >= 1)
                {
                    candidates[n_candidates++] = mask[index - 1] + a; //west
                }
                if (!n_candidates)
                {
                    mask[index] = 1;
                }
                else
                {
                    int min_loc = 0;
                    for (int i = 1; i < n_candidates; i++)
                    {
                        if (candidates[i] < candidates[min_loc])
                        {
                            min_loc = i;
                        }
                    }
                    mask[index] = candidates[min_loc];
                }
            }
        }
    }
    //second pass
    for (int r = (height - 1); r >= 0; r--)
    {
        for (int c = (width - 1); c >= 0; c--)
        {
            int index = r * width + c;
            if (mask[index])
            {
                uint32_t candidates[5];
                candidates[0] = mask[index];
                int n_candidates = 1;
                if (r < (height - 1))
                {
                    candidates[n_candidates++] = mask[index + width] + a; //south
                    if (c >= 1)
                    {
                        candidates[n_candidates++] = mask[index + width - 1] + b; //south west
                    }
                    if (c <= (width - 1))
                    {
                        candidates[n_candidates++] = mask[index + width + 1] + b; //south east
                    }
                }
                if (c <= (width - 1))
                {
                    candidates[n_candidates++] = mask[index + 1] + a; //east
                }
                if (!n_candidates)
                {
                    mask[index] = 1;
                }
                else
                {
                    int min_loc = 0;
                    for (int i = 1; i < n_candidates; i++)
                    {
                        if (candidates[i] < candidates[min_loc])
                        {
                            min_loc = i;
                        }
                    }
                    mask[index] = candidates[min_loc];
                }
            }
        }
    }
}

/** @brief calculate centroid of a single blob
 * @param mask,width,height the binary blob mask and its dimensions
 * @param index the vairable to store the linear index of the centroid
 * @return size of the blob
*/
static uint32_t calc_centroid(uint8_t *mask, int width, int height, uint32_t *centroid_index)
{
    uint32_t row_sum = 0;
    uint32_t col_sum = 0;
    uint32_t count = 0;
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            if (mask[index])
            {
                count++;
                col_sum += c;
                row_sum += r;
            }
        }
    }
    row_sum /= count;
    col_sum /= count;
    *centroid_index = row_sum * width + col_sum;
    return count;
}

/* full destroy a blob list */
void delete_blob_list(Blob *blob_list, int n_blobs)
{
    if ((n_blobs == 0) || (blob_list == NULL))
    {
        return;
    }
    for (int i = 0; i < n_blobs; i++)
    {
        if (blob_list[i].central_distance_list)
        {
            free(blob_list[i].central_distance_list);
        }
        if (blob_list[i].central_index_list)
        {
            free(blob_list[i].central_index_list);
        }
    }
    free(blob_list);
}