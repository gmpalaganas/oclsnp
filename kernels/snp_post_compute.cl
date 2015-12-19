__kernel void snpPostCompute(int n, int m, __global float *rules, __global float *transitionVector){
    
    int i = get_global_id(0);

    if( i < n ){
        if(rules[i * 3 + 1] > -1)
            rules[i * 3 + 1] -= 1;
        transitionVector[i * (m + 1)] = 0;
    }
}
