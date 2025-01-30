#include "extendables.h"
//Code based on - https://learn.microsoft.com/en-us/windows/win32/shell/links

//Taskbar shortcuts are stored within - %APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar
//Also make sure to check the desktop - %HOMEPATH%\Desktop

//Function to retrieve shortcut arguments
void shortcutGetArgs(LPCCH lpszLinkFile) {
    //Initialising variables
    IShellLink* psl;
    WIN32_FIND_DATA wfd;
    WCHAR szGotPath[MAX_PATH];
    WCHAR args[MAX_PATH];
    DWORD exists = 0;


    //Opens file attributes to check if it exists
    exists = GetFileAttributesA(lpszLinkFile);

    if (exists == INVALID_FILE_ATTRIBUTES) {
        printf("\n\n\nFile - %s does not appear to exist!\n", lpszLinkFile);
        return;
    }

    //Initialise COM
    CoInitialize(NULL);

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called. 
    CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface. 
    psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

    WCHAR wsz[MAX_PATH];

    // Ensure that the string is Unicode. 
    MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);


    // Load the shortcut. 
    ppf->Load(wsz, STGM_READ);

    
    // Resolve the link. 
    psl->Resolve(((HWND)-1), 0);

    
    //Get current target filepath of shortcut
    psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

    wprintf(L"\n\n\n[+]Current target filepath - %s\n", szGotPath);

    //Get arguments for current LNK
    psl->GetArguments(args, MAX_PATH);
    if (wcscmp(args, L"") == 0) {
         printf("[-] There are no arguments for this shortcut\n");
    }
    else {
         wprintf(L"[+] Current arguments - %s\n", args);
    }


    // Release the pointer to the IPersistFile interface. 
    ppf->Release();
   

    // Release the pointer to the IShellLink interface. 
    psl->Release();
  
    //Uninitialise COM after we have finished using it
    CoUninitialize();

    return;
}

//Function to append arguments to a shortcut
void shortcutEdit(LPCCH lpszLinkFile, char* additionalArgs){
    //Initialising variables
    IShellLink* psl;
    WCHAR args[MAX_PATH];
    DWORD exists = 0;


    //Opens file attributes to check if it exists
    exists = GetFileAttributesA(lpszLinkFile);

    if (exists == INVALID_FILE_ATTRIBUTES) {
        printf("\n\n\nFile - %s does not appear to exist!\n", lpszLinkFile);
        return;
    }

    //Initialise COM
    CoInitialize(NULL);

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called. 
    CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface. 
    psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

    WCHAR wsz[MAX_PATH];

    WCHAR additionalArgsW[MAX_PATH];

    // Ensure that the string is Unicode. 
    MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH*2);
    MultiByteToWideChar(CP_ACP, 0, additionalArgs, -1, additionalArgsW, MAX_PATH*2);


    // Load the shortcut. 
    ppf->Load(wsz, STGM_READ);

    // Resolve the link. 
    psl->Resolve(((HWND)-1), 0);


    //Get arguments for current LNK
    psl->GetArguments(args, MAX_PATH);

    wprintf(L"\n\n\n[+] Old arguments - %s\n", args);

    //Concatenate our arguments on the end of the commandline
    wcscat_s(args, MAX_PATH, additionalArgsW);

    //Add our arguments
    psl->SetArguments(args);

    psl->GetArguments(args, MAX_PATH);
    wprintf(L"[+] New args - %s\n", args);


    //Save it all
    ppf->Save(wsz, TRUE);


    // Release the pointer to the IPersistFile interface. 
    ppf->Release();
    
    // Release the pointer to the IShellLink interface. 
    psl->Release();

    //Uninitialise COM after we have finished using it
    CoUninitialize();  

    return;
}