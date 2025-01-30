#include <windows.h>
#include <stdio.h>
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"


//Defining functions specified in other files
void extensionRecon(int verbose);
void extensionInstall(char* extensionId, char* browser, char* allUsers, int verbose);
void extendablesPersist(char* extensionId, char* extensionName, char* targetPath, char* browser, char* nativePath, char* allUsers, char* description, char* force, int verbose);
void shortcutGetArgs(LPCCH lpszLinkFile);
void shortcutEdit(LPCCH lpszLinkFile, char additionalArgs[MAX_PATH]);