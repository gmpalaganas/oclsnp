__kernel void vectorAdd(__global float *A, __global float *B, __global float *C, int vecSize){
    int = get_global_id(0);

    if(i < vecSize)
        C[i] = A[i] + B[i];
}
