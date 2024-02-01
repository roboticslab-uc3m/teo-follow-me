## Teo-follow-me demonstration: Installation from Source Code

First install the dependencies:
- [Install CMake 3.16+](https://github.com/roboticslab-uc3m/installation-guides/blob/master/docs/install-cmake.md)
- [Install YCM 0.11+](https://github.com/roboticslab-uc3m/installation-guides/blob/master/docs/install-ycm.md)
- [Install YARP 3.6+](https://github.com/roboticslab-uc3m/installation-guides/blob/master/docs/install-yarp.md)

### Install teo-follow-me demo on Ubuntu

Our software integrates the previous dependencies. Note that you will be prompted for your password upon using `sudo` a couple of times:

```bash
cd  # go home
mkdir -p repos; cd repos  # create $HOME/repos if it does not exist; then, enter it
git clone https://github.com/roboticslab-uc3m/teo-follow-me.git  # Download teo-follow-me demostration software from the repository
cd follow-me; mkdir build; cd build; cmake ..  # Configure the teo-follow-me demostration software
make -j$(nproc)  # Compile
sudo make install  # Install :-)
cp ../scripts/gnome/teo-follow-me.desktop $HOME/Desktop  # Nice desktop icon. May require some updates to point to path and icon.
```

For additional options use `ccmake` instead of `cmake`.
