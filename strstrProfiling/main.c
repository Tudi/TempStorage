#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"
#include "strstrReference.h"
#include "strstrSearchedLen.h"
#include "strstrBothLen.h"
#include "strstrMischasan.h"
#include "strstrMischasanEdited.h"
#include "strstrBothLen_v2.h"
#include "strstrNOPString.h"
#include "strstrAVX2.h"
#include "strstrAVX2_V2.h"
#include "strstrSumCmp.h"
#include "strstr5bit.h"

int main()
{
	printf("Starting init ...");
	RunDebug5BitTests();
	// generate in memory string to be searched in
	GenerateInputStrings(MEMORY_ALLOC_FOR_INPUT, MIN_INPUT_LEN, MAX_INPUT_LEN, USE_STRING_PADDING);
	printf(" ... ");
	// just a curiosity test
	GenerateInputNOPStrings(MEMORY_ALLOC_FOR_INPUT, MIN_INPUT_LEN, MAX_INPUT_LEN, USE_STRING_PADDING);
	printf(" ... ");
	// generate strings we will use to run on the input
	GenerateSearchedStrings(MEMORY_ALLOC_FOR_SEARCH, MIN_SEARCH_LEN, MAX_SEARCH_LEN, USE_STRING_PADDING);
	printf(" ... Done \n");

	// initialize the timer module
	InitTimer();

	// run a specific test
	Run_strstr_reference_test();
//	Run_strstr_know_searched_length_test();
//	Run_strstr_know_both_lengths_test();
//	Run_strstr_Mischasan_test();
	Run_strstr_Mischasan_Extended_test(); // this is better on linux but not on windows ?
	Run_strstr_Both_Len_V2_test();	// about 15% better than strstr
	Run_strstr_NOPString_test();
	Run_strstr_AVX2_test();			// about 15% better than strstr
//	Run_strstr_AVX2_V2_test();
//	Run_strstr_Sum_Word_test();		// this is only for large strings where the value does not fit into single registry
	Run_strstr_5Bit();

	printf("\nAll tests finished\n");
	// not using this
	return 0;
}