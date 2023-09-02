# phoenix-firestorm

This fork is being maintained by a viewer developer using Arch linux and opensim.

To configure follow the following instructions.

1. Don't even think about this if your PC has less than 16 GB of ram; 32 GB ram plus 32 GB swap is highly recommended.

2. Find a partition with *at least* 36 GB free (read: 64 GB), and create a new directory in it where you will put everything related to this viewer
(not including an optional ccache, which should have a size of another 32 GB, but can be put in a different partition of course).
For example:
```
$ df -H /opt
Filesystem               Size  Used Avail Use% Mounted on
/dev/mapper/NVME1T1-opt  580G  348G  203G  64% /opt

$ export TOPPROJECT=/opt/secondlife/viewers/firestorm
$ mkdir -p "$TOPPROJECT"
```
This directory must be writable by you; we're not going to do this as root.

As I have over 200 GB free in this partition, I also add my ccache here:
```
$ export CCACHE_DIR=/opt/ccache
$ sudo mkdir -p "$CCACHE_DIR"
$ sudo chown yourname "$CCACHE_DIR"
$ sudo pacman -S --needed ccache
$ ccache --max-size 32
```

3. Change directory into it and clone the repository into `phoenix-firestorm-git`:
```
$ cd "$TOPPROJECT"
$ git clone https://github.com/AlericInglewood/phoenix-firestorm.git phoenix-firestorm-git
$ export REPOBASE="$TOPPROJECT/phoenix-firestorm-git"
```
We also set `REPOBASE` to that directory, so that now `$REPOBASE/indra` exists.

4. Create a virtual environment for python3 and install autobuild and llsd.
```
$ python3 -m venv "$TOPPROJECT/venv"
$ export PATH="$TOPPROJECT/venv/bin:$PATH"
$ pip install autobuild llsd
```
Note that pip in the last line should run `$TOPPROJECT/venv/bin/pip` because of the added path to `PATH`
and likewise `which autobuild` now should return `$TOPPROJECT/venv/bin/autobuild`.

5. Clone the repository `fs-build-variables`:
```
$ cd "$TOPPROJECT"
$ git clone https://github.com/AlericInglewood/fs-build-variables.git
$ export AUTOBUILD_VARIABLES_FILE="$TOPPROJECT/fs-build-variables/variables"
$ export AUTOBUILD_ADDRSIZE=64
```

6. [Optional] Download and build the `3p-fmodstudio` package.

We start with cloning the repository `3p-fmodstudio` into `3p-fmodstudio-git`:
```
$ cd "$TOPPROJECT"
$ git clone https://github.com/FirestormViewer/3p-fmodstudio 3p-fmodstudio-git
$ cd 3p-fmodstudio-git
$ grep '^FMOD_VERSION_PRETTY' build-cmd.sh
```
Open the file called `build-cmd.sh` and look at the sixth line down, it begins with `FMOD_VERSION_PRETTY=`.
This is the version of the API you need to download.
The FMOD Studio API can be downloaded [here](https://www.fmod.com/) (requires creating a free account to access the download section).
Register and login as needed, then go to https://www.fmod.com/download#fmodengine . Click `FMOD Engine` to expand the section if it isn't already.
Click the button representing the required version, then click the Download link for the Linux file.
Copy that file to the `3p-fmodstudio-git` directory.

Next, build `3p-fmodstudio` and create an autobuild package:
```
$ cd "$TOPPROJECT/3p-fmodstudio-git"
$ export AUTOBUILD_BUILD_ID=$(date +%Y%m%d%H%M)
$ AUTOBUILD_CONFIG_FILE="autobuild.xml" autobuild build --all
$ AUTOBUILD_CONFIG_FILE="autobuild.xml" autobuild package
```
This will end with a line saying it wrote the package, for example something similar to:
```
wrote  /opt/secondlife/viewers/firestorm/3p-fmodstudio-git/fmodstudio-2.03.07-linux64-202507010106.tar.bz2
```
Lets store this path in an environment variable and store the md5sum of the file in another environment variable:
```
$ FMODPKG=$TOPPROJECT/3p-fmodstudio-git/fmodstudio-$FMOD_VERSION_PRETTY-linux64-$AUTOBUILD_BUILD_ID.tar.bz2   # Use the actual path that you got.
$ unset AUTOBUILD_BUILD_ID
$ FMODMD5=$(md5sum $FMODPKG | sed -e 's/ .*//')
$ echo $FMODMD5
```
Make sure that `FMODMD5` contains a sensible value (ie, set the FMOD_VERSION_PRETTY environment variable first).

Finally add the just installed `fmodstudio` as a package to a newly created `my_autobuild.xml`, using autobuild:
```
$ cd "$REPOBASE"
$ cp autobuild.xml my_autobuild.xml
$ export AUTOBUILD_CONFIG_FILE="$REPOBASE/my_autobuild.xml"
$ autobuild installables edit fmodstudio platform=linux64 hash="$FMODMD5" url="file://$FMODPKG"
```

7. Configure the viewer.

By now you should have the following environment variable set:

```
# Change to whatever you used:
export TOPPROJECT=/opt/secondlife/viewers/firestorm
export REPOBASE=$TOPPROJECT/phoenix-firestorm-git

# Prepend python virtual environment to PATH.
pre_path $TOPPROJECT/venv/bin

export AUTOBUILD_CONFIG_FILE=$REPOBASE/my_autobuild.xml
export AUTOBUILD_VARIABLES_FILE=$TOPPROJECT/fs-build-variables/variables
export AUTOBUILD_ADDRSIZE=64

# Change to whatever you used:
export CCACHE_DIR=/opt/ccache
```
Additionally set the following environment variables.
You might want to use http://carlowood.github.io/howto/cdeh.html for this
and just put all of these variables in `$TOPPROJECT/env.source`.
```
# Something.
export AUTOBUILD_BUILD_ID=aleric
export XZ_DEFAULTS="-T0"
export PYTHONIOENCODING="utf-8"
# Set this to automatically run the viewer in gdb whenever you run ./firestorm.
#export LL_WRAPPER='gdb --args'

# Pick one of these three.
#CMAKE_CONFIG=Release
CMAKE_CONFIG=RelWithDebInfo
#CMAKE_CONFIG=Debug
```
Then configure with:
```
$ git switch aleric
$ autobuild configure -c ${CMAKE_CONFIG}FS_open -- --fmodstudio --compiler-cache -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
```
The FS in `FS_open` stands for FmodStudio. Use `-c ${CMAKE_CONFIG}OS` if you don't want to use `--fmodstudio` (`_open` and `OS` stand for OpenSim).

**Important**: all work is done on the branch `aleric`. The `master` branch is just upstream
and other branches are tests or whatnot (not interesting for you).

8. Building the viewer.
```
$ autobuild build -c ${CMAKE_CONFIG}FS_open --no-configure -- --fmodstudio --compiler-cache -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
```

