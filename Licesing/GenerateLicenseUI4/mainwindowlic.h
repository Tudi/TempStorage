#ifndef MAINWINDOWLIC_H
#define MAINWINDOWLIC_H

class LicenseAPI
{
public:
    static int GetSeedContentFromFile(const char *Path, char **Seed, int *Len);
    static int GetSeedInfo(char *Seed, int SeedLen, char **HostName, char **Role);
    static void FreeLicDup(void *p);
};

#endif // MAINWINDOWLIC_H
