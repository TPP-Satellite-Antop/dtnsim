import matplotlib.pyplot as plt
import ast
import os

#OUTPUT
FILE_OUTPUT_PATH = os.getcwd() + "/results/results_centrality/centrality_bundles_Re-Ruteados.pdf"

#INPUT. Complete path will be computed as DIRECTORY/FILE_PATHS_C1[i]
DIRECTORY = os.getcwd() + "/results/results_centrality"
FILE_PATHS_C1 = ["1.0/METRIC=dtnBundleReRouted:count-DENSITY=1-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=120.txt",
"0.5/METRIC=dtnBundleReRouted:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=60.txt",
"0.1/METRIC=dtnBundleReRouted:count-DENSITY=0.1-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=true-MAX_DELETED_CONTACTS=10.txt"]

FILE_PATHS_C2 = ["1.0/METRIC=dtnBundleReRouted:count-DENSITY=1-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=120.txt",
"0.5/METRIC=dtnBundleReRouted:count-DENSITY=0.5-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=60.txt",
"0.1/METRIC=dtnBundleReRouted:count-DENSITY=0.1-AMOUNT_CONTACT_PLANS=10-FAULTAWARE=false-MAX_DELETED_CONTACTS=10.txt"]

#Calculate complete path
COMPLETE_PATHS_C1 = list(map(lambda x: DIRECTORY + "/" + x, FILE_PATHS_C1))
COMPLETE_PATHS_C2 = list(map(lambda x: DIRECTORY + "/" + x, FILE_PATHS_C2))

#Names of plotted functions
NAME_CURVAS_1 = ["$\delta = 1.0, Faults Aware$","$\delta = 0.5, Faults Aware$", "$\delta = 0.1, Faults Aware$"]
NAME_CURVAS_2 = ["$\delta = 1.0, Non Faults Aware$","$\delta = 0.5, Non Faults Aware$", "$\delta = 0.1, Non Faults Aware$"]

#Color of ploted functions
COLOR_CURVAS_1 = ['blue','blue','blue']
COLOR_CURVAS_2 = ['orange','orange','orange']

#Style of plotted function
STYLE_CURVAS_1 = ['--o','--s','--v']
STYLE_CURVAS_2 = ['--*','--x','--+']

#Number of contact in each contact plan. It must correspond to FILE_PATHS
#NUMBER_OF_CONTACT = [10, 60, 120]

#Defines wich points plot in graph
#DOTS = [0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]
#RANGE = [[round(p * i) for p in DOTS] for i in NUMBER_OF_CONTACT

X_LABEL = "Proporción de Contactos con Fallas"
Y_LABEL = "Bundles Re-Ruteados"

def main():
    c1s = [getListFromFile(fname) for fname in COMPLETE_PATHS_C1]
    c2s = [getListFromFile(fname) for fname in COMPLETE_PATHS_C2]

    # plot results

    lines = []
    for c1,c2,name1,name2,color1,color2,style1,style2, in zip(c1s,c2s,NAME_CURVAS_1,NAME_CURVAS_2,COLOR_CURVAS_1,COLOR_CURVAS_2,STYLE_CURVAS_1,STYLE_CURVAS_2):
        line_up, = plt.plot([x[0] for x in c1],[y[1] for y in c1],style1, color=color1, label=name1)
        line_down, = plt.plot([x[0] for x in c2],[y[1] for y in c2],style2, color=color2, label=name2)
        lines.append(line_up)
        lines.append(line_down)

    plt.legend(handles= lines)


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
