from os import listdir
from os.path import isfile, join
import os
import subprocess

csv_separator = ","
hasher_path = "..\\perceptual_hash\\ImageHash\\x64\\Release\\ImageHash.exe"
mypath = "..\\page_fetcher\\images\\cryptopunks\\"
compareto = ["..\\page_fetcher\\images\\cryptopunks\\","..\\image_manip\\blurr\\","..\\image_manip\\brightness\\","..\\image_manip\\colorswap\\","..\\image_manip\\move\\","..\\image_manip\\resize\\","..\\image_manip\\rotated\\"]
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]

csv_header = "file"
for reference_dir in compareto:
    words = reference_dir.split("\\")
    csv_header = csv_header + csv_separator + words[len(words)-2]
    
skip_first_N=102
if skip_first_N > 0:
    f = open("false_positive_stats.csv", "at")
else:
    f = open("false_positive_stats.csv", "wt")
f.write(csv_header+"\n")
    
treshold_consider_match = 80
rows_processed = 0
for image_file1 in onlyfiles:
    compared_count = 0
    greater_than_treshold_count = 0
    csvRow = image_file1
    rows_processed = rows_processed + 1
    if skip_first_N > 0:
        skip_first_N = skip_first_N - 1
        continue
    for reference_dir in compareto:
        f1 = mypath + image_file1
        for image_file2 in onlyfiles:
            if image_file1 == image_file2:
                continue;               
            f2 = reference_dir + image_file2
#            print("compare " + f1 + " to " + f2)
            result = subprocess.run([hasher_path, f1, f2], stdout=subprocess.PIPE)
#           print(result.stdout.decode('ascii'))
            words = result.stdout.decode('ascii').split(" ")
            match_chance = float(words[0])
            if match_chance >= treshold_consider_match:
                greater_than_treshold_count = greater_than_treshold_count + 1
            compared_count = compared_count + 1
        false_positive_chance = match_chance * 100 / compared_count
        csvRow = csvRow + csv_separator + str(false_positive_chance)
        print("done : " + image_file1 + " in dir " + reference_dir + " row now " + csvRow)
#        print(csvRow)
#        os.exit
    f.write(csvRow+"\n")
    print( str(rows_processed) + ")done : " + image_file1 + " csv row : " + csvRow)
#    os.exit

f.close()