

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


int thread_suc = 0; // Flag to indicate that a thread was able to find a solution
pthread_t winner_thread = 0; // Thread ID of the first thread to find a solution

void print_grid(int size, int grid[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) printf("%d\t", grid[i][j]);
		printf("\n");
	}
}

void read_grid_from_file(int size, char* ip_file, int grid[36][36]) {
	FILE* fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++) {
			fscanf(fp, "%d", &grid[i][j]);
		}
}

// Function for finding square root
int sqRoot(int N) {
	switch (N) {
	case 1:
		return 1;
	case 4:
		return 2;
	case 9:
		return 3;
	case 16:
		return 4;
	case 25:
		return 5;
	case 36:
		return 6;
	}
}

typedef struct {
	int size;
	int grid[36][36];
	int solved;
} solve_struct;

/* Structure for threaded checker function */
typedef struct {
	int size;
	int row;
	int col;
	int grid[36][36];
	int num_to_be_checked;
} sudoku_board;


void* col_checker(void* arg) {
	sudoku_board* s = (sudoku_board*)arg;
	int r;
	int* found = malloc((sizeof(int)));
	for (r = 0; r < s->size; r++) {
		if (s->grid[r][s->col] == s->num_to_be_checked) {
			*found = 1;
			break;
		}
		else
			*found = 0;
	}
	return (void*)found;
}

void* row_checker(void* arg) {
	sudoku_board* s = (sudoku_board*)arg;
	int c;
	int* found = malloc((sizeof(int)));
	for (c = 0; c < s->size; c++) {
		if (s->grid[s->row][c] == s->num_to_be_checked) {
			*found = 1;
			break;
		}
		else
			*found = 0;
	}
	return (void*)found;
}

void* box_checker(void* arg) {
	sudoku_board* s = (sudoku_board*)arg;
	int r, c;
	int* found = malloc((sizeof(int)));
	int boxOriginRow = s->row - (s->row % (int)sqRoot(s->size));
	int boxOriginCol = s->col - (s->col % (int)sqRoot(s->size));
	for (int r = 0; r < (int)sqRoot(s->size); r++) {  // box checker
		for (int c = 0; c < (int)sqRoot(s->size); c++) {
			if (s->grid[r + boxOriginRow][c + boxOriginCol] == s->num_to_be_checked) {
				*found = 1;
				break;
			}
			else
				*found = 0;
		}
		return (void*)found;
	}
}

// Threaded checking function (Calls three threads for checking row, column and sub-box)
int checker(int grid[36][36], int row_num, int col_num, int num_to_be_checked,
	int size) {
	pthread_t row, col, box;
	sudoku_board s[3];
	int* found_0;
	int* found_1;
	int* found_2;

	// initializing the structures
	for (int h = 0; h < 3; h++) {
		memcpy(s[h].grid, grid, 36 * 36 * sizeof(int));
		s[h].size = size;
		s[h].row = row_num;
		s[h].col = col_num;
		s[h].num_to_be_checked = num_to_be_checked;
	}

	if (pthread_create(&row, NULL, row_checker, (void*)(&s[0]))) {
		printf("Error in pthread_create for row thread");
		exit(-1);
	}
	if (pthread_create(&col, NULL, col_checker, (void*)(&s[1]))) {
		printf("Error in pthread_create for col thread");
		exit(-1);
	}
	if (pthread_create(&box, NULL, box_checker, (void*)(&s[2]))) {
		printf("Error in pthread_create for box thread");
		exit(-1);
	}

	pthread_join(row, (void**)&found_0);
	pthread_join(col, (void**)&found_1);
	pthread_join(box, (void**)&found_2);

	if (!(*found_0 + *found_1 + *found_2)) {
		return 1;
	}

	return 0;
}

int checker_non_threaded(int grid[36][36], int row_num, int col_num,
	int num_tobechecked, int size) {
	int boxOriginRow = row_num - (row_num % (int)sqRoot(size));
	int boxOriginCol = col_num - (col_num % (int)sqRoot(size));

	for (int c = 0; c < size; c++)  // column checker
	{
		if (grid[row_num][c] == num_tobechecked) return 0;
	}
	for (int r = 0; r < size; r++)  // row checker
	{
		if (grid[r][col_num] == num_tobechecked) return 0;
	}

	for (int r = 0; r < (int)sqRoot(size); r++) {  // box checker
		for (int c = 0; c < (int)sqRoot(size); c++) {
			if (grid[r + boxOriginRow][c + boxOriginCol] == num_tobechecked) {
				return 0;
			}
		}
	}
	return 1;
}

void swap(int* a, int* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

void shuffle(int arr[], int n) {
	srand(time(NULL));
	int i;
	for (i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		swap(&arr[i], &arr[j]);
	}
}

int is_solved(int size, int grid[36][36]) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (!grid[i][j]) {
				return 0;
			}
		}
	}
	return 1;
}

typedef struct {
	int size;
	int row;
	int col;
	int grid[36][36];
	int possibilities[36][36][36];
} fill_possibilities;

void* possibilities_row_checker(void* arg) {
	fill_possibilities* s = (fill_possibilities*)arg;
	for (int x = 0; x < s->size; x++) {
		if (s->grid[s->row][x]) {
			s->possibilities[s->row][s->col][(s->grid[s->row][x] - 1)] = 0;
		}
	}
}

void* possibilities_col_checker(void* arg) {
	fill_possibilities* s = (fill_possibilities*)arg;
	for (int x = 0; x < s->size; x++) {
		if (s->grid[x][s->col]) {
			s->possibilities[s->row][s->col][(s->grid[x][s->col] - 1)] = 0;
		}
	}
}

void* possibilities_box_checker(void* arg) {
	fill_possibilities* s = (fill_possibilities*)arg;
	int sub_box_size = sqRoot(s->size);
	int sub_box_index_i = (s->row / sub_box_size);
	int sub_box_index_j = (s->col / sub_box_size);

	for (int x = sub_box_index_i * sub_box_size; x < ((sub_box_index_i + 1) * sub_box_size); x++) {
		for (int y = sub_box_index_j * sub_box_size; y < ((sub_box_index_j + 1) * sub_box_size); y++) {
			if (s->grid[x][y]) {
				s->possibilities[s->row][s->col][(s->grid[x][y] - 1)] = 0;
			}
		}
	}
}

int fill_obvious(int size, int grid[36][36]) {
	int solved_bool = 0;

	// Generating possibilities matrix
	int possibilities[36][36][36] = {};
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			for (int k = 0; k < size; k++) {
				if (!(grid[i][j])) {
					possibilities[i][j][k] = 1;
				}
			}
		}
	}

	pthread_t row, col, box;
	fill_possibilities s[3];
	int found_obvious = 0;

	//initializing the structures (partially)
	for (int g = 0; g < 3; g++) {
		s[g].size = size;
		memcpy(s[g].possibilities, possibilities, 36 * 36 * 36 * sizeof(int));
	}

	do {
		found_obvious = 0;
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				// initializing the rest of the structure
				for (int g = 0; g < 3; g++) {
					memcpy(s[g].grid, grid, 36 * 36 * sizeof(int));
					s[g].row = i;
					s[g].col = j;
				}
				if (!grid[i][j]) {
					if (pthread_create(&row, NULL, possibilities_row_checker, (void*)(&s[0]))) {
						printf("Error in pthread_create for row thread");
						exit(-1);
					}
					if (pthread_create(&col, NULL, possibilities_col_checker, (void*)(&s[1]))) {
						printf("Error in pthread_create for row thread");
						exit(-1);
					}
					if (pthread_create(&box, NULL, possibilities_box_checker, (void*)(&s[2]))) {
						printf("Error in pthread_create for row thread");
						exit(-1);
					}

					pthread_join(row, NULL);
					pthread_join(col, NULL);
					pthread_join(box, NULL);


					//adding the possibilities returned by the threads
					for (int k = 0; k < size; k++) {
						possibilities[i][j][k] = s[0].possibilities[i][j][k] * s[1].possibilities[i][j][k] * s[2].possibilities[i][j][k];
					}

					int sum = 0;
					for (int k = 0; k < size; k++) {
						sum += possibilities[i][j][k];
					}

					// if obvious
					if (sum == 1) {
						found_obvious = 1;
						for (int k = 0; k < size; k++) {
							if (possibilities[i][j][k]) {
								grid[i][j] = (k + 1);
								break;
							}
						}
					}
				}
			}
		}

		solved_bool = is_solved(size, grid);

		if (!solved_bool) {
			if (!found_obvious) {
				return 0;
			}
		}
		else {
			// obvious solution found
			return 1; // prevents further solved calls
		}


	} while (found_obvious);

}

// solve function called inside the thread
int solve(int grid[36][36], int size) {
	int randArray[size];
	for (int i = 0; i < size; i++) randArray[i] = i + 1;
	shuffle(randArray, size);
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size; c++) {
			if (grid[r][c] == 0) {  // empty cell found
				for (int possible = 0; possible < size; possible++) {
					if (checker_non_threaded(grid, r, c, randArray[possible], size)) {
						grid[r][c] = randArray[possible];  // set cell value

						/* Alternative: Recursively calling the fill_obvious() function before backtracking */
						// if (!fill_obvious(size, grid)) {
						if (solve(grid, size)) {
							printf("solve returning correct solution\n");
							return 1;
						}  // correct solution found, return.

						grid[r][c] = 0;  // not correct solution; reset cell,try next possibility
					// }
					// else {
					// 	return 1;
					// }
					}
				}
				// No possibilities in this cell. Backtracks
				return 0;
			}
		}
	}
	return 1;
}

// Multithreading wrapper for the solve() function
void* solve_threaded(void* arg) {
	solve_struct* s = (solve_struct*)arg; // s is the struct that was passed to pthread_create()

	int* solved = malloc((sizeof(int)));
	*solved = solve(s->grid, s->size);

	if (*solved == 1) {
		thread_suc = 1;
		winner_thread = pthread_self();
		printf("solve_threaded returning correct solution\n");
	}

	free(solved);
	return NULL;
}

// Called by the int main() function. Creates threads for which solve_threaded is the driver function
int solve_caller(int grid[36][36], int size) {
	int num_threads = size;
	pthread_t* tids;
	tids = malloc(sizeof(pthread_t) * num_threads);
	solve_struct s[num_threads];

	// initializing the structures
	for (int h = 0; h < num_threads; h++) {
		memcpy(s[h].grid, grid, 36 * 36 * sizeof(int));
		s[h].size = size;
	}

	for (int n = 0; n < num_threads; n++) {
		if (pthread_create(&tids[n], NULL, solve_threaded, (void*)(&s[n]))) {
			printf("Error in pthread_create for thread %d", n);
			exit(-1);
		}
	}

	while (!thread_suc) { // until thread_suc is 1 (i.e, solution is found)
		sleep(1); // poll the threads looking for any successes every second
	}

	// these blocks are only reached when a solution has been found

	// cancel all running threads (blocking)
	// not using join cause we want to terminate the moment one thread returns
	for (int c = 0; c < num_threads; c++) {
		pthread_cancel(tids[c]);
	}

	for (int c = 0; c < num_threads; c++) {
		if (winner_thread == tids[c]) {
			print_grid(size, s[c].grid);
			break;
		}
	}

	return 1;  // can never fail because it will be stuck in the while loop till succeeds
}

int main(int argc, char* argv[]) {
	int grid[36][36], size, i, j;

	if (argc != 3) {
		printf("Usage: ./sudoku.out grid_size inputfile");
		exit(-1);
	}

	size = atoi(argv[1]);
	read_grid_from_file(size, argv[2], grid);

	if (!fill_obvious(size, grid)) { // no obvious solution found initially
		solve_caller(grid, size);
	}
	else {
		//Obvious function was enough
		print_grid(size, grid);
	}
}