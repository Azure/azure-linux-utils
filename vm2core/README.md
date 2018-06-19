Contact: Sean Spratt <seansp@microsoft.com>

OVERVIEW
=================
"vm2core" is a tool that can take saved state files from Hyper-V VM snapshots
and converts them into an ELF-format core dump that is readable by Linux
kernel analysis tools such as "crash".

The output format of the generated core dump is based on the same format that
is used by the Linux kernel's /proc/vmcore component.


### Microsoft Windows SDK Requirements ... 
You can build **vm2core** using the public Windows SDK however the supporting DLL is not present in the release.  This omission has been corrected in the Insider's Edition of the Windows SDK.  Insider Windows SDK releases including 10.0.0.**17686**.0 contain the file **_vmsavedstatedumpprovider.dll_**

## Installing the Insider Windows SDK (winkit)

The following steps assume Visual Studio 2017 is used for compilation...


## [Install Visual Studio and the Insider Windows SDK, minimum version 10.0.17686.0](https://www.microsoft.com/en-us/software-download/windowsinsiderpreviewSDK)
Once installed, verify that the redistributable _vmsavedstatedumpprovider.dll_ is present.  This file will default to _C:\Program Files(x86)\Windows Kits\10\bin\10.0.17686.0\x64_.
![winkit](/images/vmsavedstatedumpprovider_location.png)

You can also verify that the required **lib** and **h** files are present.
![VmSavedStateDumpProviderLib](/images/vmsavedstatedumpproviderlib_location.png)
![VmSavedStateDumpProviderHeaders](/images/vmsavedstatedumpproviderheaders_location.png)

## Create the Visual Studio Project
### Get the sources from github.
```bash
git clone https://github.com/seansp/azure-linux-utils.git
```
The sources for vm2core can be found in the vm2core folder.
```dos
cd azure-linux-utils
cd vm2core
cd src
dir
````

Now, include the main header file into the project to start coding! Make 
		sure Windows.h is included before this header.
	o	#include <vmsavedstatedump.h>
•	To compile, for some reason Visual Studio is not smart enough to include the
		library from the Windows SDK, and it needs to be included manually.
	o	Go to Project->Properties->Linker->Input->Additional Dependencies and 
		add a new entry vmsavedstatedumpprovider.lib.
	o	Make sure the build target is x64!

Now build and enjoy!

NOTE: You can share the header files and library as a redist, and add them 
manually onto a C++ project and its build environment and it should also work. 
This is helpful for projects that don't want to install the newest Windows SDK 
version, but want to link against this.
	o	Redist files:
		•	Vmsavedstatedump.h
		•	Vmsavedstatedumpdefs.h
		•	Vmsavedstatedumpprovider.lib

USAGE
=================
1) Take a checkpoint from your VM.
	Make sure the checkpoint is a STANDARD checkpoint vs a PRODUCTION checkpoint.
	

   

2) Find the saved state files for this snapshot. The location of the saved
state files can be found in the "Checkpoint File Location" setting in the
Hyper-V VM configuration.
Browse to your checkpoint location 
	(commonly C:\ProgramData\Microsoft\Windows\Hyper-V\Snapshots)
	On Server 2016 and Windows 10, you will find one VMRS file. 
	On Server 2012 R2​ you will find a folder with the GUID containing both the 
		BIN and VSV files.

3) Generate the "vmcore" dump with one of the following commands:

	vm2core.exe <.vsv file> <.bin file> <output file>

	vm2core.exe <.vmrs file> <output file>

NOTE: The tool relies on VmSavedStateDumpProvider.dll which needs to be in same 
  folder as the tool in order to run.

4) Copy the generated dump file to a Linux machine and load it into a debugger such as crash.
The generated output dump will be at least as large as the VM memory size.

For example, you can run this "crash" command:

	crash <System Map file> <Kernel image with debug info> <vmcore file>


CRASH CONFIGURATION
=================
 
Configuring crash to loa​​d your core.
Once you have created your core, you can load it using gdb or crash.  
Included below are details for installing kernel debug symbols and configuring crash 
(verified that this works using 7.1.5) on Centos 7.2 and Ubuntu 16.04.03
If you are checking our more than one kernel no worries, find is your friend.
find / -name System.map*
find / -name vmlinux
CentOS 7.2 ... 
(on centos, I performed all actions as root.)

Install the kernel debuginfo​ (in my case, it was just an instance of the same vm.  
Replace uname -r with your specific version if using a different kernel.
yum --enablerepo=base-debuginfo install kernel-debuginfo-$(uname -r) -y


Get Crash version 7.1.5 and build it.  Adding tools needed for ability to build before make.
````bash
yum install -y wget
wget https://github.com/crash-utility/crash/archive/7.1.5.tar.gz
tar -xvzf 7.1.5.tar.gz
​yum groupinstall "Development Tools" -y
yum install ncurses-devel -y
yum install -y zlib-devel
cd crash-7.1.5/
make
````

Start crash and examine the core (assuming it is ../centos7.2.1511.core)
./crash /usr/lib/debug/usr/lib/modules/3.10.0-327.el7.x86_64/vmlinux /boot/System.map-3.10.0-327.el7.x86_64 ../centos7.2.1511.core

​
​​Ubuntu 16.04.03 
(on ubuntu, i used my local user)
First get the build tools to make crash​​​​.
````bash
sudo apt-get install -y git build-essential libncurses5-dev zlib1g-dev bison make
wget, make, and​​ install crash.
wget https://github.com/crash-utility/crash/archive/7.1.5.tar.gz
tar -xvzf 7.1.5.tar.gz
cd crash-7.1.5/
make
sudo make install
cd ..
echo "Then get the ubuntu ​​​kernel symbols."
sudo tee /etc/apt/sources.list.d/ddebs.list <<EOF
deb http://ddebs.ubuntu.com/ $(lsb_release -c -s)          main restricted universe multiverse
deb http://ddebs.ubuntu.com/ $(lsb_release -c -s)-updates  main restricted universe multiverse
deb http://ddebs.ubuntu.com/ $(lsb_release -c -s)-proposed main restricted universe multiverse
EOF
sudo apt-key adv \
    --keyserver keyserver.ubuntu.com \
    --recv-keys 428D7C01 C8CAB6595FDFF622
sudo apt-get update -y
sudo apt-get install -y linux-image-$(uname -r)-dbgsym
echo "And finally run crash... (assuming xerus.core is your core)"
su​do crash /usr/lib/debug/boot/vmlinux-4.4.0-87-generic /boot/System.map-4.4.0-87-generic xerus.core
````

USEFUL AUTOMATION EXAMPLES ... 
=================
echo bt |crash -s /boot/System.map-4.4.0-87-generic /usr/lib/debug/boot/vmlinux-4.4.0-87-generic example2.core > logfile
echo ps -t |crash -s /boot/System.map-4.4.0-87-generic /usr/lib/debug/boot/vmlinux-4.4.0-87-generic example2.core > logfile
echo mod |crash -s /boot/System.map-4.4.0-87-generic /usr/lib/debug/boot/vmlinux-4.4.0-87-generic example2.core > logfile



REFERENCES
=================
[CentOS debug images](http://debuginfo.centos.org/)  (also applicable to Red Hat)

[Ubuntu debug images](http://ddebs.ubuntu.com/pool/main/l/linux/)

[SuSE Linux debug images](https://en.opensuse.org/Package_repositories#Debug)

[Microsoft Insider Program for Developers](https://insider.windows.com/en-us/for-developers/)

