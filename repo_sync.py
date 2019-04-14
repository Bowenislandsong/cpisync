# Search Top repos
# check if there are two release versions
# Download Repo (both release versions)
# RCDS quota then Rsync
# record: (Repo Name, latest version, laste second version, RCDS_comm, RCDS_time, rsync_comm, rsync_time, url)


from github import Github

class repo:
    def __init__(self,cred):
        self.g = Github(cred)

    # Search Top repos
    def searchRepo(self,search_it):
        repositories = set()
        content_files = self.g.search_code(query=search_it)
        for content in content_files:
            repositories.add(content.repository.full_name)
            rate_limit = self.g.get_rate_limit()
            if rate_limit.search.remaining == 0 or len(repositories)>5:
                break
        return repositories
        
    def releaseVer(repo_name):
        addr = "www.github.com/"+repo_name


if __name__=="__main__":
    r = repo('418bab5f9af51e73c07de6eb163fecd81ea83e72')
    repos = r.searchRepo('discover')
    for repo in repos:
        print(repo)
        # if(releaseVer(repo)):
            # print(releaseVer(repo))