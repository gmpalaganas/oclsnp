from random import randint
from collections import namedtuple
import re

Neuron = namedtuple("Neuron", "name rules outgoing spikes")
Rule = namedtuple("Rule", "regex consumed produced delay")

def main():

    for i in range(0,9):
        n = pow(2,i+1)
        network = generateNeuronList(n)
        f = open("../texts/{}_input_gen_sort.txt".format(n) ,'w+')
        print 'Generating %d input general sort' % n
        generateNetwork(network,n)
        generateRules(network,n)

        neurons = []
        for layer in network:
            neurons.extend(layer)
        rules_count = sum([len(x.rules) for x in neurons]);

        f.write('%d\n' % rules_count)
        f.write('%d\n' % len(neurons))

        initConfig = ' '.join(str(x.spikes) for x in neurons)
        f.write('%s\n' % initConfig)

        for i,neuron in enumerate(neurons):
            for rule in neuron.rules:
                outs = (
                        i+1,
                        rule.regex,
                        rule.consumed,
                        rule.produced,
                        rule.delay
                        )
                f.write('%d %s %d %d %d\n' % outs) 

        for neuron in neurons:
            f.write('%s\n' % neuron.name)

        for i in range(0, len(neurons) + 1):
            if i == 0:
                f.write('-1\n')
            else:
                outgoing = [] 
                neuron = neurons[i-1]
                for out in neuron.outgoing:
                    index = getNeuronID(out,neurons)
                    outgoing.append(index+1)

                if len(outgoing) == 0:
                    outgoing.append(-1)

                for out in outgoing:
                    f.write("%d " % out)
                
                f.write('\n')

        f.close()

def generateNeuronList(inputSize):
    neurons = [[],[],[]]
    
    for i in range(0,inputSize):
        name = 'in{%d}' % i
        spikes = randint(1,100)
        neuron = Neuron(name, [], [], spikes)
        neurons[0].append(neuron)

        name = 'mid{%d}' % i
        neuron = Neuron(name, [], [], 0)
        neurons[1].append(neuron)

        name = 'out{%d}' % i
        neuron = Neuron(name, [], [], 0)
        neurons[2].append(neuron)
    
    return neurons

def generateNetwork(neurons,n):
    layer = neurons[0]
    for neuron in layer:
        for i in range(0,n):
            neuron.outgoing.append('mid{%d}' % i)

    layer = neurons[1]
    for i,neuron in enumerate(layer):
        for j in range(0, n-i):
            neuron.outgoing.append('out{%d}' % (i+j))

def generateRules(neurons,n):
    layer = neurons[0]
    for neuron in layer:
        rule = Rule('a*', 1, 1, 0)
        neuron.rules.append(rule)

    layer = neurons[1]
    for i,neuron in enumerate(layer):
        for j in range(0, n):
            regex = 'a'
            regex += '^%d' % (n-j) if (n-j) > 1 else ''
            rule = Rule(regex, n-j, 1 if i == j else 0, 0)
            neuron.rules.append(rule)
 
def getNeuronID(name, network):
    ret = -1
    
    for i,neuron in enumerate(network):
        if neuron.name == name:
            ret = i
            break

    return ret

if __name__ == '__main__':
    main()
