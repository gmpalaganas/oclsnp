__kernel void snpComputeNetGain(int n, int m, __global float *stateVector, __global float *lossVector, 
        __global float *gainVector, __global float *netGainVector){
    
    int i = get_global_id(0);

    if(i < m)
        netGainVector[i] = gainVector[i] * stateVector[i] + lossVector[i];

}

__kernel void snpDetermineRules(int n, __global float *spikingVector, __global float *ruleMatches){
    
    int i = get_global_id(0);

    if( i < n ){
        if( ruleMatches[i] ){

            spikingVector[i] = 1; 

        }else{
            spikingVector[i] = 0;
        }
    }
}

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

__kernel void snpSetStates(int n, int m, __global float *configVector, __global float *spikingVector,
        __global float* rules, __global float* delays, __global float* lossVector,
        __global float* stateVector, __global float* transitionVector){

    int i = get_global_id(0);
    
    if(i < n){
        int index = (int)rules[i * 3] - 1;
        lossVector[index] = 0;

        if(spikingVector[i] == 1){
            lossVector[index] = rules[i * 3 + 2];
            rules[i * 3 + 1] = delays[i];
            stateVector[index] = 0;
            if(delays[i] == 0){
                transitionVector[i * (m + 1)] = 1;
                stateVector[index] = 1;
            }
        } else if(rules[i * 3 + 1] == 0){
            transitionVector[i * (m + 1)] = 1;
            stateVector[(int)rules[i * m]] = 1;
        }
    }

}

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
