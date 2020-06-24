def grepinto(text, infile, outfile):
    """
    Auto generated function documentation. You can read this with grepinto.__doc__
    :param text:
    :param infile:
    :param outfile:
    :return:
    """
    with open(infile, "rt") as fin, open(outfile, "wt") as fout:
        for line in fin:
            if text in line:
                fout.write(line)
        fin.close()
        fout.close()


try:
    grepinto("line2", "test.txt", "TestOut.txt")
except Exception:
    print("Things gone wrong")
