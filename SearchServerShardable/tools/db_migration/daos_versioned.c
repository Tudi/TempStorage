#include <daos.h>
#include <logger.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <profile_definitions.h>
#include <sys/stat.h>

int V7_daos_loadAllItemsFromFile(ItemFunctions_t *itemFunctions, const char *filename, void ***items, DaosCount_t *numItems, DaosFileVersion_t expectedVersion);

static int deleteOldFile(const char *inFileName)
{
    if (unlink(inFileName) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: remove(%s) failed. errno = %d (\"%s\").", inFileName, errno, strerror(errno));
        return 1;
    }
    return 0;
}

static void saveAllItems(Daos_t daos, ItemFunctions_t *itemFunctions, const char *inFileName, void **items, DaosCount_t numItems)
{
    if (items == NULL)
    {
        return;
    }
    for (size_t i = 0; i < numItems; i++)
    {
        if (items[i] == NULL)
        {
            continue;
        }
        bool isNew = false;
        int saveErr = daos_saveItem(daos, items[i], &isNew);
        if (saveErr != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to save upgraded item %zd from file %s.", i, inFileName);
        }
        itemFunctions->freePersistentItem(items[i]);
        items[i] = NULL;
    }
}

int upgradeFileVersion(Daos_t daos, ItemFunctions_t *itemFunctions, const char *inFileName, unsigned int fileVersion)
{
    void **items = NULL;
    DaosCount_t numItems = 0;
    int ret = V7_daos_loadAllItemsFromFile(itemFunctions, inFileName, &items, &numItems, fileVersion);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to load all items from version %d file %s.", fileVersion, inFileName);
        free(items);
        return ret;
    }
    ret = deleteOldFile(inFileName);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: deleteOldFile() returned %d", ret);
        free(items);
        return ret; 
    }
    saveAllItems(daos, itemFunctions, inFileName, items, numItems);
    free(items); // individual items got freed on save

    return 0;
}
