/**
 * naiive 2d convolution method, do not use them in final codes
 * because extreamly slow speed
 * but could be use to validate result of the faster algorithms
 */
#include "detect.h"

struct Filter
{
    int *kernel;
    int side;
    int *weight;
};

struct Filter *gaussian_kernel_2d(double sigma)
{
    struct Filter *f = malloc(sizeof(struct Filter));
    if (f == NULL)
    {
        return NULL;
    }
    int l = round((3 * sigma) * 2);
    if (l % 2 == 0)
    {
        l = l - 1;
    }
    int radius = (l - 1) / 2;
    int center = radius * l + radius;
    int *kernel = malloc(sizeof(int) * (l * l));
    if (kernel == NULL)
    {
        free(f);
        return NULL;
    }
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            int ref = y * l + x + center;
            double value = exp(-0.5 * (x * x + y * y) / (sigma * sigma));
            value *= 256;
            kernel[ref] = round(value);
        }
    }
    int sum = 0;
    for (int i = 0; i < l * l; i++)
    {
        sum += kernel[i];
    }
    f->side = l;
    f->kernel = kernel;
    f->weight = NULL;
    return f;
}

void discrete_convolution_2d(short *image, short *output, int image_width, int image_height, struct Filter *filter, int step)
{
    assert(((image_width - 1) / step) % 1 == 0);
    assert(((image_height - 1) / step) % 1 == 0);

    int output_index = 0;
    int radius = (filter->side - 1) / 2;
    int center = radius * filter->side + radius;
    for (int row = 0; row < image_height; row += step)
    {
        for (int col = 0; col < image_width; col += step)
        {
            int index = row * image_width + col;
            unsigned int intermediate_sum = 0;
            int weight = 0;
            for (int ref = 0; ref < (filter->side * filter->side); ref++)
            {
                /**one loop like this is somehow strange and may decrease performance
                 * change to two loops by x and y like the gaussian_kernel_2d() could be better
                 * but is slow anyway
                 */
                if (filter->kernel[ref] == 0)
                {
                    continue;
                }
                int row_shift = (ref - center) / filter->side;
                int col_shift = (ref - center) % filter->side;
                if (col_shift < -radius)
                {
                    row_shift = row_shift - 1;
                    col_shift = col_shift + filter->side;
                }
                else if (col_shift > radius)
                {
                    row_shift = row_shift + 1;
                    col_shift = col_shift - filter->side;
                }
                bool out_of_bondary;
                out_of_bondary = ((row + row_shift) < 0) ||
                                 ((row + row_shift) >= image_height) ||
                                 ((col + col_shift) < 0) ||
                                 ((col + col_shift) >= image_width);
                if (out_of_bondary)
                {
                    continue;
                }
                int index_shift = row_shift * image_width + col_shift;
                intermediate_sum += image[index + index_shift] * filter->kernel[ref];
                weight += filter->kernel[ref];
            }
            intermediate_sum /= weight;
            output[output_index] = intermediate_sum;
            output_index++;
        }
    }
}

void pooling_2d(short *image, short *output, int image_width, int image_height, struct Filter *mask, pool_function_t fun, int step)
{
    assert(((image_width - 1) / step) % 1 == 0);
    assert(((image_height - 1) / step) % 1 == 0);

    int output_index = 0;
    int radius = (mask->side - 1) / 2;
    int center = radius * mask->side + radius;
    for (int row = 0; row < image_height; row += step)
    {
        for (int col = 0; col < image_width; col += step)
        {
            int index = row * image_width + col;
            short array[mask->side * mask->side];
            int count = 0;
            for (int ref = 0; ref < (mask->side * mask->side); ref++)
            {
                /**one loop like this is somehow strange and may decrease performance
                 * change to two loops by x and y like the gaussian_kernel_2d() could be better
                 * but is slow anyway
                 */
                if (mask->kernel[ref] == 0)
                {
                    continue;
                }
                int row_shift = (ref - center) / mask->side;
                int col_shift = (ref - center) % mask->side;
                if (col_shift < -radius)
                {
                    row_shift = row_shift - 1;
                    col_shift = col_shift + mask->side;
                }
                else if (col_shift > radius)
                {
                    row_shift = row_shift + 1;
                    col_shift = col_shift - mask->side;
                }
                bool out_of_bondary;
                out_of_bondary = ((row + row_shift) < 0) ||
                                 ((row + row_shift) >= image_height) ||
                                 ((col + col_shift) < 0) ||
                                 ((col + col_shift) >= image_width);
                if (out_of_bondary)
                {
                    continue;
                }
                int index_shift = row_shift * image_width + col_shift;
                array[count] = image[index + index_shift];
                count++;
            }
            output[output_index] = (short)fun(array, count);
            output_index++;
        }
    }
}