import matplotlib.pyplot as plt
from math import *
import random as rd

import sys

numberGW=sys.argv[1]
numberEN=sys.argv[2]
numberGS=sys.argv[3]

#Check that arguments are valid
if sys.argv[1]==None or sys.argv[2]==None or sys.argv[3]==None:
    print("You have to specify 3 arguments ")
    sys.exit()

if int(numberGW) > 12 or int(numberGW) < 1:
    print("Your number of GW is not correct ")
    sys.exit()
if int(numberEN) > 100 or int(numberEN) < 1:
    print("Your number of EN is not correct ")
    sys.exit()
if int(numberGS) > 12 or int(numberGS) < 1:
    print("Your number of GS is not correct ")
    sys.exit()

## TODO change path
input = open("contactPlan/Delta.txt", "r")
output = open("contactPlan/Custom.txt","w")
input_Lines=input.readlines()
input.close()


#Calculation of the GW to be removed
GW=[]
if (int(numberGW)==12):
    GW=[i for i in range(1,13)]
else:
    while len(GW)!=int(numberGW):
        i=rd.randint(1,12)
        if i not in GW:
            GW.append(i)

#Calculation of the EN to be removed
EN=[]
if (int(numberEN)==100):
    EN=[i for i in range(13,113)]
else:
    while len(EN)!=int(numberEN):
        i=rd.randint(13,112)
        if i not in EN:
            EN.append(i)

#Calculation of the GS to be removed
GS=[]
if (int(numberGS)==10):
    GS=[i for i in range(113,123)]
else:
    while len(GS)!=int(numberGS):
        i=rd.randint(113,122)
        if i not in GS:
            GS.append(i)

GW.sort()
EN.sort()
GS.sort()

nodesKept=GW+EN+GS

output.write("# Satellites ID : %s \n"%(str(GW)))
output.write("# EN ID : %s \n"%(str(EN)))
output.write("# GS ID : %s \n"%(str(GS)))


for line in input_Lines:
    for i in nodesKept:
        for j in nodesKept:
            if " %s %s "%(str(i),str(j)) in line:
                 output.write(line) 

for i in GS:
    output.write("a contact +0 +86400 %s 123 1000000 \n"%(str(i)))

output.close()
