#include "extendables.h"


//Function to parse the contents of a nativemessaging manifest file
int nmManifestParser(char* path) {

    //Setup vars
    HANDLE fileHandle;
    char* outBuf = 0;
    OFSTRUCT openBuf = { NULL };
    BOOL readSuccess = FALSE;
    long fileSize = 0;
    OVERLAPPED ol = { 0 };
    DWORD a = 0;
    char* occuranceAddr = 0;
    char* occuranceEndAddr = 0;
    char* reversed = 0;
    char* unreversed = 0;
    char* extensionSubStr = 0;
  

    //Set current directory incase of relative paths
    char* dupePath = (char*)calloc(1, strlen(path)+1);
    memcpy_s(dupePath, strlen(path) + 1, path, strlen(path));
    reversed = _strrev(dupePath);
    extensionSubStr = strstr(reversed, "\\");
    unreversed = _strrev(extensionSubStr);
    
    BOOL success = SetCurrentDirectoryA(unreversed);

    if (success == FALSE) {
        printf("[-] Failed to set current directory to NativeMessaging manifest. May cause errors if relative paths are used.\n");
    }

    //Get a handle to the target manifest and get its size
    fileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    fileSize = GetFileSize(fileHandle, NULL);

    //If retrieving the handle fails, return error
    if ((HFILE)fileHandle == HFILE_ERROR) {
        printf("[-] NativeMessaging file does not seem to exist! - %s\n\n", path);
        return 2;
    }

    //If retrieving the handle fails, return error
    if (fileSize > 1048576) {
        printf("[-] File is too big!\n\n");
        return 2;
    }

    //Allocate a buffer for the file contents and set all bytes to 0
    outBuf = (char*)calloc(1, fileSize);

    //Read the file contents into the buffer
    ReadFile(fileHandle, outBuf, fileSize - 1, &a, &ol);
    CloseHandle(fileHandle);

    //Extract the "path" string from the manifest and check if it is writable
    occuranceAddr = strstr(outBuf, "ath\":");
    if (occuranceAddr == NULL) {
        occuranceAddr = strstr(outBuf, "ath\" ");
    }
    if (occuranceAddr != NULL) {
        //Moving the offset forward to skip the characters we know
        occuranceAddr += 7;
        occuranceEndAddr = strstr(occuranceAddr, "\"");       

        char* nmHostPath = (char*)calloc(1, 255);

        memcpy_s(nmHostPath, 255, occuranceAddr, (occuranceEndAddr - occuranceAddr));

        HANDLE nativeManifestFile = CreateFileA(nmHostPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
        if (nativeManifestFile != INVALID_HANDLE_VALUE) {
            printf("NativeMessaging host application: %s\n[*] NativeMessaging host application writable and hijackable!\n\n", nmHostPath);
            CloseHandle(nativeManifestFile);
        }
        else {
            printf("NativeMessaging host application: %s\n[-] NativeMessaging host application not hijackable!\n\n", nmHostPath);
        }
        //Free buffer
        free(nmHostPath);
    }

    else {
        printf("[-] Unable to identify 'path' in NativeMessaging manifest!\n");
    }
    return 0;
}



//Function to parse the contents of an extension manifest file
int manifestParser(char* path) {
    //Setup vars
    HANDLE fileHandle;
    char* outBuf = 0;
    OFSTRUCT openBuf = { NULL };
    BOOL readSuccess = FALSE;
    long fileSize = 0;
    OVERLAPPED ol = { 0 };
    DWORD a = 0;
    PVOID occuranceAddr = 0;

    //Get a handle to the target manifest and get its size
    fileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    fileSize = GetFileSize(fileHandle, NULL);

    //If retrieving the handle fails, return error
    if ((HFILE)fileHandle == HFILE_ERROR) {
        printf("[-] File does not seem to exist! - %s\n\n", path);
        return 2;
    }

    //If retrieving the handle fails, return error
    if (fileSize > 1048576) {
        printf("[-] File is too big!\n\n");
        return 2;
    }

    //Allocate a buffer for the file contents and set all bytes to 0
    outBuf = (char*)calloc(1, fileSize);

    //Read the file contents into the buffer
    ReadFile(fileHandle, outBuf, fileSize - 1, &a, &ol);
    CloseHandle(fileHandle);

    //Check if the "nativeMessaging" string is within the file
    occuranceAddr = strstr(outBuf, "\"nativeMessaging\"");

    //If the "nativeMessaging" string is in the file, it may be vulnerable. Return success
    if (occuranceAddr != 0) {
        printf("[*] Potentially vulnerable extension!\nIf NativeMessaging manifest and/or NativeMessaging host registry key does not exist, you can create them using the '/persist' feature of this tool!\nYou will need to retrieve the extension name from the legitimate NativeMessaging manifest (you may need to install the NativeMessaging components legitimately on a test machine to get this)\nOR\nLook through the extension's JavaScript and find what is passed to 'runtime.connectNative()' in the format 'com.companyName.extensionName' for the extensionName.\n\n\n");
        free(outBuf);
        return 0;
    }

    //If string not in the buffer, the extension does not have the correct permissions
    printf("[-] Not vulnerable!\n\n\n");
    //Free buffer
    free(outBuf);
    return 1;
}

//Function to enumerate values
int enumValues(char* path, HKEY localOrCurrent, int verbose, int forcelist) {

    //Set up variables
    HKEY default_key;
    HKEY default_key2;
    DWORD numKeys = 0;
    DWORD numVals = 0;
    DWORD numVals1 = 0;
    TCHAR achKey[255];
    DWORD cbName = 0;
    DWORD cchValue = 0;
    DWORD retCode = 0;
    TCHAR achValue[255] = { '\0' };
    char* valueVal = (char*)malloc(16383);
    DWORD lpDataLength;
    int number = 0;

    //Open up key to enumerate
    NTSTATUS status = RegOpenKeyA(localOrCurrent, path, &default_key);
    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error opening key - %s\n[-] Probably because it has not been configured!\n", path);
        return 1;
    }

    //Query info and save the number of subkeys in a variable
    status = RegQueryInfoKeyA(default_key, NULL, NULL, NULL, &numKeys, NULL, NULL, &numVals, NULL, NULL, NULL, NULL);
    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error getting info from key - %s!\n", path);
        return 1;
    }
    if (verbose == 1) {
        printf("[*] Reading info about target key!\n");
    }

    //Loop through each value
    for (int i = 0; i < numVals; i++)
    {
        cchValue = 255;
        achValue[0] = '\0';
        lpDataLength = 16383;
        //Enumerate values
        retCode = RegEnumValueA(default_key, i, (LPSTR)achValue, &cchValue, NULL, NULL, (unsigned char*)valueVal, &lpDataLength);
        if (retCode == ERROR_SUCCESS)
        {
            printf("Value: %s\n", valueVal);
            
            HANDLE nativeManifestFile = CreateFileA(valueVal, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (nativeManifestFile != INVALID_HANDLE_VALUE) {
                if (forcelist == 0) {
                    printf("[+] NativeMessaging manifest is writable and hijackable by the current user!\n");
                }
                else {
                    printf("[+] ExtensionInstallForcelist is writable and hijackable by the current user!\n");
                }
                CloseHandle(nativeManifestFile);

            }
            else {
                if (forcelist == 0) {
                    printf("[-] NativeMessaging manifest is not writable by the current user!\n");
                }
                else {
                    printf("[-] ExtensionInstallForcelist is not writable by the current user!\n");
                }
            }
            
            //Test to see if the nativeMessaging host application is writable if not looking for the ExtensionInstallForceList
            if (forcelist == 0) {
                nmManifestParser(valueVal);
            }

        }
        else {
            printf("Error reading value: %d\n", retCode);
        }
    }
    //Close the key and free the valueVal
    RegCloseKey(default_key);
    free((PVOID)valueVal);
    return 0;
}

//Function to enumerate registry keys and values
int enumKeys(char* path, HKEY localOrCurrent, int verbose) {

    //Set up variables
    HKEY default_key;
    HKEY default_key2;
    HKEY default_key3;
    DWORD numKeys = 0;
    DWORD numVals = 0;
    DWORD numVals1 = 0;
    TCHAR achKey[255];
    DWORD cbName = 0;
    DWORD cchValue = 0;
    DWORD retCode = 0;
    TCHAR achValue[255] = { '\0' };
    DWORD lpDataLength;


    //Open up key to enumerate
    NTSTATUS status = RegOpenKeyA(localOrCurrent, path, &default_key);
    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error opening key - %s\n[-] Probably because it has not been configured!\n", path);
        return 1;
    }

    //Query info and save the number of subkeys in a variable
    status = RegQueryInfoKeyA(default_key, NULL, NULL, NULL, &numKeys, NULL, NULL, &numVals, NULL, NULL, NULL, NULL);
    if (status != ERROR_SUCCESS)
    {
        printf("[-] Error getting info from key - %s!\n", path);
        return 1;
    }
    if (verbose == 1) {
        printf("[*] Reading info about target key!\n");
    }

    //If number of numKeys isn't 0
    if (numKeys != 0)
    {
        //Loop through all sub-keys
        for (int i = 0; i < numKeys; i++)
        {
            //Set up variable
            cbName = 255;

            //Enumerate sub-keys
            retCode = RegEnumKeyExA(default_key, i, (LPSTR)achKey, &cbName, NULL, NULL, NULL, NULL);

            if (retCode == ERROR_SUCCESS)
            {
                printf("\nKey: %s\\%s\n", path,achKey);
            }
            else {
                printf("Error reading key: %d\n", retCode);
            }

            //Open up key to enumerate
            char* pathBuf = (char*)calloc(1, 255);
            memcpy_s(pathBuf, 255, path, 255);
            strcat_s(pathBuf, 255, "\\");
            strcat_s(pathBuf, 255, (char*)achKey);
            NTSTATUS status = RegOpenKeyA(localOrCurrent, pathBuf, &default_key2);
            if (status != ERROR_SUCCESS)
            {
                printf("[-] Error opening key - %s\n[-] Probably because there are no managed extensions!\n", path);
            }
            //Query info and save the number of subkeys in a variable
            status = RegQueryInfoKeyA(default_key2, NULL, NULL, NULL, NULL, NULL, NULL, &numVals1, NULL, NULL, NULL, NULL);
            if (status != ERROR_SUCCESS)
            {
                printf("[-] Error getting info from key - %s!\n", path);
            }
            //If number of value results isn't 0
            if (numVals1 != 0) {
                
                status = RegOpenKeyExA(localOrCurrent, pathBuf, NULL, KEY_WRITE, &default_key3);
                if (status != ERROR_SUCCESS)
                {
                    printf("[-] Key is not writable by the current user!\n");
                }
                else {
                    printf("[+] Key is overwritable and hijackable by the current user!\n");
                    RegCloseKey(default_key3);
                }

                enumValues(pathBuf, localOrCurrent, verbose,0);

            }
            else {
                printf("There were no values in - %s\n", pathBuf);
            }
            //Close the key and free buffer
            RegCloseKey(default_key2);
            free((PVOID)pathBuf);

        }
    }
    //If no keys or values
    else {
        printf("There were no keys in - %s\n", path);
    }
    //Close the key and free the valueVal
    RegCloseKey(default_key);
    return 0;
}

//Function to list contents of a directory and return a char* array with the file/folder names
int dirListing(char* folderPath) {
    //Add wildcard to list all files
    char* folderPathWild = (char*)calloc(1, 255);
    memcpy_s(folderPathWild, 255, folderPath, 255);
    strcat_s(folderPathWild, 255, "*");

    //Get first file in dir and handle for enum
    printf("\n[+] Enumerating extensions\n\n\n\n");


    //Set up variables for spidering further
    WIN32_FIND_DATAA dirFile;
    char* extDir = (char*)"";
    char* manifestPath = (char*)"";
    BOOL dirSpider = TRUE;
    BOOL dirSpider1 = TRUE;
    char* manifestPathBase = (char*)"";
    char* manifestPathWild = (char*)"";

    //Loop through the directory contents
    HANDLE dirHandle = FindFirstFileA(folderPathWild, &dirFile);
    FindNextFileA(dirHandle, &dirFile);

    //Loop through each item in the directory, that is a folder until there are no more
    while (dirSpider != FALSE) {
        dirSpider = FindNextFileA(dirHandle, &dirFile);
        if ((dirSpider != FALSE) && (dirFile.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) && (strcmp(dirFile.cFileName, "Temp") != 0)) {

            printf("[+] Extension - %s\n", dirFile.cFileName);
            //When a dir is found, concatenate that with the current full path and then end it with "\\"
            char* extDir = (char*)calloc(1, 255);
            memcpy_s(extDir, 255, folderPath, 255);
            strcat_s(extDir, 255, dirFile.cFileName);
            strcat_s(extDir, 255, "\\");
            manifestPathWild = (char*)calloc(1, 255);
            memcpy_s(manifestPathWild, 255, extDir, 255);
            strcat_s(manifestPathWild, 255, "*");

            //Within this new folder, find the folder with the manifest (should only be one folder)
            HANDLE nextFolder = FindFirstFileA(manifestPathWild, &dirFile);
            FindNextFileA(nextFolder, &dirFile);
            dirSpider1 = FindNextFileA(nextFolder, &dirFile);

            //Go through each extension and parse for nativemessaging
            while (dirSpider1 != FALSE) {
                //If directory is found (there should be one)
                if (dirFile.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
                    //Build full path to manifest file
                    strcat_s(extDir, 255, dirFile.cFileName);
                    strcat_s(extDir, 255, "\\Manifest.json");
                    printf("[+] Path - %s\n", extDir);

                    //Call function to parse manifest file
                    manifestParser(extDir);
                    dirSpider1 = FindNextFileA(nextFolder, &dirFile);
                }
            }

            //Close dir handle and free buffers
            FindClose(nextFolder);
            free((PVOID)extDir);
            free((PVOID)manifestPathWild);

        }
    }

    //Close dir handle and free buffer
    FindClose(dirHandle);
    free((PVOID)folderPathWild);


    return 0;
}

//Function to list contents of a directory and return a char* array with the file/folder names
int fireFoxDirListing(char* folderPath) {

    char* folderPathWild = (char*)calloc(1, 255);
    memcpy_s(folderPathWild, 255, folderPath, 255);
    strcat_s(folderPathWild, 255, "*");

    //Get first file in dir and handle for enum
    printf("\n[+] Enumerating profiles [+]\n[#]----------------------[#]\n");


    //Set up variables for spidering frther
    WIN32_FIND_DATAA dirFile;
    char* extDir = (char*)"";
    char* profilePath = (char*)"";
    BOOL dirSpider = TRUE;
    BOOL dirSpider1 = TRUE;
    char* profilePathWild = 0;

    //Loop through the directory contents
    HANDLE dirHandle = FindFirstFileA(folderPathWild, &dirFile);
    FindNextFileA(dirHandle, &dirFile);

    //Loop through each item in the directory, that is a folder until there are no more
    while (dirSpider != FALSE) {
        dirSpider = FindNextFileA(dirHandle, &dirFile);
        if ((dirSpider != FALSE) && (dirFile.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)) {

            //When a dir is found, concatenate that with the current full path and then end it with "\\"
            char* extDir = (char*)calloc(1, 255);
            memcpy_s(extDir, 255, folderPath, 255);
            strcat_s(extDir, 255, dirFile.cFileName);
            strcat_s(extDir, 255, "\\extensions\\");
            profilePathWild = (char*)calloc(1, 255);
            memcpy_s(profilePathWild, 255, extDir, 255);
            strcat_s(profilePathWild, 255, "*");

            //The main profile appears to have '-release' as a suffix for FireFox, so this folder is only searched
            if (strstr(extDir, "-release") != 0) {
                printf("\n[+] Profile - %s\n", dirFile.cFileName);
                //Within this new folder, find the folder with the manifest (should only be one folder)
                HANDLE nextFolder = FindFirstFileA(profilePathWild, &dirFile);
                FindNextFileA(nextFolder, &dirFile);
                dirSpider1 = FindNextFileA(nextFolder, &dirFile);

                printf("[+] Download the following files, unzip them locally and run 'grep -nriF nativeMessaging' in the folder with the extracted folders.\nIf any extension's manifest.json contains this string in the permissions, it is potentially vulnerable:\n");

                //Go through each extension and print the path
                while (dirSpider1 != FALSE) {
                    //Build full path to manifest file
                    printf("%s%s", extDir, dirFile.cFileName);


                    dirSpider1 = FindNextFileA(nextFolder, &dirFile);
                }
                //Close dir handle and free buffers
                FindClose(nextFolder);
                free((PVOID)extDir);
                free((PVOID)profilePathWild);

            }


        }
    }

    //Close dir handle and free buffer
    FindClose(dirHandle);
    free((PVOID)folderPathWild);

    return 0;
}

/*
Lists all extensions and if they are vulnerable for chrome and edge
*/

void extensionRecon(int verbose)
{
    //Set vars for environment var to expand and paths for extensions
    char appData[255] = "%LOCALAPPDATA%";
    char appDataRoam[255] = "%APPDATA%";
    char chromePath[255] = "\\Google\\Chrome\\User Data\\Default\\";
    char edgePath[255] = "\\Microsoft\\Edge\\User Data\\Default\\";
    char firefoxPath[255] = "\\mozilla\\firefox\\profiles\\";
    char* appDataExpand = (char*)calloc(1, 255);
    char* appDataRoamExpand = (char*)calloc(1, 255);
    WIN32_FIND_DATAA dirFile;

    //Expand the appdata env var
    DWORD success = ExpandEnvironmentStringsA(appData, appDataExpand, 255);
    int appDataExpandLen = strlen(appDataExpand);

    //Error handling
    if (success == 0) {
        printf("[-] Something went wrong expanding the roaming local APPDATA environment variable\n");
        return;
    }

    //Expand the appdata roaming env var
    success = ExpandEnvironmentStringsA(appDataRoam, appDataRoamExpand, 255);
    int appDataRoamExpandLen = strlen(appDataRoamExpand);

    //Error handling
    if (success == 0) {
        printf("[-] Something went wrong expanding the APPDATA environment variable\n");
        return;
    }


    //Create a buffer for the full chrome path by getting the length of the env var + endpoint + 1
    int chromeExtensionLen = strlen(chromePath) + 12;
    int chromeLen = strlen((char*)chromePath);
    char* appDataExpandChrome = (char*)calloc(1, 255);
    memcpy_s(appDataExpandChrome, 255, appDataExpand, appDataExpandLen);
    strcat_s((char*)appDataExpandChrome, 255, chromePath);
    char* chromeBuf = (char*)calloc(1, 255);
    memcpy_s(chromeBuf, 255, appDataExpandChrome, appDataExpandLen + chromeLen);
    strcat_s((char*)chromeBuf, 255, "Extensions\\");

    //Create a buffer for the full edge path by getting the length of the env var + endpoint + 1
    int edgeExtensionLen = strlen(edgePath) + 12;
    int edgeLen = strlen((char*)edgePath);
    char* appDataExpandEdge = (char*)calloc(1, 255);
    memcpy_s(appDataExpandEdge, 255, appDataExpand, appDataExpandLen);
    strcat_s((char*)appDataExpandEdge, 255, edgePath);
    char* edgeBuf = (char*)calloc(1, 255);
    memcpy_s(edgeBuf, 255, appDataExpandEdge, appDataExpandLen + edgeLen);
    strcat_s((char*)edgeBuf, 255, "Extensions\\");

    //Create a buffer for the full firefox path by getting the length of the env var + endpoint + 1
    int firefoxExtensionLen = strlen((char*)firefoxPath);
    char* firefoxBuf = (char*)calloc(1, 255);
    memcpy_s(firefoxBuf, 255, appDataRoamExpand, appDataRoamExpandLen);
    strcat_s(firefoxBuf, 255, (char*)firefoxPath);

    printf("\n\n\n==========\n[+] Chrome\n==========\n\n\n");

    //Check if file exists to see if chrome is installed
    DWORD exists = GetFileAttributesA(appDataExpandChrome);
    //If retrieving the handle fails, return error
    if (exists == INVALID_FILE_ATTRIBUTES) {
        printf("[-] Chrome does not appear to be installed! \n\n");

    }
    else {
        //Call function to list files in directory
        dirListing(chromeBuf);
        printf("\n[+] Enumerating existing Chrome NativeMessaging Application's manifest paths [+]\n[#]--------------------------------------------------------------------------[#]\n\n[+] HKCU:\n");
        enumKeys((char*)"Software\\Google\\Chrome\\NativeMessagingHosts", HKEY_CURRENT_USER, verbose);
        printf("\n\n[+] HKLM:\n");
        enumKeys((char*)"Software\\Google\\Chrome\\NativeMessagingHosts", HKEY_LOCAL_MACHINE, verbose);
        printf("\n\n[+] Enumerating managed Chrome extensions [+]\n[#]---------------------------------------[#]\n\n[+] HKCU:\n\n");
        enumValues((char*)"Software\\Policies\\Google\\Chrome\\ExtensionInstallForcelist", HKEY_CURRENT_USER, verbose,1);
        printf("\n\n[+] HKLM:\n\n");
        enumValues((char*)"Software\\Policies\\Google\\Chrome\\ExtensionInstallForcelist", HKEY_LOCAL_MACHINE, verbose,1);
    }

    //printf("\n\n\n[+]-------------------------------------------------------------[+]\n\n\n[+] Edge\n========\n\n\n");
    printf("\n\n\n========\n[+] Edge\n========\n\n\n");
    exists = 0;
    //Check if file exists to see if edge is installed
    exists = GetFileAttributesA(appDataExpandEdge);
    //If retrieving the handle fails, return error
    if (exists == INVALID_FILE_ATTRIBUTES) {
        printf("[-] Edge does not appear to be installed!\n\n");
    }
    else {
        //Call function to list files in directory
        dirListing(edgeBuf);
        printf("\n[+] Enumerating existing Edge NativeMessaging Application's manifest paths [+]\n[#]------------------------------------------------------------------------[#]\n\n[+] HKCU:\n\n");
        enumKeys((char*)"Software\\Microsoft\\Edge\\NativeMessagingHosts", HKEY_CURRENT_USER, verbose);
        printf("\n\n[+] HKLM:\n");
        enumKeys((char*)"Software\\Microsoft\\Edge\\NativeMessagingHosts", HKEY_LOCAL_MACHINE, verbose);
        printf("\n\n[+] Enumerating managed Edge extensions [+]\n[#]-------------------------------------[#]\n\n[+] HKCU:\n\n");
        enumValues((char*)"Software\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist", HKEY_CURRENT_USER, verbose,1);
        printf("\n\n[+] HKLM:\n\n");
        enumValues((char*)"Software\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist", HKEY_LOCAL_MACHINE, verbose,1);

    }

    //printf("\n\n\n[+]-------------------------------------------------------------[+]\n\n\n[+] FireFox\n===========\n\n\n");
    printf("\n\n\n===========\n[+] FireFox\n===========\n\n\n");
    exists = 0;
    //Check if file exists to see if firefox is installed
    exists = GetFileAttributesA(firefoxBuf);
    //If retrieving the handle fails, return error
    if (exists == INVALID_FILE_ATTRIBUTES) {
        printf("[-] FireFox does not appear to be installed!\n\n");
    }
    else {
        //Call function to list files in directory
        fireFoxDirListing(firefoxBuf);
        printf("\n\n[+] Enumerating existing FireFox NativeMessaging Application's manifest paths [+]\n[#]---------------------------------------------------------------------------[#]\n\n[+] HKCU:\n\n");
        enumKeys((char*)"Software\\Mozilla\\NativeMessagingHosts", HKEY_CURRENT_USER, verbose);
        printf("\n\n[+] HKLM:\n");
        enumKeys((char*)"Software\\Mozilla\\NativeMessagingHosts", HKEY_LOCAL_MACHINE, verbose);

    }

    //Free buffers
    free((PVOID)appDataExpand);
    free((PVOID)chromeBuf);
    free((PVOID)edgeBuf);
    free((PVOID)firefoxBuf);
    free((PVOID)appDataExpandChrome);
    free((PVOID)appDataExpandEdge);

    return;
}

