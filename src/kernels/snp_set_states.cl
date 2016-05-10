__kernel void snpSetStates(int n, int m, __global float *configVector, __global float *spikingVector,
        __global float* rules, __global float* delays, __global float* lossVector,
        __global float* stateVector, __global float* transitionVector, __local float *buffer){

    int i = get_global_id(0);
    int j = get_local_id(1);

    buffer[j * 3] = rules[i * 3];
    buffer[j * 3 + 1] = rules[i * 3 + 1];
    buffer[j * 3 + 2] = rules[i * 3 + 2];

    if(i < n){
        int index = (int)buffer[j * 3] - 1;
        lossVector[index] = 0;

        if(spikingVector[i] == 1){
            lossVector[index] = buffer[j * 3 + 2];
            rules[i * 3 + 1] = delays[i];
            stateVector[index] = 0;
            if(delays[i] == 0){
                transitionVector[i * (m + 1)] = 1;
                stateVector[index] = 1;
            }
        } else if(buffer[j * 3 + 1] == 0){
            transitionVector[i * (m + 1)] = 1;
            stateVector[(int)buffer[j * m]] = 1;
        }
    }

}
