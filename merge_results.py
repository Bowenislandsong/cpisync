import sys


def get_file(file_name):
    f = open(file_name,'r')
    content = f.readlines()
    f.close()
    return content

def write_file(file_name,content):
    f = open(file_name,'w')
    f.writelines(content)
    f.close()


if __name__ == "__main__":

    filenames = sys.argv

    all_contents = []
    for i in range(1,len(filenames)):
        all_contents.append(get_file(filenames[i]))

    filelen = 1
    # they should all have the same length
    for i in range(1,len(all_contents)):
        if len(all_contents[i]) != len(all_contents[i-1]):
            print "These files are not the same length"
            exit()
        filelen = len(all_contents[i])

    newfile = [all_contents[0][0]]
    for i in range (1,filelen):
        for j in range(len(all_contents)):
            newfile.append(all_contents[j][i])

    write_file(filenames[-1].split('.')[0][:-1]+".txt",newfile)
    