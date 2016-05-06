__kernel void snpDetermineRules(int n, __global float *spikingVector, __global float *ruleMatches){
    
    int i = get_global_id(0);

    if( i < n ){
        if( rules[i*3+2] ){

            spikingVector[i] = 1; 

        }else{
            spikingVector[i] = 0;
        }
    }
}
