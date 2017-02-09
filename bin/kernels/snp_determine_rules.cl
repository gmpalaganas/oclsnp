typedef struct {
    float k;
    float j;
} regex_repr;

int fcompare(float a, float b){
    if( fabs(a - b) < FLT_EPSILON )
        return 0;
    else if( a - b > 0)
        return 1;
    return -1;
}


int match_regex_k(float len, regex_repr *repr){
    if( fcompare(len, repr->k) == 0 )
        return 1;
    return 0;
}

int match_regex_plus(float len, regex_repr *repr){
    float k = 0;

    if(fcompare(repr->k, -1.0f) == -1){
        k = fabs((repr->k + 1)/2);

        if(fcompare(len, k) == -1)
            return 0;

        len -= k;
    }

    if(fcompare(fmod(len,repr->j), 0.0f) == 0 &&
            fcompare(len, 0.0f) != 0)
        return 1;

    return 0;
}

int match_regex_star(float len, regex_repr *repr){
    float k = 0;

    if(fcompare(len, 0.0f) == 0)
        return 0;

    if(fcompare(repr->k, 0.0f) == -1){
        k = fabs((repr->k)/2);

        if(fcompare(len, k) == -1)
            return 0;

        len -= k;
    }

    if(fcompare(fmod(len,repr->j), 0.0f) == 0)
        return 1;

    return 0;
}

int match_regex(float len, regex_repr *repr){
    if(fcompare(repr->k, 0.0f) == 1)
        return match_regex_k(len, repr);
    else if(fmod(repr->k, 2) == 0)
        return match_regex_star(len, repr);
    return match_regex_plus(len, repr);
}

__kernel void snpDetermineRules(int n, int m,  __global float *spikingVector, __global float *configVector, __global float *rules,  __global regex_repr *repr) {
    printf("here\n");
    
    int i = get_global_id(0);

    if( i < n ){
        regex_repr holder = repr[i];
        if( match_regex(configVector[(int)(rules[i * 3] - 1)],&holder) ){
            spikingVector[i] = 1; 

        }else{
            spikingVector[i] = 0;
        }
    }
}
