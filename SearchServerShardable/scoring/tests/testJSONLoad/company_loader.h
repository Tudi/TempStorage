#ifndef _COMPANY_LOADER_H_
#define _COMPANY_LOADER_H_

#define MAX_COMPANY_ID_ALLOWED 10000000

void LoadCompaniesFromFiles();
void freeLoadedCompanies();

extern struct CompanyCached** cachedCompanies;

#endif