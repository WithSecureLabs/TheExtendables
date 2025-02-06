# TheExtendables

<img width="293" height="459" alt="extensionBlogLogo" src="https://github.com/user-attachments/assets/9e4be8df-8da2-4a86-9ac7-0ab34b56c41e" />

This is a tool, based on the Wild West Hackin' Fest 2025 Mile High talk by WithSecure under the same title.

It tool has 5 distinct functionalities:

- recon - It can identify vulnerable extensions on a machine
- install - It can install arbitrary extensions on to a machine for exploitation
- lnkTarget - Recovers the target path of a shortcut file
- lnkBackdoor - Backdoors a browser shortcut to load a specified extension
- persist - It can exploit the functionality in the talk to persist on a users machine or allow for privilege escalation

Some of the tool's features can be seen in the video below:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=dZ5lgoIHcjo
" target="_blank"><img src="http://img.youtube.com/vi/dZ5lgoIHcjo/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="360" height="240" border="10" /></a>


# Example commands

(All command and argument names are case insensitive and can be specified in `/argument`, `--argument` or `-argument` format)

## Recon

For each browser, this command will, enumerate all installed extensions and check if they have the native messaging permissions. If they do, it will say `Potentially vulnerable extension!` if not it will say `Not vulnerable!`. For firefox, as the extensions are in an archive, it will return the paths to the archives in question for you to retrieve and run `grep -nriF nativeMessaging` on. If the manifest contains this permission it will be vulnerable.

Then it will enumerate all of the NativeMessaging applications manifest paths from the registry (both HKCU and HKLM), returning the extension name from the value and the path to the extention manifest json file on disk. If you can overwrite this, you can hijack that extension.

Then it will enumerate the registry for all "Managed" extensions which were force installled. There may not be any.


### Command

```TheExtendables.exe /recon```

### Optional arguments

You can optionally specify `/verbose` for more verbosity in the output.


## Install

This will use the force install registry key to install an extension either for the current user or for all users on a system. This usually requires administrative privileges and only works in chromium-based browsers (eg. chrome, edge, etc.). It will just add the extension ID as a value in this key and when the browser is next opened it will be force installed from the respective extension store.

If you are an administrator, it is possible to add the registry key through regedit. You will need to add the following value for chrome in either HKLM or HKCU depending on if you want it for all users or the current user:

`Software\Policies\Google\Chrome\ExtensionInstallForcelist`

You will need to add the following value for edge in either HKLM or HKCU depending on if you want it for all users or the current user:

`Software\Policies\Microsoft\Edge\ExtensionInstallForcelist`


The value within this key should be named with a number starting at `1` if none exist. If keys do exist, it should just be the next iteration of the number. Then the data within that value should be the following:

`<EXTENSION ID>;https://clients2.google.com/service/update2/crx`

### Command


```TheExtendables.exe /install /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb```

### Required arguments

Where `anffbmbabbpmcaflbelkkhgdcfgppknb` is an extension ID. This can be retrieved by opening up either `chrome://extensions/` or `edge://extensions/` in your browser (depending on what you are targeting), then view the details of an extension, which should append `?id=<EXTENSION ID>` to the URL from which you can take the extensionID. Or you could run the `/recon` command to list all installed extensions.
### Optional arguments

You can optionally specify `/verbose` for more verbosity in the output.

You can also specify `/browser` for the taget brower (chrome, edge)

And you can specify `/allUsers` to install for `user` which is the current user or `machine` which installs for all users.

## LnkTarget

This will use COM to retrieve the target path and agruments of a specified shortcut (`.lnk`) file.


### Command

```TheExtendables.exe /lnkTarget /lnkPath "%HOMEPATH%\\Desktop\\Google Chrome.lnk"```

### Required arguments

Where `"%HOMEPATH%\\Desktop\\Google Chrome.lnk"` is the path to an LNK file to retrieve the target executable and arguments.


## LnkBackdoor

This will add a `--load-extension=` argument with the path to an unpackaged extension to a given shortcut file through COM. If there are no Group Policy Objects or registry keys in-place blocking loading extensions and the target browser is chromium-based, the next time the shortcut is lauched, the extension will be loaded.

Useful shortcut paths that users likely use include:

- Their desktop - `%HOMEPATH%\\Desktop\\`
- Their taskbar - `%APPDATA%\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar\\`
- Their start menu - `%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\`
- All user's start menu - `%PROGRAMDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\`

### Command

```TheExtendables.exe /lnkBackdoor /lnkPath "%HOMEPATH%\\Desktop\\Google Chrome.lnk" /extensionPath C:\\ProgramData\\extension\\```

### Required arguments

Where `C:\\ProgramData\\extension\\` is the path to a folder with the extension source code.

### Optional arguments

You can optionally specify `/browser` for the taget brower (chrome, edge)

## Persist

This feature creates a registry key and NativeMessaging manifest to weaponise and exploit an installed NativeMessaging extension.

### Command

```TheExtendables.exe /persist /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb /extensionName com.companyName.extensionsName /targetPath C:\\windows\\system32\\calc.exe```

### Required arguments

Where `anffbmbabbpmcaflbelkkhgdcfgppknb` is an extension ID. 

Where `com.companyName.extensionName` is a name to call the extension (this can be anything, just follow the `com.compayName.extensionName` format). To find this look into the target extension's JavaScript and search for where `runtime.connectNative()` is used. The value passed to this in the `com.compayName.extensionName` format is the extension name. Alternatively, you could install the extension legitimately with the NativeMessaging functionality on a seperate machine and look at the NativeMessaging manifest and retrieve the extension name from there.

Where `C:\\windows\\system32\\calc.exe` is the path to the NativeMessaging host application, script or file.

### Optional arguments

You can optionally specify `/verbose` for more verbosity in the output.

You can also specify `/browser` for the taget brower (chrome, edge)

You can specify `/allUsers` to install for `user` which is the current user or `machine` which installs for all users.

You can supply `/nativePath` which is the path where the NativeMessaging manifest will be created.

`/description` can be supplied to change the description of the native application.

You can add `/force` to your command to ignore any mutex's that show this as having been already run.

# Example Extension

Additionally, this repository also contains the source code for an example vulnerable extension, which can be loaded onto a target machine for persistence using one of the methods above.

Instructions are also included on how to make your own vulnerable extension in the `extension-src` directory.

# Defensive Considerations

There are a number of steps you can take to protect yourself/alert on this behaviour:

- Apply group policy/registry keys to stop extension loading
- Ensure NativeMessaging applications, manifests and registry keys are correctly permissioned
- Create alerts for if a registry key is created under NativeMessaging for a browser
- Create alerts for if a `ExtensionInstallForcelist` registry key value is created for a browser
- Create alerts for if `—load-extension=` is in a commandline
- Create alerts as NVISO described in their blog for uncommon child processes (https://blog.nviso.eu/2018/12/21/detecting-suspicious-child-processes-using-ee-outliers-and-elasticsearch/)


# Help menu

The tool's help page can be seen below:
```
[+] Main Commands [+]
[#]===============[#]
(No main commands take arguments)

/recon        - Perform reconaissance on installed extensions
[OPTIONAL     - /verbose]

/install      - Force installs an extension. REQUIRES ADMIN, chrome or edge only.
[REQUIRED     - /extensionID]
[OPTIONAL     - /browser, /allUsers, /verbose]

/lnkTarget    - Retrieves the path and arguments of a supplied shortcut file
[REQUIRED     - /lnkPath]

/lnkBackdoor  - Backdoors a shortcut to a browser, chrome or edge only.
[REQUIRED     - /lnkPath, /extensionPath]
[OPTIONAL     - /browser]
Taskbar shortcuts are stored within - %APPDATA%\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar
Also make sure to check the desktop - %HOMEPATH%\\Desktop

/persist      - Exploits a vulnerable extension to run arbitrary code through it
[REQUIRED     - /extensionID, /extensionName, /targetPath]
[OPTIONAL     - /browser, /allUsers, /verbose, /nativePath, /description, /force]


[+] Arguments [+]
[#]===========[#]
(All below take arguments apart from /verbose and /force)

/extensionID   - Identifier string for the extension, eg. anffbmbabbpmcaflbelkkhgdcfgppknb
/extensionName - Internal name of the extension, eg. com.companyName.extensionsName
/targetPath    - Path of exe or file with default app mapped, to run. eg. C:\\windows\\system32\\calc.exe
/browser       - Browser to target: chrome, edge or firefox. Default = chrome
/allUsers      - Install in HKLM or HKCU. user = HKCU, machine = HKLM (only works if admin). Default = user
/nativePath    - Path for native.json to be saved to. Default = C:\\ProgramData\\native.json.
/description   - Description of extension in native.json. Default - This is an extension that extends browser functionality
/lnkPath       - Path to shortcut file to retrieve target or backdoor. eg. %HOMEPATH%\Desktop\Google Chrome.lnk
/extensionPath - Path to packed extension. eg. C:\\ProgramData\\extensions.crx.
/verbose       - Adds additional verbosity to output (Takes no argument)
/force         - Ignore any mutex to edit or replace old payload (Takes no argument)


[+] Examples [+]
[#]==========[#]

Example: TheExtendables.exe /recon /verbose
Example: TheExtendables.exe /install /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb
Example: TheExtendables.exe /lnkTarget /lnkPath "%HOMEPATH%\\Desktop\\Google Chrome.lnk"
Example: TheExtendables.exe /lnkBackdoor /lnkPath "%HOMEPATH%\\Desktop\\Google Chrome.lnk" /extensionPath C:\\ProgramData\\extension\\
Example: TheExtendables.exe /persist /extensionID anffbmbabbpmcaflbelkkhgdcfgppknb /extensionName com.companyName.extensionsName /targetPath C:\\windows\\system32\\calc.exe
```
