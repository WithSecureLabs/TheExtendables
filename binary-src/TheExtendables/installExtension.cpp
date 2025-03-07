#include "extendables.h"


/*
Installs an extension that is 'managed by the organisation' in a chromium based browser. Requires administrative privileges to do.

REQUIRED:
extensionId - identifier string for the extension, eg. aeblfdkhhhdcdjpifhhbdiojplfjncoa

OPTIONAL:
browser - browser to target: chrome or edge (all must be lowercase), default - chrome
allUsers - install in HKLM or HKCU: user = HKCU, machine = HKLM (only works if admin), default - machine
*/
void extensionInstall(char* extensionId, char* browser, char* allUsers, int verbose) {
    //Set up variables and default values
    HKEY localOrCurrent = HKEY_CURRENT_USER;
    HKEY default_key;
    DWORD numValues = 0;
    char* regPath = (char*)"";
    char* valueName = (char*)"";
    BOOL freeer = FALSE;


    //Set the correct registry location depending on browser specified
    int extLen = strlen(extensionId);
    char* valueContent = (char*)calloc(1, extLen + 57);
    if (strcmp(browser, "chrome") == 0) {
        memcpy_s(valueContent, extLen + 49, extensionId, extLen + 49);
        strcat_s(valueContent, extLen + 49, ";https://clients2.google.com/service/update2/crx");
        regPath = (char*)"Software\\Policies\\Google\\Chrome\\ExtensionInstallForcelist";
    }
    else if (strcmp(browser, "edge") == 0) {
        memcpy_s(valueContent, extLen + 57, extensionId, extLen + 57);
        strcat_s(valueContent, extLen + 57, ";https://edge.microsoft.com/extensionwebstorebase/v1/crx");
        regPath = (char*)"Software\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist";
    }
    else {
        printf("\n\n\n[-] Browser not recognised! Use - Chrome or Edge\n");
        return;
    }

    //Set whether to use the NativeMessaging Application for all users or just the current user
    if (strcmp(allUsers, "user") == 0) {
        localOrCurrent = HKEY_CURRENT_USER;
        printf("\n\n\n[*] Installing for current user\n");
    }
    else if (strcmp(allUsers, "machine") == 0) {
        localOrCurrent = HKEY_LOCAL_MACHINE;
        printf("\n\n\n[*] Installing for all users\n");
    }
    else {
        printf("\n\n\n[-] Please pick either user for HKCU or machine for HKLM (Default is local)\n");
        return;
    }

    //Open up key for install
    NTSTATUS status = RegOpenKeyA(localOrCurrent, regPath, &default_key);
    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error opening key - %s for %s, attempting to create the key instead\n", regPath, allUsers);
        //If errored, it may be because key does not exist, so create the target registry key
        NTSTATUS status = RegCreateKeyExA(localOrCurrent, regPath, 0, NULL, 0, KEY_SET_VALUE, NULL, &default_key, NULL);
        if (status != ERROR_SUCCESS)
        {
            printf("[-] Error creating key!\n");
            if (strcmp(allUsers, "machine") == 0) {
                printf("[-] Are you running in a high-integrity process?\n");
            }
            return;
        }
        printf("[*] Created key!\n", browser);
        valueName = (char*)"1";
    }
    else {
        freeer = TRUE;
        printf("[*] Opened target key!\n", browser);
        //Query info and save the number of subkeys in a variable
        status = RegQueryInfoKeyA(default_key, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, NULL, NULL, NULL, NULL);
        if (status != ERROR_SUCCESS)
        {
            printf("[-] Error getting info from key - %s for %s!\n", regPath, allUsers);
            return;
        }
        //Allocate buffer and then convert the current number of subkeys + 1 to a string (for the value name as it iterates)
        valueName = (char*)malloc(5);
        _itoa_s(numValues + 1, valueName, 5, 10);

        printf("[*] Reading info about target key!\n", browser);
    }

    //Set the value of the registry key to point at the native.json path
    status = RegSetValueExA(default_key, valueName, 0, REG_SZ, (const BYTE*)valueContent, strlen(valueContent) + 1);
    free((PVOID)valueContent);

    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error setting value!\n");
        return;
    }
    //Close the key
    RegCloseKey(default_key);

    printf("[*] Registry value %s created in key %s for %s\n", valueName, regPath, allUsers);

    //If malloc is used, free buffer
    if (freeer == TRUE) {
        free((PVOID)valueName);
    }

    printf("[*] Extension installed for %s successfully!\n", browser);



    return;
}
