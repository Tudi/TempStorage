#ifndef _PROFILE_PERSISTENT_2_H_
#define _PROFILE_PERSISTENT_2_H_

#include "serialize_generic.h"

// always update this in case you add new fields
#define PROFILE_PERSISTENT_WRITER_VERSION 1

// Feel free to add/remove/reorder enums here. 
// The important part is PPFN_MAX_FIELD_NAME to be large enough to store all fields 
typedef enum ProfilePersistentFieldNames
{
	PPFN_ID = 0,
	PPFN_FIRST_NAME,
	PPFN_LAST_NAME,
	PPFN_ARCHIVED_PRIMARY_EMAIL,
	PPFN_LINKEDIN_USER,
	PPFN_PROFILE_CODE,
	PPFN_PROFILE_KEY,
	PPFN_HEADLINE,
	PPFN_SUMMARY,
	PPFN_IMAGE_URL,
	PPFN_INDUSTRY,
	PPFN_LINKEDINURL,
	PPFN_NUM_CONNECTIONS,
	PPFN_SOURCE,
	PPFN_BATCH,
	PPFN_COVID19,
	PPFN_LINKEDIN_CANONICAL_URL,
	PPFN_LINKEDIN_CANONICAL_USER,
	PPFN_LINKEDIN_ID,
	PPFN_SOURCE_KEY,
	PPFN_LOCALITYID,
	PPFN_LOCATION,
	PPFN_COUNTRY,
	PPFN_LOGICAL_DELETE,
	PPFN_GENDER,
	PPFN_ETHNICITY,
	PPFN_LAST_CACHED_AT,
	PPFN_LAST_MESSAGED,
	PPFN_LAST_REPLIED,
	PPFN_LAST_POSITIVE_REPLIED,
	PPFN_GROUPS,
	PPFN_PROJECTS,
	PPFN_ATS_COMPANIES,
	PPFN_ATS_STATUS,
	PPFN_ATS_LAST_ACTIVITY,
	PPFN_ACTIONED,
	PPFN_POSITIONS,
	PPFN_EDUCATION,
	PPFN_SKILLS,
	PPFN_PROFILE_EMAILS,
	PPFN_PROFILE_SOCIAL_URLS,
	PPFN_PROFILE_PHONE_NUMBERS,
	PPFN_PROFILE_TAGS,
	PPFN_FULL_NAME_STANDARDIZED,
	PPFN_HEADLINE_STANDARDIZED,
	PPFN_SUMMARY_STANDARDIZED,
	PPFN_MAX_FIELD_NAME
}ProfilePersistentFieldNames;

typedef enum CompanyPersistentFieldNames
{
	CPFN_COMPANY_NAME,
	CPFN_COMPANY_DOMAIN,
	CPFN_COMPANY_ID,
	CPFN_COMPANY_PARENT_ID,
	CPFN_COMPANY_INDUSTRIES,
	CPFN_COMPANY_NUM_EMPLOYEES,
	CPFN_COMPANY_STAGE,
	CPFN_COMPANY_HEADQUARTERS_CITY,
	CPFN_COMPANY_HEADQUARTERS_STATE,
	CPFN_COMPANY_HEADQUARTERS_ZIPCODE,
	CPFN_COMPANY_URL,
	CPFN_COMPANY_DESCRIPTION,
	CPFN_COMPANY_CRUNCHBASEURL,
	CPFN_COMPANY_CRUNCHBASEHEADQUARTERS,
	CPFN_COMPANY_HEADQUARTERS_COUNTRY,
	CPFN_COMPANY_FACEBOOKURL,
	CPFN_COMPANY_TWITTERURL,
	CPFN_COMPANY_LINKEDINURL,
	CPFN_COMPANY_LINKEDIN_USER,
	CPFN_COMPANY_LAST_CACHED_AT,
	CPFN_COMPANY_LOGO_URL,
	CPFN_COMPANY_INDUSTRIES_STRUCT,
	CPFN_MAX_FIELD_NAME
}CompanyPersistentFieldNames;

// Never change the !value! of a field. Feel free to create new fields at the end
// Every time you change a field in a structure that is persisted in DAO, !create a new ID! for it
typedef enum DAO_FIELD_IDs
{	
	DFID_PROFILE_ID = 1,
	DFID_PROFILE_FIRST_NAME = 2,
	DFID_PROFILE_LAST_NAME = 3,
	DFID_PROFILE_ARCHIVED_PRIMARY_EMAIL = 4,
	DFID_PROFILE_LINKEDIN_USER = 5,
	DFID_PROFILE_PROFILE_CODE = 6,
	DFID_PROFILE_PROFILE_KEY = 7,
	DFID_PROFILE_HEADLINE = 8,
	DFID_PROFILE_SUMMARY = 9,
	DFID_PROFILE_IMAGE_URL = 10,
	DFID_PROFILE_INDUSTRY = 11,
	DFID_PROFILE_LINKEDINURL = 12,
	DFID_PROFILE_NUM_CONNECTIONS = 13,
	DFID_PROFILE_SOURCE = 14,
	DFID_PROFILE_BATCH = 15,
	DFID_PROFILE_COVID19 = 16,
	DFID_PROFILE_LINKEDIN_CANONICAL_URL = 17,
	DFID_PROFILE_LINKEDIN_CANONICAL_USER = 18,
	DFID_PROFILE_LINKEDIN_ID = 19,
	DFID_PROFILE_SOURCE_KEY = 20,
	DFID_PROFILE_LOCALITYID = 21,
	DFID_PROFILE_LOCATION = 22,
	DFID_PROFILE_COUNTRY = 23,
	DFID_PROFILE_LOGICAL_DELETE = 24,
	DFID_PROFILE_GENDER = 25,
	DFID_PROFILE_ETHNICITY = 26,
	DFID_PROFILE_LAST_CACHED_AT = 27,
	DFID_PROFILE_LAST_MESSAGED = 28,
	DFID_PROFILE_LAST_REPLIED = 29,
	DFID_PROFILE_LAST_POSITIVE_REPLIED = 29,
	DFID_PROFILE_GROUPS = 30,
	DFID_PROFILE_PROJECTS = 31,
	DFID_PROFILE_ATS_COMPANIES = 32,
	DFID_PROFILE_ATS_STATUS = 33,
	DFID_PROFILE_ATS_LAST_ACTIVITY = 34,
	DFID_PROFILE_ACTIONED = 35,
	DFID_PROFILE_POSITIONS = 36,
	DFID_PROFILE_EDUCATION = 37,
	DFID_PROFILE_SKILLS = 38,
	DFID_PROFILE_PROFILE_EMAILS = 38,
	DFID_PROFILE_PROFILE_SOCIAL_URLS = 39,
	DFID_PROFILE_PROFILE_PHONE_NUMBERS = 40,
	DFID_PROFILE_PROFILE_TAGS = 41,
	DFID_PROFILE_FULL_NAME_STANDARDIZED = 42,
	DFID_PROFILE_HEADLINE_STANDARDIZED = 43,
	DFID_PROFILE_SUMMARY_STANDARDIZED = 44,

	DFID_COMPANY_NAME = 256,
	DFID_COMPANY_DOMAIN = 257,
	DFID_COMPANY_ID = 258,
	DFID_COMPANY_PARENT_ID = 259,
	DFID_COMPANY_INDUSTRIES = 260,
	DFID_COMPANY_NUM_EMPLOYEES = 261,
	DFID_COMPANY_STAGE = 262,
	DFID_COMPANY_HEADQUARTERS_CITY = 263,
	DFID_COMPANY_HEADQUARTERS_STATE = 264,
	DFID_COMPANY_HEADQUARTERS_ZIPCODE = 265,
	DFID_COMPANY_URL = 266,
	DFID_COMPANY_DESCRIPTION = 267,
	DFID_COMPANY_CRUNCHBASEURL = 268,
	DFID_COMPANY_CRUNCHBASEHEADQUARTERS = 269,
	DFID_COMPANY_HEADQUARTERS_COUNTRY = 270,
	DFID_COMPANY_FACEBOOKURL = 271,
	DFID_COMPANY_TWITTERURL = 272,
	DFID_COMPANY_LINKEDINURL = 273,
	DFID_COMPANY_LINKEDIN_USER = 274,
	DFID_COMPANY_LAST_CACHED_AT = 275,
	DFID_COMPANY_LOGO_URL = 276,
	DFID_COMPANY_INDUSTRIES_STRUCT = 277,
}DAO_FIELD_IDs;
#endif