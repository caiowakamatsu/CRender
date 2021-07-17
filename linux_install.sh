echo "Commecing the install of this garbage";
read -p "Are you sure? " -n 1 -r
echo 
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo -e "PLEASE INSTALL" '\e]8;;https://software.intel.com/content/www/us/en/develop/tools/oneapi/rendering-toolkit/download.html\ahttps://software.intel.com/content/www/us/en/develop/tools/oneapi/rendering-toolkit/download.html\e]8;;\a'
    exit 1;
fi
echo "Now I will download the dozen or so dependinces needed";
sudo apt install libxi-dev build-essential xorg libglfw3 libglfw3-dev libxinerama-dev libxcursor-dev