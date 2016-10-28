#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

string solution_Laura(vector<int> &A)
{
	
	int sum = 0;
	int N = A.size();
	
	int nrOdds = 0;
	int firstOdd = -1;
	int lastOdd = -1;
	int prevLastOdd = -1;

	for (int i = 0; i < N;  i++)
	{
		sum += A[i];
		if (A[i] % 2 == 1)
		{
		    nrOdds++;
		    if (firstOdd < 0 ) firstOdd = i;
			prevLastOdd = lastOdd;
		    lastOdd = i;
		}

	}

	//first case, only one number
	if (N == 1)
	{
		if (A[0] % 2 == 0) return "0,0";
		else return "NO SOLUTION";
	}


	//return string
	ostringstream os;

	//if sum is even, remove the entire sequence
	if (sum % 2 == 0) os << "0," << (N-1);

	//if all odds and sum is odd, remove all except last number
	else if (nrOdds == N && sum % 2 == 1) os << "0," << N-2;
	
	else
	{
		/* The idea is to give the other player a no solution sequence,
		***which is a sequence with odd number in the middle and equal 
		***number of evens to the left and to the right.
		***Sum is odd so we have at least one odd number
		*/

		//already have the no solution sequence
		if ((firstOdd == lastOdd) && (firstOdd == (N / 2) && N % 2 == 1))
			os << "NO SOLUTION";
		//only one odd number
		else if (firstOdd == lastOdd)
		{
			//remove odd numbers from one side
			if (firstOdd < N/2)
			{
				//remove elements to the right
				os << firstOdd + 1 << "," << firstOdd + N - (2 * firstOdd + 1);
			}
			else
			{
				//remove elements to the left
				os <<  "0," <<   N - 2 * (N - firstOdd);
			}
		}
		else{
			//3 odd numbers or more

			//we keep only one odd number, the first or the last, and remove the rest
			//the number of even numbers to the right is N - 1 - lastOdd
			//the number of even numbers to the left is firstOdd
			//remove from the beginning if possible
			if ((prevLastOdd - firstOdd + 1) > N - ( 2 * (N - 1 - lastOdd) + 1))
			{
				os << firstOdd + 1 << "," << firstOdd + N - (2 * firstOdd + 1);
			}
			else
			{
				if ((N - 1 - lastOdd) + prevLastOdd < lastOdd - 1)
					os << "0," << N - 1 - 2 * (N - 1 - lastOdd) - 1;
				else
					os << (N - 1 - lastOdd) + prevLastOdd - lastOdd + 1 << "," << prevLastOdd;
			}
		}
		
	}
	return os.str();	
}
