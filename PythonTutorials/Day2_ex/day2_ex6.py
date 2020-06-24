companies = {1: "company X", 2: "Y company", 3: "Z Inc."}
persons = {"Maria": 1, "Vali": 1, "Ana": 2, "Mihai": 2, "Adi": 3}

CompanyEmployees2 = {}
for EmployeeName, EmployeeCompanyId in persons.items():
    if companies[EmployeeCompanyId] not in CompanyEmployees2:
        CompanyEmployees2[companies[EmployeeCompanyId]] = list()
    CompanyEmployees2[companies[EmployeeCompanyId]].append(EmployeeName)
print(CompanyEmployees2)
