#include "detect.h"
#include "image_size.h"

void image_copy(short *src, short *dst, int size)
{
    for (int i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
}

void mask_copy(uint8_t *src, uint8_t *dst, int size)
{
    for (int i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
}

int labeling8(uint8_t *mask, int width, int height)
{
    static uint16_t search_list[IM_LEN];
    if ((width > 256) || (height > 256))
    {
        printf("ERROR: grideye api support image size up to uint8_t\n");
        return -1;
    }
    //change object indicator mark from number 1 to 0xff(255) to avoid
    //meaning overload between "object exist" and "object label 1"
    uint8_t mark = 0xff;
    for (int i = 0; i < (width * height); i++)
    {
        if (mask[i])
        {
            mask[i] = mark;
        }
    }
    uint16_t area_min = 20;
    uint8_t num = ucAMG_PUB_ODT_CalcDataLabeling8((uint8_t)width, (uint8_t)height, mark, area_min, mask, search_list);
    return num;
}

bool average_filter(short *image, int width, int height, int side)
{
    static unsigned int sum_table[IM_LEN];
    summed_area_table(image, sum_table, width, height);
    average_of_area(sum_table, image, width, height, side);
    return 0;
}

bool binary_fill_holes(uint8_t *mask, int width, int height)
{
    if (((width + 2) * (height + 2)) > 65536)
    {
        /** back ground search list is used to store pixel coordinates
         * it must be large enought to hold (width+2)*(height+2)
         * here it is a uint16_t list so max index is 65535
        */
        return -1;
    }
    static uint16_t search_list[(IM_W + 2) * (IM_H + 2)];
    static uint8_t padded[(IM_W + 2) * (IM_H + 2)];
    static uint8_t holemask[IM_LEN];
    binary_extract_holes(mask, holemask, width, height, padded, search_list);
    for (int i = 0; i < (width * height); i++)
    {
        mask[i] = (holemask[i] | mask[i]);
    }
    return 0;
}

bool discrete_convolution_2d_seperable(short *image, int width, int height, Filter *fx, Filter *fy)
{
    static short holder[IM_LEN];
    convolution_x(image, holder, width, height, fx);
    convolution_y(holder, image, width, height, fy);
    return 0;
}
