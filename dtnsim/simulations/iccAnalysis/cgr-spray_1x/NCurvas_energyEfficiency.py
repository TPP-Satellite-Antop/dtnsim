import matplotlib.pyplot as plt
import ast
import os

#OUTPUT
FILE_OUTPUT_PATH = os.getcwd() + "/results/energyEfficiency.pdf"

#INPUT. Complete path will be computed as DIRECTORY/FILE_PATHS_C1[i]
DIRECTORY = os.getcwd() + "/.."
FILE_PATHS_C1 = [
"4Nodes_RandomNetwork_cgr1_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr1_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr3_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr3_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_sprayAndWait_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_sprayAndWait_3copies_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_epidemic_1x/results/results_random/0.5/METRIC=appBundleReceived:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt"
]

FILE_PATHS_C2 = [
"4Nodes_RandomNetwork_cgr1_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr1_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr3_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_cgr3_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_sprayAndWait_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_sprayAndWait_3copies_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"4Nodes_RandomNetwork_epidemic_1x/results/results_random/0.5/METRIC=dtnBundleSentToCom:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt"
]

#Calculate complete path
COMPLETE_PATHS_C1 = list(map(lambda x: DIRECTORY + "/" + x, FILE_PATHS_C1))

#Calculate complete path
COMPLETE_PATHS_C2 = list(map(lambda x: DIRECTORY + "/" + x, FILE_PATHS_C2))

#Names of plotted functions

NAME_CURVAS_1 = ["$CGR-DelTime-NonFaultsAware$", "$CGR-DelTime-FaultsAware$",
"$CGR-Hops-NonFaultsAware$", "$CGR-Hops-FaultsAware$",
"$SprayAndWait-2$","$SprayAndWait-3$","$Epidemic$"]

#Color of ploted functions
COLOR_CURVAS_1 = ['blue','darkblue','green','darkgreen', 'orange','red', 'darkmagenta']

#Style of plotted function
STYLE_CURVAS_1 = ['--o','--s','--v','--p','--x','--+', '--.']

#Number of contact in each contact plan. It must correspond to FILE_PATHS
#NUMBER_OF_CONTACT = [10, 60, 120]

#Defines wich points plot in graph
#DOTS = [0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]
#RANGE = [[round(p * i) for p in DOTS] for i in NUMBER_OF_CONTACT

X_LABEL = "Proportion of Contacts with Faults"
Y_LABEL = "Energy Efficiency"

def main():
    c1s = [getListFromFile(fname) for fname in COMPLETE_PATHS_C1]
    c2s = [getListFromFile(fname) for fname in COMPLETE_PATHS_C2]

    # plot results

    lines = []
    for c1,c2,name1,color1,style1, in zip(c1s,c2s,NAME_CURVAS_1,COLOR_CURVAS_1,STYLE_CURVAS_1):
        delivered = [y[1] for y in c1]
        transmitted = [y[1] for y in c2]
        eff = [y1/y2 if y2 >0 else 0 for y1, y2 in zip(delivered, transmitted)]
        line_up, = plt.plot([x[0] for x in c1],eff,style1, color=color1, label=name1)
        lines.append(line_up)

    plt.legend(handles= lines)

    #plt.xlim([0.2,0.9])
    #plt.ylim([0.4,1.05])
    plt.xlabel(X_LABEL)
    plt.ylabel(Y_LABEL)
    plt.grid(color='gray', linestyle='dashed')
    plt.savefig(FILE_OUTPUT_PATH)
    plt.clf()
    plt.cla()
    plt.close()
    #plt.show()

''''
Get a list from file reading first line only
'''
def getListFromFile(path):
    with open(path) as f:
        lines = f.readlines()

    return ast.literal_eval(lines[0])

main()
