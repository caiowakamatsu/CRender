read -p "We are about to start downloading dependencies, and compiling this project. Are you sure you would like to continue?" -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "PLEASE INSTALL" '\e]8;;https://software.intel.com/content/www/us/en/develop/tools/oneapi/rendering-toolkit/download.html\ahttps://software.intel.com/content/www/us/en/develop/tools/oneapi/rendering-toolkit/download.html\e]8;;\a'
    exit 1
fi

read -p "Are you on Ubuntu or a Debian Based distribution which maintains these packages and ensures they're up to date?" -n 1 -r
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Please install the equivalent of these on your system! libxi-dev build-essential xorg libglfw3 libglfw3-dev libxinerama-dev libxcursor-dev"
    exit 1
else
    sudo apt install libxi-dev build-essential xorg libglfw3 libglfw3-dev libxinerama-dev libxcursor-dev

fi

echo "COMMENCING CACHE BUILDING"
cmake -DCMAKE_BUILD_TYPE=RELEASE .
echo "COMMENCING ACTUAL BUILD"
echo "Setting vars, we are assuming you've installed Embree and such from Intel"

source /opt/intel/oneapi/setvars.sh
cmake --build . --target CRender
