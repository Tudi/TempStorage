#define _USE_MATH_DEFINES
#include <math.h>

void dctII(double *out_Matrix, double *in_Matrix, size_t columns, size_t rows)
{
    for (size_t row = 0; row < columns; ++row) 
	{
        for (size_t col = 0; col < rows; ++col) 
		{
            out_Matrix[row * columns + col] = 0;
            for (size_t col2 = 0; col2 < columns; col2++) 
			{
                for (size_t row2 = 0; row2 < rows; row2++) 
				{
                    out_Matrix[row * columns + col] += in_Matrix[col2 * columns + row2] 
                        * cos(M_PI / ((float)columns) * (col2 + 1. / 2.) * row) 
                        * cos(M_PI / ((float)rows) * (row2 + 1. / 2.) * col);
                }               
            }
        }
    }  
 }