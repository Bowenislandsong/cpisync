# Search Top repos
# check if there are two release versions
# Download Repo (both release versions)
# RCDS quota then Rsync
# record: (Repo Name, latest version, laste second version, RCDS_comm, RCDS_time, rsync_comm, rsync_time, RCDS_is_Better)


from github import Github
import requests
import git
import json
import os
import shutil
import subprocess
import time

Outh=("Bowenislandsong","180eb3be625d0f6d414585f3b1f1f7ae33df7822")
inst = 0

class rsync_sync:
    def __init__(self,org_fname,ed_fname):
        try:
            self.res = subprocess.check_output(["rsync", "-r", "--checksum", "--no-whole-file", "--stats", org_fname+"/", ed_fname+"/"],timeout=4000).decode("utf-8")
        except:
            self.res = ""
        self.xmit = 0
        self.recv = 0
        self.timeGen = 0
        self.timeTrans = 0
        self.total_size = 0
    
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
            elif s.startswith('Total file size: '):
                self.total_size = s[len('Total file size: '):].split(' ')[0]
        return [str(float(self.timeGen) + float(self.timeTrans)), str(int(self.xmit) + int(self.recv)), str(self.total_size)]

class RCDS_sync:
    def __init__(self,org_fname,ed_fname):
        try:
            self.res = subprocess.check_output(["./SCSync", "-intercpi", "-p="+str(inst) ,org_fname, ed_fname],timeout=4000).decode("utf-8")
        except:
            self.res = ""
        self.time = 0
        self.comm = 0
    
    def get_costs(self):
        s_res = self.res.split('\n')
        for s in s_res:
            if s.startswith('Total Number of Bytes Communicated: '):
                self.comm = s[len('Total Number of Bytes Communicated: '):]
            elif s.startswith('Total Time Elapsed: '):
                self.time = s[len('Total Time Elapsed: '):]

        return [str(self.time), str(self.comm)]


# clone
#  * check if we have done this repo
#  * check if it is popular star > 100
#  * check if it has at least 2 releases
#  * clone 2 releases (last two)
def getRepos(input_file):
    repo_list = []
    repos = json.load(open(input_file))
    if len(repos)==0:
        return repo_list
    for repo in repos["items"]:
        if repo["stargazers_count"] > 100 or repo["watchers_count"] > 100:
            repo_list.append(str(repo["full_name"]))
    writeWaitList(repo_list)
    json.dump({},open(input_file,'w'))
    return repo_list

def writeWaitList(repo):
    '''avoid duplicating from wait list of result list'''

    old_set = set()
    
    rfile = open("waitlist.txt","r")
    for name in rfile.readlines():
        old_set.add(str(name.strip('\n')))
    rfile.close()
    
    rfile = open("sync_repo_result.txt","r")
    for name in rfile.readlines():
        old_set.add(str(name.split(" ")[0]))
    rfile.close()
    
    if repo not in old_set:
        fwrite = open("waitlist.txt",'a')
        fwrite.write(repo+"\n")
        fwrite.close()

def cloneRepo(repo_name):
    folder_name = repo_name[repo_name.find("/")+1:]
    git.Git(os.environ['HOME']+"/Desktop/new").clone("https://github.com/"+repo_name+".git")

    repo = git.Repo(os.environ['HOME']+"/Desktop/new/"+folder_name)
    try:
        tags = sorted(repo.tags, key=lambda t: t.commit.committed_datetime)
    except:
        return False, "",""
    if len(tags)<2:
        shutil.rmtree(os.environ['HOME']+"/Desktop/new/"+folder_name,True)
        return False, "",""
    tag_new = tags[-1]
    tag_old = tags[-2]

    g = git.Git(os.environ['HOME']+"/Desktop/new/"+folder_name)
    g.checkout(tag_new)

    git.Git(os.environ['HOME']+"/Desktop/old").clone("https://github.com/"+repo_name+".git")
    g = git.Git(os.environ['HOME']+"/Desktop/old/"+folder_name)
    g.checkout(tag_old)

    return True, tag_old, tag_new

def deleteRepo(repo_name):
    folder_name = repo_name[repo_name.find("/")+1:]
    shutil.rmtree(os.environ['HOME']+"/Desktop/new/"+folder_name,True)
    shutil.rmtree(os.environ['HOME']+"/Desktop/old/"+folder_name,True)


def sync(repo_name):
    folder_name = repo_name[repo_name.find("/")+1:]
    new_path = os.environ['HOME']+"/Desktop/new/"+folder_name
    old_path = os.environ['HOME']+"/Desktop/old/"+folder_name
    [RCDS_time, RCDS_comm] = RCDS_sync(new_path,old_path).get_costs()
    [r_time, r_comm, total_size] = rsync_sync(new_path,old_path).get_costs()
    return RCDS_time, RCDS_comm, r_time, r_comm, total_size
    

def waitlist():
    list = []
    rfile = open("waitlist.txt","r")
    for name in rfile.readlines():
        list.append(str(name.strip('\n')))
    return list

def waitlistRM(repo_name):
    list = []
    rfile = open("waitlist.txt","r")
    for name in rfile.readlines():
        if str(name.strip('\n')) != repo_name:
            list.append(str(name))
    rfile.close()
    wfile = open("waitlist.txt","w")
    for name in list:
        wfile.write(name)
    wfile.close()

#Repo Name, latest version, laste second version, RCDS_comm, RCDS_time, rsync_comm, rsync_time, RCDS_is_Better
def report(repo,old_v,new_v,RCDS_time, RCDS_comm, r_time, r_comm, total_size):
    print([repo,old_v,new_v,RCDS_time, RCDS_comm, r_time, r_comm, total_size])
    res = open("sync_repo_result.txt","a")
    res.write(str(repo) + " " + str(old_v) + " " + str(new_v) + " " + str(RCDS_time) + " " +  str(RCDS_comm) + " " +  str(r_time) + " " +  str(r_comm)  + " " + str(total_size)+" "+  str(int(RCDS_comm)<int(r_comm))+"\n")
    res.close()

def update(entire_json):
    for i in range(len(entire_json["items"])):
        release_url = entire_json["items"][i]["releases_url"].strip("{/id}")
        r = requests.get(release_url,auth=Outh)
        json_res = r.json()
        if len(json_res)>= 2:
            writeWaitList(str(entire_json["items"][i]["full_name"]))
    
def searchRepo(key_word):
    repo_set = set()
    # r = requests.get('https://api.github.com/repositories?since='+last_id)
    r = requests.get('https://api.github.com/search/repositories?q='+key_word+'&sort=stars&order=desc&per_page=100&page=1',auth=Outh)
    res = r.json()
    update(res)
    # print(res["total_count"])
    if int(res["total_count"])<1000:
        return
    for i in range(2,11):
        r = requests.get('https://api.github.com/search/repositories?q='+key_word+'&sort=stars&order=desc&per_page=100&page='+str(i),auth=Outh)
        update(r.json())
    with open('sample_json.json', 'w') as outfile:
        json.dump(res, outfile)
    # print(res)


    fwrite = open("waitlist.txt",'a')
    for repo_name in repo_set:
        fwrite.write(repo_name+"\n")
    fwrite.close()






if __name__=="__main__":
    inst = input("instance number, first one 1:")
    inst = int(inst)
    while True:
        for key_word in range(1,1000):
            if(inst==1):
                searchRepo(str(key_word))
        # getRepos("sample_json.json")
        # break
            while len(waitlist())>0:
                try:
                    repo = waitlist()[0]
                    waitlistRM(repo)
                    iscloned,old_v,new_v = cloneRepo(repo)
                    if (not iscloned):
                        waitlistRM(repo)
                        continue
                    print("We cloned: "+repo)
                    RCDS_time, RCDS_comm, r_time, r_comm, total_size = sync(repo)
                    report(repo,old_v,new_v,RCDS_time, RCDS_comm, r_time, r_comm, total_size)
                    deleteRepo(repo)
                except:
                    print("We skipped one")
                    continue

