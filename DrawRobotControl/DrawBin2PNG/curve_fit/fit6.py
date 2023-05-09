import numpy, scipy, scipy.optimize
import sys

def func(x, x0, x1):
    return x1 * x + x0
    
if __name__ == "__main__":   
    startLoc = int(sys.argv[1])
    endLoc = int(sys.argv[2])
    given_in = [int(sys.argv[3]),int(sys.argv[4]),int(sys.argv[5]),int(sys.argv[6]),int(sys.argv[7])]
    given_out = [float(sys.argv[8]),float(sys.argv[9]),float(sys.argv[10]),float(sys.argv[11]),float(sys.argv[12])]
#    print(given_in)
#    print(given_out)
    fp, pcov = scipy.optimize.curve_fit(func, given_in, given_out, bounds=[-4000,4000], maxfev=5000)
    
    toprint = ""
    range_step = 1
    if endLoc < startLoc :
        range_step = -1
    for i in range(startLoc,endLoc,range_step):
#        toprint = toprint + str(i) + '=' + str(func(i,fp[0],fp[1])) + ' '
        toprint = toprint + str(func(i,fp[0],fp[1])) + ' '
        
#   not required, just for debugging
    for i in given_in:
#        toprint = toprint + str(i) + '=' + str(func(i,fp[0],fp[1])) + ' '
        toprint = toprint + str(func(i,fp[0],fp[1])) + ' '
        
    print(toprint)
        
