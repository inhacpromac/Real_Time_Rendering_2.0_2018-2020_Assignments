#include <stdio.h>

#define NUM_ROWS 5
#define NUM_COLUMNS 3
#define DEPTH 2

int main(void)
{
	//IN-LINE INITIALIZATION
	int iArray[NUM_ROWS][NUM_COLUMNS][DEPTH] = { { { 9, 18 }, { 27, 36 }, { 45, 54 } },
											     { { 8, 16 }, { 24, 32 }, { 40, 48 } },
							                     { { 7, 14 }, { 21, 28 }, { 35, 42 } },
							                     { { 6, 12 }, { 18, 24 }, { 30, 36 } },
							                     { { 5, 10 }, { 15, 20 }, { 25, 30 } } };
	int i, j, k;

	int iArray_2D[NUM_ROWS][NUM_COLUMNS * DEPTH]; // 5 ROWS, 3*2 = 6 COLUMNS

	// ****** DISPLAY 3D ARRAY ******
	printf("\n");
	printf("Elements In The 3D Array : \n");
	for (i = 0; i < NUM_ROWS; i++)
	{
		printf("****** ROW %d ******\n", (i + 1));
		for (j = 0; j < NUM_COLUMNS; j++)
		{
			printf("****** COLUMN %d ******\n", (j + 1));
			for (k = 0; k < DEPTH; k++)
			{
				printf("iArray[%d][%d][%d] = %d\n", i, j, k, iArray[i][j][k]);
			}
		}
	}

	// ****** CONVERTING 3D TO 2D ******
	for (i = 0; i < NUM_ROWS; i++)
	{
		for (j = 0; j < NUM_COLUMNS; j++)
		{
			for (k = 0; k < DEPTH; k++)
			{
				iArray_2D[i][(j * DEPTH) + k] = iArray[i][j][k];
			}
		}
	}

	// ****** DISPLAY 2D ARRAY ******
	printf("\n\n\n\n");
	printf("Elements In The 2D Array : \n\n");
	for (i = 0; i < NUM_ROWS; i++)
	{
		printf("****** ROW %d ******\n", (i + 1));
		for (j = 0; j < (NUM_COLUMNS * DEPTH); j++)
		{
			printf("iArray_2D[%d][%d] = %d\n", i, j, iArray_2D[i][j]);
		}
	}

	return(0);
}
