sudo apt-get install build-essential
sudo apt-get install g++-multilib
sudo apt-get install gcc-multilib
sudo apt-get install mesa-common-dev
sudo apt-get install libgl1-mesa-dev
sudo apt-get install libglu1-mesa-dev

sudo apt-get install ia32-libs

ln -s /usr/lib/i386-linux-gnu/mesa/libGL.so.1 /usr/lib/i386-linux-gnu/libGL.so
ln -s /usr/lib/i386-linux-gnu/libGLU.so.1 /usr/lib/i386-linux-gnu/libGLU.so
ln -s /usr/lib/i386-linux-gnu/libX11.so.6 /usr/lib/i386-linux-gnu/libX11.so
ln -s /usr/lib/i386-linux-gnu/libXrandr.so.2 /usr/lib/i386-linux-gnu/libXrandr.so

