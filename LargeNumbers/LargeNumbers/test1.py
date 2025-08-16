# if c1=2, we know A=(c1*a1+m11) B=(c1*b1+m12) and neither a1 or m1 can be divided by c1
# if c2=2*3, we know A=(c2*a2+m21) B=(c2*b2+m22) and neither a1 or m1 can be divided by 2 or 3
#       this means that m1=[1,5] and m2=[1,5] becase m can't be a multiple of 2 or 3
# if we need a larger c1, we can multiply N by a prime, and that way we know that m1 will be part of a list of primes or a multiple of the extra prime
# ex : c1=2*3*5   and  we multiple N by next prime to (5*5) => 7*7 so A=7*A=(c1*a+m1) B=7*B=(c1*b+m2)
#       but this way we can't test for new A,B modulo, we can only test modulo for 2 and 5
#       ex : c1=3*5*7 and we can multiple N by 8>7 ... so A=8*A and B=8*B ..N=N*64
# we want to test here is by multiplying by X would lead to all possible m1,m2 combinations or some can be eliminated
#
# why do we want to do this ? Because the bigger we get C1, the larger m1,m2 becomes. Is this good for us ? what we really care is the least amount of m1*m2 combinations
# but the larger C1 is, the more m1*m2 combinations appear. so, do we want to go higher ? only if the amount of combinations is less than the bitcount we extract with c1
#   ex  c1=2*3 m in [1,5]   -> generates 3 combinations : 1*1, 1*5, 5*5 ( if we do not count mirrored combinations )
#       c1=2*3*5 m in [1,7,11,13,17,19,23,29] -> 8.. 27 combinations = 9 times more than previous, but c1 is only 5 times bigger
import math

def is_prime(n):
    if n < 2:
        return False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            return False
    return True

def is_power_of_2(n):
    if n <= 0:
        return False
    return (n & (n - 1)) == 0    
    
def primes_between(a, b, include_pow_2 = False):
    start = min(a, b) + 1
    end = max(a, b)
    primes = []
    for num in range(start, end):
        if is_prime(num) or (include_pow_2 == True and is_power_of_2(num)):
            primes.append(num)
    return primes
    
def get_next_prime(num):
    num += 1
    while( is_prime(num) == False ):
        num += 1
    return num
  
def could_be_multiple_of(num, of, mask):
    for i in range(0,mask):
        mul = i * num
        if (mul % mask) == (of % mask):
            return True
    return False
    
def filter_possible_m(A,B,N,SQN,m,c1,multiplierAB,possible_m,num_mask = 10):
    expected_A = A * multiplierAB
    expected_B = B * multiplierAB
    expected_N = expected_A * expected_B
    expected_m1m2_mod = ((expected_N % multiplierAB)% num_mask)
    ret1 = {}
    ret2 = []    
    for m1 in possible_m:
        for m2 in possible_m:
            '''        
            m1m2 = m1 * m2
            m1m2_is_possible = False
            for unk_add in range (0, num_mask):
                if ((m1m2+unk_add*multiplierAB) % num_mask) == expected_m1m2_mod:
                    m1m2_is_possible = True
                    break
            if m1m2_is_possible == False:
                print(f"wow, we actually managed to filter {m1},{m2}")
                continue
            '''                
            for a in range(0,num_mask):
                for b in range(0,num_mask):
                    possible_A = c1 * a + m1
                    possible_B = c1 * b + m2
                    possible_N = possible_A * possible_B
                    should_be_considered_possible_solution = (((possible_A % num_mask) == (expected_A % num_mask)) and ((possible_B % num_mask) == (expected_B % num_mask)))
#                    if (should_be_considered_possible_solution):
#                        print(f"Should find this as a solution m1={m1},m2={m2},possible_A={possible_A},expected_A={expected_A},possible_B={possible_B},expected_B={expected_B},possible_N={possible_N},expected_N={expected_N}")
                    if (possible_N % num_mask) != (expected_N % num_mask):
                        if (should_be_considered_possible_solution):
                            print("1 Was supposed to find this solution")
#                        print("1 skip")
                        continue
                    # we also expect A and B to be a multiple of multiplierAB
                    if could_be_multiple_of(possible_A, multiplierAB, num_mask) == False:
                        if (should_be_considered_possible_solution):
                            print("2 Was supposed to find this solution")
                        print("2 skip")
                        continue
                    if could_be_multiple_of(possible_B, multiplierAB, num_mask) == False:
                        if (should_be_considered_possible_solution, num_mask):
                            print("3 Was supposed to find this solution")
                        print("3 skip")
                        continue
                    ret1[m1*m2] = f"({m1},{m2})"
                    if m1 not in ret2:
                        ret2.append(m1)
                    if m2 not in ret2:
                        ret2.append(m2)
                        
    return ret1, ret2

def get_max_combo_count(possible_m):
    tot_len = len(possible_m)
    ret = int(tot_len * (tot_len+1) / 2)
    return ret
    
A = 349
B = 751
N = A * B
SQN_float=math.sqrt(N)
SQN = int(SQN_float)
m = N - SQN * SQN
print(f"A={A}, B={B}, N={N}, SQN={SQN}, m={m}")

# let's test a case where we do not scale our input at all
extractAB = 2*3*5
multiplierAB=1
possible_m = [1] + primes_between(5,extractAB)

print(f"A={A*multiplierAB}, B={B*multiplierAB}, N={N*multiplierAB*multiplierAB}, extractAB={extractAB}, multiplierAB={multiplierAB}")
print(f"for extractAB={extractAB} A={extractAB}*{int(A*multiplierAB/extractAB)}+{(A*multiplierAB)%extractAB} B={extractAB}*{int(B*multiplierAB/extractAB)}+{(B*multiplierAB)%extractAB}")
print(f"We presume we tested all primes until {extractAB} and they were not A")
print("possible values for m :")
for pm in possible_m:
    print(f"{pm},",end="")
print("")

ret1, ret2 = filter_possible_m(A,B,N,SQN,m,extractAB,multiplierAB,possible_m)
print(f"possible value combos({len(ret1)}/{get_max_combo_count(possible_m)}) for m :")
for key in ret1:
    print(f"{ret1[key]},",end="")
print("")

ret2.sort()
print("possible values for m after filter :")
for pm in ret2:
    print(f"{pm},",end="")
print("")
print("======================================================================")

# this number can be as large as we tried all the primes until it
# ex if we pick 2*3*5=30, we should test if N can be divided by [2,3,5,7,1,13....29] -> ensures A is larger than 30
# we added 7 as an additional prime to the extracted number and see how much worse our situation got
extractAB = 2*3*5*7
multiplierAB=get_next_prime(7)
possible_m = [1,11*11,11*13,11*17,11*19,13*13] + primes_between(7,extractAB)

print(f"A={A*multiplierAB}, B={B*multiplierAB}, N={N*multiplierAB*multiplierAB}, extractAB={extractAB}, multiplierAB={multiplierAB}")
print(f"for extractAB={extractAB} A={extractAB}*{int(A*multiplierAB/extractAB)}+{(A*multiplierAB)%extractAB} B={extractAB}*{int(B*multiplierAB/extractAB)}+{(B*multiplierAB)%extractAB}")
print("possible values for m :")
for pm in possible_m:
    print(f"{pm},",end="")
print("")

ret1, ret2 = filter_possible_m(A,B,N,SQN,m,extractAB,multiplierAB,possible_m,100)
print(f"possible value combos({len(ret1)}/{get_max_combo_count(possible_m)}) for m :")
for key in ret1:
    print(f"{ret1[key]},",end="")
print("")

ret2.sort()
print("possible values for m after filter :")
for pm in ret2:
    print(f"{pm},",end="")
print("")

print("My conclusion: we scaled up N by 11, but the number of combinations we need to track increased by 295/10=29 times. Not good. Everything simply got larger a,b,m1,m2. Does not help")
print("Bonus fact that m2=121=11*11,11*13,11*17,11*19,13*13 -> m2 became a multiple of another number. No longer a simple prime")
