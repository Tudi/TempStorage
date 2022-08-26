from os import listdir
from os.path import isfile, join
import os
import subprocess

csv_separator = ","
hasher_path = "..\\perceptual_hash\\ImageHash\\x64\\Release\\ImageHash.exe"
mypath = "..\\page_fetcher\\images\\cryptopunks\\"
compareto = ["..\\image_manip\\blurr\\","..\\image_manip\\brightness\\","..\\image_manip\\colorswap\\","..\\image_manip\\move\\","..\\image_manip\\resize\\","..\\image_manip\\rotated\\"]
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]

csv_header = "file"
for reference_dir in compareto:
    words = reference_dir.split("\\")
    csv_header = csv_header + csv_separator + words[len(words)-2]
    
f = open("detect_match_results.csv", "wt")
f.write(csv_header+"\n")
    
for image_file in onlyfiles:
    csvRow = image_file
    for reference_dir in compareto:
        f1 = mypath + image_file
        f2 = reference_dir + image_file
#        print("compare " + f1 + " to " + f2)
        result = subprocess.run([hasher_path, f1, f2], stdout=subprocess.PIPE)
#        print(result.stdout.decode('ascii'))
        words = result.stdout.decode('ascii').split(" ")
        csvRow = csvRow + csv_separator + words[0]
#    print(csvRow)
    f.write(csvRow+"\n")
    print("done : " + image_file)
#    os.exit

f.close()