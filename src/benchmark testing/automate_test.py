import subprocess
import re


def measureProg(command,output_file):
    print command
    proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    output_file.write('{0:.10f}'.format(float(time.upper())))
    output_file.write('\n')


def main():
    oclsnp = './oclsnp'
    linsnp = './linsnp'

    input_dir = '../inputs/texts/'

    bitonic_files = '%d_input_bitonic_sort.txt'
    general_files = '%d_input_gen_sort.txt'

    ocl_bitonic_time_output = 'ocl_%d_input_bitonic_time.txt'
    ocl_general_time_output = 'ocl_%d_input_gen_time.txt'

    lin_bitonic_time_output = 'lin_%d_input_bitonic_time.txt'
    lin_general_time_output = 'lin_%d_input_gen_time.txt'


    command_template = '%s ' + input_dir +'%s --txt'

    for i in range(0,1):
        inputSize = pow(2,i+1)
        bitonic_input = bitonic_files % inputSize
        general_input = general_files % inputSize

        ocl_bitonic_output = open(ocl_bitonic_time_output % inputSize,'w+')
        ocl_general_output = open(ocl_general_time_output % inputSize,'w+')

        lin_bitonic_output = open(lin_bitonic_time_output % inputSize,'w+')
        lin_general_output = open(lin_general_time_output % inputSize,'w+')

        ocl_bitonic_command = command_template % (oclsnp,bitonic_input)
        ocl_general_command = command_template % (oclsnp,general_input)

        lin_bitonic_command = command_template % (linsnp,bitonic_input)
        lin_general_command = command_template % (linsnp,general_input)
        
        print "Running %d input parallel bitonic"
        for i in range(0,3):
            measureProg(ocl_bitonic_command,ocl_bitonic_output)

        print "Running %d input parallel general"
        for i in range(0,3):
            measureProg(ocl_general_command,ocl_general_output)

        print "Running %d input linear bitonic"
        for i in range(0,3):
            measureProg(lin_bitonic_command,lin_bitonic_output)

        print "Running %d input linear general"
        for i in range(0,3):
            measureProg(lin_general_command,lin_general_output)                               

if __name__ == '__main__':
    main()
