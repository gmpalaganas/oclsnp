__kernel void vectorElemMult(__global float *vectorA, __global float *vectorB,
	__global float *outputVector, int vectorSize){
    
    int i = get_global_id(0);

    if(i < vectorSize)
        outputVector[i] = vectorA[i] * vectorB[i];

}
