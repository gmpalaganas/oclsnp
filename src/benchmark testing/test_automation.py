import subprocess
import re

cmd2 = "./oclsnp ../inputs/2input_sort.bin"
cmd4 = "./oclsnp ../inputs/4input_sort.bin"
cmd8 = "./oclsnp ../inputs/8input_sort.bin"
cmd16 = "./oclsnp ../inputs/16input_sort.bin"

f2 = open('ocl_test_2_sort_output.txt', 'w')
f4 = open('ocl_test_4_sort_output.txt', 'w')
f8 = open('ocl_test_8_sort_output.txt', 'w')
f16 = open('ocl_test_16_sort_output.txt', 'w')

f2_mem = open('ocl_test_2_sort_output_mem.txt', 'w')
f4_mem = open('ocl_test_4_sort_output_mem.txt', 'w')
f8_mem = open('ocl_test_8_sort_output_mem.txt', 'w')
f16_mem = open('ocl_test_16_sort_output_mem.txt', 'w')


print("Running Parallel 2 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd2, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f2.write('{0:.10f}'.format(float(time.upper())))
    f2.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f2_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f2_mem.write(',')

print("Running Parallel 4 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd4, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f4.write('{0:.10f}'.format(float(time.upper())))
    f4.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f4_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f4_mem.write(',')


print("Running Parallel 8 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd8, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f8.write('{0:.10f}'.format(float(time.upper())))
    f8.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f8_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f8_mem.write(',')


print("Running Parallel 16 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd16, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f16.write('{0:.10f}'.format(float(time.upper())))
    f16.write(',')


    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f16_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f16_mem.write(',')

f2 = open('lin_test_2_output.txt', 'w')
f4 = open('lin_test_4_output.txt', 'w')
f8 = open('lin_test_8_output.txt', 'w')
f16 = open('lin_test_16_output.txt', 'w')


f2_mem = open('lin_test_2_sort_output_mem.txt', 'w')
f4_mem = open('lin_test_4_sort_output_mem.txt', 'w')
f8_mem = open('lin_test_8_sort_output_mem.txt', 'w')
f16_mem = open('lin_test_16_sort_output_mem.txt', 'w')

cmd2 = "./linsnp ../inputs/2input_sort.bin"
cmd4 = "./linsnp ../inputs/4input_sort.bin"
cmd8 = "./linsnp ../inputs/8input_sort.bin"
cmd16 = "./linsnp ../inputs/16input_sort.bin"

print("Running Linear 2 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd2, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f2.write('{0:.10f}'.format(float(time.upper())))
    f2.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f2_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f2_mem.write(',')

print("Running Linear 4 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd4, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f4.write('{0:.10f}'.format(float(time.upper())))
    f4.write(',')


    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f4_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f4_mem.write(',')

print("Running Linear 8 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd8, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f8.write('{0:.10f}'.format(float(time.upper())))
    f8.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f8_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f8_mem.write(',')

print("Running Linear 16 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd16, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f16.write('{0:.10f}'.format(float(time.upper())))
    f16.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f16_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f16_mem.write(',')

cmd2 = "./oclsnp ../inputs/2input_gen_sort.bin"
cmd4 = "./oclsnp ../inputs/4input_gen_sort.bin"
cmd8 = "./oclsnp ../inputs/8input_gen_sort.bin"
cmd16 = "./oclsnp ../inputs/16input_gen_sort.bin"

f2 = open('ocl_test_2_gen_sort_output.txt', 'w')
f4 = open('ocl_test_4_gen_sort_output.txt', 'w')
f8 = open('ocl_test_8_gen_sort_output.txt', 'w')
f16 = open('ocl_test_16_gen_sort_output.txt', 'w')

f2_mem = open('ocl_test_2_gen_sort_output_mem.txt', 'w')
f4_mem = open('ocl_test_4_gen_sort_output_mem.txt', 'w')
f8_mem = open('ocl_test_8_gen_sort_output_mem.txt', 'w')
f16_mem = open('ocl_test_16_gen_sort_output_mem.txt', 'w')

print("Running Parallel 2 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd2, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f2.write('{0:.10f}'.format(float(time.upper())))
    f2.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f2_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f2_mem.write(',')

print("Running Parallel 4 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd4, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f4.write('{0:.10f}'.format(float(time.upper())))
    f4.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f4_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f4_mem.write(',')


print("Running Parallel 8 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd8, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f8.write('{0:.10f}'.format(float(time.upper())))
    f8.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f8_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f8_mem.write(',')


print("Running Parallel 16 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd16, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f16.write('{0:.10f}'.format(float(time.upper())))
    f16.write(',')


    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f16_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f16_mem.write(',')

f2 = open('lin_test_2_gen_sort_output.txt', 'w')
f4 = open('lin_test_4_gen_sort_output.txt', 'w')
f8 = open('lin_test_8_gen_sort_output.txt', 'w')
f16 = open('lin_test_16_gen_sort_output.txt', 'w')


f2_mem = open('lin_test_2_gen_sort_output_mem.txt', 'w')
f4_mem = open('lin_test_4_gen_sort_output_mem.txt', 'w')
f8_mem = open('lin_test_8_gen_sort_output_mem.txt', 'w')
f16_mem = open('lin_test_16_gen_sort_output_mem.txt', 'w')

cmd2 = "./linsnp ../inputs/2input_gen_sort.bin"
cmd4 = "./linsnp ../inputs/4input_gen_sort.bin"
cmd8 = "./linsnp ../inputs/8input_gen_sort.bin"
cmd16 = "./linsnp ../inputs/16input_gen_sort.bin"

print("Running Linear 2 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd2, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 
    
    f2.write('{0:.10f}'.format(float(time.upper())))
    f2.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f2_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f2_mem.write(',')

print("Running Linear 4 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd4, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f4.write('{0:.10f}'.format(float(time.upper())))
    f4.write(',')


    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f4_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f4_mem.write(',')

print("Running Linear 8 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd8, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f8.write('{0:.10f}'.format(float(time.upper())))
    f8.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f8_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f8_mem.write(',')

print("Running Linear 16 input")
for i in range(0,30):
    proc = subprocess.Popen(cmd16, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=r'../../bin')

    out, err = proc.communicate()

    holder, holder2, time = out.partition('Execution time: ')
    time, holder2, holder = time.partition('ns') 

    f16.write('{0:.10f}'.format(float(time.upper())))
    f16.write(',')

    holder, holder, memUsage = out.partition('Memory Usage: ')
    memUsage, holder2, holder = memUsage.partition('kb')
    f16_mem.write('{0:.10f}'.format(float(memUsage.upper())))
    f16_mem.write(',')
