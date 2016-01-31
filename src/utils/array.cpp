void copyArray(float *source, float *destination, int size){
    for(int i = 0; i < size; i++)
        destination[i] = source[i];
}

void convertMatrixToVector(float **matrix, float *vector, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; i++){
            int index = (i * cols) + j;
            vector[index] = matrix[i][j];
        }
    }
}

void copyIntArrIntoFloatArr(int *source, float *destination, int size){
    for(int i = 0; i < size; i++)
        destination[i] = (float)source[i];
}

void printArray(float *array, int n){
    std::cout << "[ ";
    for(int i = 0; i < n; i++){
        std::cout << array[i];
        if(i < n - 1 )
            std::cout << ", ";
    }
    std::cout << " ]\n";
}

void printVectorAs2DArray(float *array, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            std::cout << array[i * cols + j];
            if(j < cols - 1)
                std::cout << ", ";
            else
                std::cout << "\n";
        }
    }
}
