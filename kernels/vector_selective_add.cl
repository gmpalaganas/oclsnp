__kernel void vectorSelectiveAdd(__global float *inputVector, __global float *outputVector, int rows, int cols){
    int i = get_global_id(0);

    if( i < cols ){
        outputVector[i] = 0;
        int holder = 0;

        for(int j = 0; j < rows; j++)
            if(inputVector[j * (cols + 1)] == 1)
                holder += inputVector[j*(cols+1) + (i+1)];
        outputVector[i] = holder;
    }
}
