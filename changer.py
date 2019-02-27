import csv

rpath = "./Sets of Content IBLTSyncSetDiff RandText (lvl 4) HEAP.txt"

content = []
with open(rpath, 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        if len(row) < 12:
            content.append(row)
replacepath = "./S4.txt"
content_replacement = []
with open(replacepath, 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        if len(row) < 11:
            content_replacement.append(row)

strsize = [100000, 400000, 800000, 1200000, 1600000, 2000000]
edits = [0.00001, 0.0001, 0.001, 0.01, 0.1, 1]


c = 100
cit = 0
rit = 0
for st in strsize:
    for ed in edits:
        for i in range(100):
            if(ed > 0.01):
                tmp = content_replacement[rit]
                tmp.extend([0,0])
                content.insert(cit,tmp)
            
            cit +=1
            rit +=1



newf = open("Sets of Content IBLTSyncSetDiff RandText lvl 4 HEAP.txt", 'a')
newf.write("String|Edit ratio|Comm (bytes)|Actual Sym Diff|Time Tree(s)|Time Recon(s)|Time Backtrack (included in Time Recon) (s)|Str Recon True|Tree Heap SIze|High Water Heap\n")
for row in content:
    for item in row:
        newf.write(str(item)+ " ")
    newf.write("\n")
newf.close()


    