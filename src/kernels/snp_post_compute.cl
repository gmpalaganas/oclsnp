__kernel void snpPostCompute(int n, int m, __global float *rules, __global float *transitionVector, __local float *buffer){
    
    int i = get_global_id(0);
    int j = get_local_id(0);

    if( i < n ){
        buffer[j] = rules[i * 3 + 1]; 
        if(buffer[j] > -1)
            rules[i * 3 + 1] -= 1;
        transitionVector[i * (m + 1)] = 0;
    }
}
