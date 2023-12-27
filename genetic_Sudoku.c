#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define EMPTY 0
#define SIZE 9
#define BLOCK_SIZE 3
#define POPULATION_SIZE 500
#define MAX_GENERATIONS 1000
#define SELECTION_RATE 0.5
#define MUTATION_RATE 0.25 
#define MAX_RESTART_TIMES 5

// Ý tưởng:

// B1: Khởi tạo cá thể: gán vào grid các giá trị cố định cho trước từ file,
// các ô trống sẽ được gán giá trị ngẫu nhiên từ 1 - 9 sao cho trong mỗi
// block 3x3 không có giá trị bị lặp lại (mỗi grid được chia làm 9 block 3x3
// được đánh số thứ tự từ 0 - 8).

// B2: Khởi tạo quần thể ngẫu nhiên gồm POPULATION_SIZE cá thể.

// B3: Sắp xếp quần thể theo thứ tự độ thích nghi giảm dần (fitness tăng dần).

// B4: Chọn SELECTION_RATE % số cá thể ở đầu quần thể(thế hệ bố mẹ "tốt")
// cùng với (1 - SELECTION_RATE)% số cá thể ngẫu nhiên trong quần thể để
// tạo thành quần thể mới.

// B5: Chọn ngẫu nhiên POPULATION_SIZE cặp bố mẹ để lai tạo cho ra cá thể con.
// Với mỗi cặp bố mẹ, chọn ngẫu nhiên m block từ cá thể mẹ và 9 - m block
// từ cá thể bố để hợp thành cá thể mới.

// B6: Sau khi lai tạo, đột biến quần thể mới, số lượng cá thể đột biến
// tuỳ thuộc vào MUTATION_RATE. Đột biến bằng cách chọn ngẫu nhiên 1 block
// trong cá thể, trong block đó tiếp tục chọn ngẫu nhiên 2 ô không chứa
// giá trị cố định cho trước sau đó đổi vị trí 2 ô này.

// B7: Nếu chưa tìm được kết quả thì lặp lại giải thuật cho đến khi số lần
// lặp lại bằng với MAX_RESTART_TIMES, nếu vẫn chưa tìm được kết quả thì chương trình kết thúc.

typedef struct
{
    int grid[SIZE][SIZE];            // Lưu bảng sudoku
    int fixedValueCells[SIZE][SIZE]; // Lưu các ô có sẵn giá trị
    int fitness;                     // Độ thích nghi
} Individual;

typedef struct
{
    Individual individual[POPULATION_SIZE];
} Population;

void loadFile(int grid[SIZE][SIZE])
{
    FILE *f = fopen("./input.txt", "r");
    if (f == NULL)
    {
        printf("Khong the mo file.\n");
    }
    else
    {
        int x, row, col;
        for (row = 0; row < SIZE; row++)
        {
            for (col = 0; col < SIZE; col++)
            {
                fscanf(f, "%d", &x);
                grid[row][col] = x;
            }
        }
    }
}

void printGrid(int grid[SIZE][SIZE])
{
    printf("=======================\n");
    int row, col;
    for (row = 0; row < SIZE; row++)
    {
        if (row > 0 && row % BLOCK_SIZE == 0)
        {
            printf("----------------------\n");
        }
        for (col = 0; col < SIZE; col++)
        {
            printf("%d ", grid[row][col]);
            if ((col + 1) % BLOCK_SIZE == 0)
            {
                printf("| ");
            }
        }
        printf("\n");
    }
    printf("=======================\n");
}

int valueExistInBlock(int grid[SIZE][SIZE], int row, int col, int value)
{
    // Xác định block có chứa hàng row cột col đã xuất hiện giá trị value hay chưa
    int startRow = row - row % BLOCK_SIZE;
    int startCol = col - col % BLOCK_SIZE;
    int i, j;
    for (i = startRow; i < startRow + BLOCK_SIZE; i++)
    {
        for (j = startCol; j < startCol + BLOCK_SIZE; j++)
        {
            if (grid[i][j] == value)
            {
                return 1;
            }
        }
    }
    return 0;
}

void initializeIndividual(Individual *individual, int inputGrid[SIZE][SIZE])
{
    // Khởi tạo ngẫu nhiên cá thể sao cho trong mỗi block 3x3 không có giá trị lặp lại
    int row, col;
    for (row = 0; row < SIZE; row++)
    {
        for (col = 0; col < SIZE; col++)
        {
            individual->grid[row][col] = inputGrid[row][col];
            individual->fixedValueCells[row][col] = 1;
        }
    }
    int value;
    for (row = 0; row < SIZE; row++)
    {
        for (col = 0; col < SIZE; col++)
        {
            if (individual->grid[row][col] == EMPTY)
            {
                individual->fixedValueCells[row][col] = 0;
                value = rand() % SIZE + 1;
                while (valueExistInBlock(individual->grid, row, col, value))
                {
                    value = rand() % SIZE + 1;
                }
                individual->grid[row][col] = value;
            }
        }
    }
}

void initializePopulation(Population *population)
{
    // Khởi tạo quần thể
    int inputGrid[SIZE][SIZE];
    loadFile(inputGrid);

    int i;
    for (i = 0; i < POPULATION_SIZE; i++)
    {
        initializeIndividual(&population->individual[i], inputGrid);
    }
}

int containsValue(int array[], int size, int value)
{
    // Kiểm tra giá trị value có tồn tại trong mảng array hay chưa
    int i;
    for (i = 0; i < size; i++)
    {
        if (array[i] == value)
        {
            return 1;
        }
    }
    return 0;
}

int *generateRandomNumbers(int n, int min, int max)
{
    // Chọn ngẫu nhiên n giá trị trong khoảng từ min đến max
    int *randomNumbers = (int *)malloc(n * sizeof(int));
    int count = 0;

    while (count < n)
    {
        int randomNumber = rand() % (max - min + 1) + min;
        int isDuplicate = 0;

        // Kiểm tra xem số vừa tạo đã tồn tại trong mảng chưa
        int i;
        for (i = 0; i < count; i++)
        {
            if (containsValue(randomNumbers, count, randomNumber))
            {
                isDuplicate = 1;
                break;
            }
        }

        // Nếu số vừa tạo chưa tồn tại thì thêm vào mảng
        if (!isDuplicate)
        {
            randomNumbers[count] = randomNumber;
            count++;
        }
    }

    return randomNumbers;
}

void crossover(Individual *father, Individual *mother, Individual *child)
{
    // Lai tạo
    int numOfBlocks = rand() % (SIZE - 1) + 1;                           // Chọn ngẫu nhiên số block
    int *randomBlocks = generateRandomNumbers(numOfBlocks, 0, SIZE - 2); // Chọn ngẫu nhiên số thứ tự block
    int block, row, col;
    for (block = 0; block < SIZE; block++)
    {
        int startRow = block / BLOCK_SIZE * BLOCK_SIZE;
        int startCol = block % BLOCK_SIZE * BLOCK_SIZE;
        for (row = startRow; row < startRow + BLOCK_SIZE; row++)
        {
            for (col = startCol; col < startCol + BLOCK_SIZE; col++)
            {
                // Gán lại vị trí các ô có giá trị cố định
                child->fixedValueCells[row][col] = father->fixedValueCells[row][col];
                // Nếu block nào được chọn thì lấy từ mẹ
                if (containsValue(randomBlocks, numOfBlocks, block))
                {
                    child->grid[row][col] = mother->grid[row][col];
                }
                // Các block còn lại lấy từ bố
                else
                    child->grid[row][col] = father->grid[row][col];
            }
        }
    }
    free(randomBlocks);
}

void evaluateFitness(Individual *individual)
{
    // Đánh giá độ thích nghi của cá thể: độ thích nghi = số xung đột trong hàng và cột,
    // mỗi lần lặp lại của 1 số được tính là 1 xung đột. Cá thể có độ thích nghi = 0 là
    // kết quả cần tìm
    int conflicts = 0;
    int row, col, i, value;
    int count[SIZE + 1];
    // Kiểm tra xung đột trong hàng
    for (row = 0; row < SIZE; row++)
    {
        for (i = 0; i <= SIZE; i++)
        {
            count[i] = 0;
        }
        for (col = 0; col < SIZE; col++)
        {
            value = individual->grid[row][col];
            count[value]++;
        }
        for (i = 1; i <= SIZE; i++)
        {
            if (count[i] > 1)
                conflicts += count[i] - 1;
        }
    }

    // Kiểm tra xung đột trong cột
    for (col = 0; col < SIZE; col++)
    {
        for (i = 0; i <= SIZE; i++)
        {
            count[i] = 0;
        }
        for (row = 0; row < SIZE; row++)
        {
            value = individual->grid[row][col];
            count[value]++;
        }
        for (i = 1; i <= SIZE; i++)
        {
            if (count[i] > 1)
                conflicts += count[i] - 1;
        }
    }
    individual->fitness = conflicts;
}

void sortPopulation(Population *population)
{
    // Sắp xếp quần thể theo độ thích nghi giảm dần (fitness tăng dần)
    int i, j;
    for (i = 0; i < POPULATION_SIZE - 1; i++)
    {
        int minIndex = i;
        for (j = i + 1; j < POPULATION_SIZE; j++)
        {
            if (population->individual[j].fitness < population->individual[minIndex].fitness)
            {
                minIndex = j;
            }
        }

        if (minIndex != i)
        {
            Individual temp = population->individual[i];
            population->individual[i] = population->individual[minIndex];
            population->individual[minIndex] = temp;
        }
    }
}

Population selectFromPopulation(Population population)
{
    // Chọn quần thể mới để tiến hành lai tạo
    Population selectedPopulation;
    int i;
    for (i = 0; i < round(POPULATION_SIZE * SELECTION_RATE); i++)
    {
        selectedPopulation.individual[i] = population.individual[i];
    } // Chọn SELECTION_RATE % cá thể đứng đầu
    for (i; i < POPULATION_SIZE; i++)
    {
        selectedPopulation.individual[i] = population.individual[rand() % POPULATION_SIZE];
    } // Chọn ngẫu nhiên các cá thể còn lại cho đủ quần thể
    return selectedPopulation;
}

void swapTwoCells(Individual *individual)
{
    // Đổi vị trí 2 ô không chứa giá trị cố định trong cá thể
    int block = rand() % SIZE; // Chọn ngẫu nhiên 1 block
    int startRow = block / BLOCK_SIZE * BLOCK_SIZE;
    int startCol = block % BLOCK_SIZE * BLOCK_SIZE;

    int cell1Row, cell1Col, cell2Row, cell2Col;

    // Chọn ngẫu nhiên 2 ô không chứa giá trị cố định
    do
    {
        cell1Row = rand() % BLOCK_SIZE + startRow;
        cell1Col = rand() % BLOCK_SIZE + startCol;
        cell2Row = rand() % BLOCK_SIZE + startRow;
        cell2Col = rand() % BLOCK_SIZE + startCol;
    } while (individual->fixedValueCells[cell1Row][cell1Col] == 1 || individual->fixedValueCells[cell2Row][cell2Col] == 1);

    int temp = individual->grid[cell1Row][cell1Col];
    individual->grid[cell1Row][cell1Col] = individual->grid[cell2Row][cell2Col];
    individual->grid[cell2Row][cell2Col] = temp;
}

void mutate(Population *population)
{
    // Đột biến
    int i;
    for (i = 0; i < POPULATION_SIZE; i++)
    {
        if ((double)rand() / RAND_MAX < MUTATION_RATE)
        {
            swapTwoCells(&population->individual[i]);
        } // Với từng cá thể nếu tỉ lệ ngẫu nhiên nhỏ hơn MUTATION_RATE thì tiến hành đột biến
    }
}

int genetic(Population population, Individual *bestIndividual)
{
    // Di truyền quần thể
    int generation = 0;
    while (generation < MAX_GENERATIONS)
    {
        int i;
        for (i = 0; i < POPULATION_SIZE; i++)
        {
            evaluateFitness(&population.individual[i]);

            if (population.individual[i].fitness == 0)
            {
                *bestIndividual = population.individual[i];
                printf("\nDa tim thay ket qua o the he thu: %d\n", generation);
                printf("Do thich nghi: %d\n", bestIndividual->fitness);
                printf("Ket qua tim duoc:\n");
                printGrid(bestIndividual->grid);
                return 1;
            }
        }

        sortPopulation(&population);
        Population selectedPopulation = selectFromPopulation(population);

        Population newPopulation;

        for (i = 0; i < POPULATION_SIZE; i++)
        {
            Individual father = selectedPopulation.individual[rand() % POPULATION_SIZE];
            Individual mother = selectedPopulation.individual[rand() % POPULATION_SIZE];
            Individual child;
            crossover(&father, &mother, &child);
            population.individual[i] = child;
        }
        mutate(&population);
        generation++;
    }
    return 0;
}

int main()
{
    srand(time(NULL));
    int runTimes = 0, success = 0;
    while (runTimes < MAX_RESTART_TIMES && !success)
    {
        Population population;
        initializePopulation(&population);
        Individual bestIndividual;
        success = genetic(population, &bestIndividual);
        // if(success) printf("Da khoi tao lai quan the %d lan",runTimes);
        if(!success) printf("\nQuan the thu %d: Khong tim thay ket qua",runTimes+1);
        runTimes++;
    }
    // if (!success)
    // {
    //     printf("\nKhong tim thay ket qua.\n");
    // }
    return 0;
}
