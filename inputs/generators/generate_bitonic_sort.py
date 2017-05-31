from random import randint
from collections import namedtuple
from math import log
import re

Neuron = namedtuple("Neuron", "name rules outgoing spikes")
Rule = namedtuple("Rule", "regex consumed produced delay")

def main():
    
    for i in range(0,9):
        inputSize = pow(2,i + 1)
        f = open("../texts/{}_input_bitonic_sort.txt".format(inputSize) ,'w+')
        print 'Generating %d input bitonic sort' % inputSize
        network = generateNeuronsList(inputSize)

        generateNetwork(network,inputSize)

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


def generateNetwork(neurons,n):
    pairs = generatePairs(n)
    i = 0
    j = 0
    while (i + 2) < len(neurons):
        for pair in pairs[j]:
            in_layer = [
                    neurons[i][pair[0]],
                    neurons[i][pair[1]],
                    ]
            mid_layer = [
                    neurons[i+1][pair[0]],
                    neurons[i+1][pair[1]],
                    ]
            out_layer = [
                    neurons[i+2][pair[0]],
                    neurons[i+2][pair[1]],
                    ]
            comparator = [in_layer, mid_layer, out_layer]
            generateComparator(comparator, pair[2])
        i += 2
        j += 1


def generateNeuronsList(n):
    neurons = []
    k = int(log(n,2))
    nLayers = 1 + sum( 2 * i for i in range (1,k+1))

    for i in range(nLayers):
        layer = generateNetLayer(i,n)
        neurons.append(layer)

    return neurons


def generateNetLayer(l,n):
    name = 'mid%d' % l
    k = int(log(n,2))

    if l == 0:
        name = 'in'
    elif l == sum(2 * i for i in range (1, k+1)):
        name = 'out'

    layer = []

    for i in range(n):
        spikes = 0
        if name == 'in':
            spikes = randint(1,100)
        neuron = Neuron(name + "{%d}" % i,[],[],spikes)
        layer.append(neuron)

    return layer

def generateComparator(comparator,increasing=True):
    inputLayer = comparator[0]

    for index, neuron in enumerate(inputLayer):
        rule = Rule('a+',1,1,0)
        neuron.rules.append(rule)
        neuron.outgoing.extend([neuron.name for neuron in comparator[1]])
   
    type1Index = 0 if increasing else 1
    type2Index = 1 if increasing else 0

    type1 = comparator[1][type1Index]
    type2 = comparator[1][type2Index]

    type1_rules = [
            Rule('a^2',2,1,0),
            Rule('a',1,0,0)
            ]

    type2_rules = [
            Rule('a^2',2,0,0),
            Rule('a',1,1,0)
            ]

    type1.rules.extend(type1_rules)
    type1.outgoing.extend([neuron.name for neuron in comparator[2]])

    type2.rules.extend(type2_rules)
    type2.outgoing.append(comparator[2][type2Index].name)

def generatePairs(n):
    pairs = []
    k = int(log(n,2))
    for i in range(1, k+1):
        for j in reversed(range(1, i+1)):
            lPairs = []
            inc = 2 ** (j-1)
            explored = []
            counter = 0
            increasing = True
            for ind in range(0, n):
                if counter == ( 2**i ):
                    increasing = not increasing
                    counter = 0
                if ind not in explored:
                    lPairs.append((ind, ind+inc, increasing))
                    explored.append(ind + inc)
                counter += 1
            pairs.append(lPairs)
    return pairs
   
def getNeuronID(name, network):
    ret = -1
    
    for i,neuron in enumerate(network):
        if neuron.name == name:
            ret = i
            break

    return ret

def closeFiles(files):
    for f in files:
        f.close()

if __name__ == '__main__':
    main()
