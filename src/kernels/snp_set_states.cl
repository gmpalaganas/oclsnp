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
