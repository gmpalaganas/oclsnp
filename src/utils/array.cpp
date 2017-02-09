#include "array.hpp"

using namespace std;

void utils::copyArray(float *source, float *destination, int size){
    for(int i = 0; i < size; i++)
        destination[i] = source[i];
}

void utils::convertMatrixToVector(float **matrix, float *vector, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; i++){
            int index = (i * cols) + j;
            vector[index] = matrix[i][j];
        }
    }
}

void utils::copyIntArrIntoFloatArr(int *source, float *destination, int size){
    for(int i = 0; i < size; i++)
        destination[i] = (float)source[i];
}

void utils::printArray(float *array, int n){
    cout << "[ ";
    for(int i = 0; i < n; i++){
        cout << array[i];
        if(i < n - 1 )
            cout << ", ";
    }
    cout << " ]\n";
}

void utils::printVectorAs2DArray(float *array, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            cout << array[i * cols + j];
            if(j < cols - 1)
                cout << ", ";
            else
                cout << "\n";
        }
    }
}

void utils::print2DArray(int **array, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            cout << array[i][j];
            if(j < cols - 1)
                cout << ", ";
            else
                cout << "\n";
        }
    }
}
