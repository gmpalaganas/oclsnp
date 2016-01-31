__kernel void snpComputeNetGain(int n, int m, __global float *stateVector, __global float *lossVector, 
        __global float *gainVector, __global float *netGainVector){
    
    int i = get_global_id(0);

    if(i < m)
        netGainVector[i] = gainVector[i] * stateVector[i] + lossVector[i];

}
