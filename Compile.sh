#
apt install libncurses5-dev libncursesw5-dev
apt install libx11-dev
gcc lmatrix.c -o lmatrix -lncurses -lX11
