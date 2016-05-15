__kernel void snpReset(int n, int m, __global float *lossVector, 
        __global float *gainVector, __global float *netGainVector, __global float *neuronFlags){
    
    int i = get_global_id(0);

    if( i < m ){
        lossVector[i] = 0;
        gainVector[i] = 0;
        netGainVector[i] = 0;
        neuronFlags[i] = -1;
    }
}
