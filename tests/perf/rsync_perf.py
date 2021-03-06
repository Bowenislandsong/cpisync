# Create text files with edit burst distance upperbound
# Use rsync and capture communcation, time, and auxialry space cost.
# Use Commandline to avoid unintentinal overhead
# by Bowen Song in Feb 14,2019

# Spicify or load Paramters
#fName # original File name
#strSize # size of the original string
#Edist # edit distnace upper bounds

# THIS CODE IS NOT USED SINCE PYTHON RANDOM HAS A DIFFERENT DISTRIBUTION THAN THAT OF C++ RAND()

import random
import subprocess


class fileIO:
    ''' load, create, edit, compare'''
    def __init__(self,f_name):
        self.f_name = f_name
        f = open(self.f_name, "r")
        self.content = f.read()
        f.close()

        prefix = self.f_name.split(".")[0] # assume one dot in a txt file
        self.org_f_name = prefix+"org.txt"
        self.ed_f_name = prefix+"ed.txt"
    
    def createEdfiles(self,f_size,edist):
        '''creat file A and B '''
        if edist > f_size:
            edist = f_size
            print("edit burst distance can not be bigger than original string size. We will do full edits then")
        f = open(self.org_f_name,'w')
        start = random.randint(0,len(self.content)-edist-1)
        f.write(self.content[start:start+edist])
        f.close()

        f = open(self.ed_f_name,'w')
        f.write(self.editbursts(self.content[start:start+edist],edist))
        f.close()


    def editbursts(self,org_content,edist):
        '''create random burst edits'''
        while (edist>0):
            per_ed = random.randint(1,edist) # random number of burst random length
            edist-=per_ed
            start = random.randint(0,len(org_content)-per_ed-1)
            if (random.randint(0,1)==0): # we insert
                start_ed = random.randint(0,len(org_content)-per_ed-1)
                insert_txt = org_content[start_ed:start_ed+per_ed]
                org_content = org_content[:start] + insert_txt + org_content[start:]
            else: # we delete
                org_content = org_content[:start] + org_content[start+per_ed:]
        return org_content


class rsync_com:
    def __init__(self,org_fname,ed_fname):
        self.res = subprocess.check_output(["rsync", "-v", "--stats", "--progress", org_fname, ed_fname]).decode("utf-8")
        self.xmit = 0
        self.recv = 0
        self.timeGen = 0
        self.timeTrans = 0
    
    def get_costs(self):
        s_res = self.res.split('\n')
        for s in s_res:
            if s.startswith('File list generation time: '):
                self.timeGen = s[len('File list generation time: '):].split(' ')[0]
            elif s.startswith('File list transfer time: '):
                self.timeTrans = s[len('File list transfer time: '):].split(' ')[0]
            elif s.startswith('Total bytes sent: '):
                self.xmit = s[len('Total bytes sent: '):]
            elif s.startswith('Total bytes received: '):
                self.recv = s[len('Total bytes received: '):]
        return [self.timeGen, self.timeTrans, self.xmit, self.recv]
            

if __name__== "__main__":
    fName = "SampleTxt.txt"
    strSize = 2000000
    Edist = 2000

    f = fileIO(fName)
    f.createEdfiles(strSize,Edist)
    sync = rsync_com("SampleTxtorg.txt","SampleTxted.txt")
    print(sync.get_costs())
# Load target file
# create original file of size A
# Make bounded edit burst
# create edited file 
# use rsync for sync and Capture cost
# Load the new file again and compare