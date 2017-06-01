import subprocess
import re

BITONIC = 'bitonic'
GENERAL = 'general'

PARALLEL = 'oclsnp portable'
PARALLEL_OLD = 'oclsnp nvidia'
LINEAR = 'linsnp (C++14)'

def measureProg(command,output_file,algo,inputSize,parallel):
    print command
    proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ms') 

    output_file.write(str(algo))
    output_file.write(',')
    output_file.write(str(inputSize))
    output_file.write(',')
    output_file.write(str(parallel))
    output_file.write(',')
    output_file.write('{0:.10f}'.format(float(time.upper())))
    output_file.write('\n')


def main():
    start = input("start: ")
    end =input("end: ")
    results = open('measurements2.txt','w+')
    oclsnp = './oclsnp'
    linsnp = './linsnp'
    oclsnp_old = './oclsnp_old'

    input_dir = '../inputs/texts/'
    results.write('algo,inputs,type,time')

    command_template = '%s ' + input_dir +'%s --txt --silent'

    for i in range(start,end+1):
        inputSize = pow(2,i+1)


        bitonic_input = '%d_input_bitonic_sort.txt' % inputSize
        general_input = '%d_input_gen_sort.txt' % inputSize

        ocl_bitonic_command = command_template % (oclsnp,bitonic_input)
        ocl_general_command = command_template % (oclsnp,general_input)

        ocl_old_bitonic_command = command_template % (oclsnp_old,bitonic_input)
        ocl_old_general_command = command_template % (oclsnp_old,general_input)

        lin_bitonic_command = command_template % (linsnp,bitonic_input)
        lin_general_command = command_template % (linsnp,general_input)

        t = 1 if inputSize > 64 else 30
        
        if inputSize < 32:
            print "Running %d input parallel bitonic" % inputSize
            for i in range(0,t):
                measureProg(ocl_bitonic_command,results,BITONIC,inputSize,PARALLEL)

        if inputSize < 64:
            print "Running %d input parallel general" % inputSize
            for i in range(0,t):
                measureProg(ocl_general_command,results,GENERAL,inputSize,PARALLEL)
 
        if inputSize < 128:
            print "Running %d input parallel old bitonic" % inputSize
            for i in range(0,t):
                measureProg(ocl_old_bitonic_command,results,BITONIC,inputSize,PARALLEL_OLD)

        if inputSize < 128:
            print "Running %d input parallel old general" % inputSize
            for i in range(0,t):
                measureProg(ocl_old_general_command,results,GENERAL,inputSize,PARALLEL_OLD)

        print "Running %d input linear bitonic" % inputSize
        for i in range(0,t):
            measureProg(lin_bitonic_command,results,BITONIC,inputSize,LINEAR)
        
        print "Running %d input linear general" % inputSize
        for i in range(0,t):
            measureProg(lin_general_command,results,GENERAL,inputSize,LINEAR)                               
        
    results.close()



if __name__ == '__main__':
    main()
