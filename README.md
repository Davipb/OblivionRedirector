# OblivionRedirector
This is a set of helpers created for the Steam release of [Nehrim: At Fate's Edge](https://sureai.net/games/nehrim/) in order to make it (almost) independent from Oblivion.
It is composed of two main parts:
* **OblivionLauncher** is a replacement for the OblivionLauncher.exe executable that allows the game to launch without passing through the vanilla Oblivion launcher. Additionally, it injects OblivionRedirector.dll into the game when it launches.
* **OblivionRedirector** is a DLL that, when injected into Oblivion, redirects file operations from Documents\My Games\Oblivion\Oblivion.ini and AppData\Local\Oblivion\Plugins.txt to the folder where the DLL is located.   
Although Oblivion naturally supports reading those files from its own directory, it still tries do some operations on the external files, which could cause issues if a player had both Nehrim and Oblivion installed at the same time. The redirector remedies that.

This project is under the MIT License, as is its one dependency, [Microsoft's Detours library](https://github.com/Microsoft/Detours).
