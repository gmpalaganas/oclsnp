__kernel void snpDetermineRules(int n, int m, __global float *configVector, __global float *spikingVector,
        __global float *rules, __global float* lhs){
    
    int i = get_global_id(0);

    if( i < n ){
        if( (configVector[(int)rules[i * 3] - 1] == lhs[i] || lhs[i] == 0)  && 
            configVector[(int)rules[i * 3] - 1] + rules[i * 3 + 2] >= 0){

            spikingVector[i] = 1; 

        }else{
            spikingVector[i] = 0;
        }
    }
}
