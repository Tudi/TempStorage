#include <company_loader.h>
#include <company.h>
#include <company_cached.h>
#include <json_utils.h>
#include <json_specialized_array.h>
#include <json_tokener.h>
#include <linkhash.h>
#include <stdarg.h>
#include <ctype.h>
#include <dirent.h> 
#include <stdio.h>
#include <string.h>

struct CompanyCached** cachedCompanies = NULL;
int companiesLoadedCount = 0;
struct CompanyCached** cachedCompaniesConsecutive = NULL;

bool getFileContents_(const char* dir, const char* fileName, uint8_t** contents, size_t* size);

void LoadCompanyFromDirectory(const char* dir, const char* filename)
{
    uint8_t* fileContent;
    size_t contentByteCount;
    bool getres = getFileContents_(dir, filename, &fileContent, &contentByteCount);
    if (getres == false)
    {
        printf("Could not get file content : %s/%s\n", dir, filename);
        return;
    }
    struct Company persCompany;
    initCompany(&persCompany);

    struct json_tokener* jsonTokener = json_tokener_new();
    struct json_object* jsonObj = json_tokener_parse_ex(jsonTokener, (const char*)fileContent, (int)contentByteCount);
    if (jsonObj == NULL)
    {
        printf("JSON buffer (size = %zu) = %s\n\n", contentByteCount, fileContent);
    }
    else
    {
        if (unmarshallCompany(&persCompany, jsonObj))
        {
            if (persCompany.id < MAX_COMPANY_ID_ALLOWED)
            {
                struct CompanyCached* cachedCompany = malloc(sizeof(struct CompanyCached));
                if (cachedCompany == NULL)
                {
                    printf("Failed to allocate memory for companyCached\n");
                    return; // at this point we are doomed
                }

                initCompanyCached(cachedCompany);
                if (companyToCompanyCached(cachedCompany, &persCompany))
                {
                    cachedCompanies[persCompany.id] = cachedCompany;
                    cachedCompaniesConsecutive[companiesLoadedCount++] = cachedCompany;
                }
                else
                {
                    free(cachedCompany);
                    printf("Error converting persistent profile to cached profile for %s\%s\n", dir, filename);
                }
            }
            else
            {
                printf("Error, company ID %d is larger than allowed %d\n", persCompany.id, MAX_COMPANY_ID_ALLOWED);
            }
        }
        else
        {
            printf("Could not convert %s/%s to profile\n", dir, filename);
        }
        json_object_put(jsonObj);
    }

    json_tokener_free(jsonTokener);

    freeCompany(&persCompany);
    free(fileContent);
}

void LoadCompaniesFromDirectory(const char* dirName)
{
    struct dirent** namelist;
    int n;

    n = scandir(dirName, &namelist, NULL, alphasort);
    if (n < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Failed to open directory %s for input files", dirName);
    }
    else
    {
        while (n--)
        {
            if (namelist[n]->d_name[0] != '.')
            {
                LoadCompanyFromDirectory(dirName, namelist[n]->d_name);
            }
            free(namelist[n]);
        }
        free(namelist);
    }
}

void freeLoadedCompanies()
{
    for (int i = 0; i < companiesLoadedCount; i++)
    {
        freeCompanyCached(cachedCompaniesConsecutive[i]);
        free(cachedCompaniesConsecutive[i]);
    }
    free(cachedCompanies);
    free(cachedCompaniesConsecutive);
    companiesLoadedCount = 0;
    cachedCompanies = NULL;
}

void LoadCompaniesFromFiles()
{
    if (cachedCompanies == NULL)
    {
        cachedCompanies = (struct CompanyCached**)malloc(sizeof(struct CompanyCached*) * MAX_COMPANY_ID_ALLOWED);
        cachedCompaniesConsecutive = (struct CompanyCached**)malloc(sizeof(struct CompanyCached*) * MAX_COMPANY_ID_ALLOWED);
    }
    memset(cachedCompanies, 0, sizeof(struct CompanyCached*) * MAX_COMPANY_ID_ALLOWED);
    memset(cachedCompaniesConsecutive, 0, sizeof(struct CompanyCached*) * MAX_COMPANY_ID_ALLOWED);

    LoadCompaniesFromDirectory("../JSON_Company");
    if (companiesLoadedCount == 0)
    {
        LoadCompaniesFromDirectory("./JSON_Company");
    }
}
