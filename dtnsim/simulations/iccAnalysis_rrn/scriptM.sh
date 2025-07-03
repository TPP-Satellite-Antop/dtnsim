#!/bin/bash

#########################################################################

cd rrn_cgr1_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..

cd rrn_cgr3_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..

cd rrn_cgr_proactive_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..

cd rrn_sprayAndWait_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..

cd rrn_sprayAndWait_4copies_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..

cd rrn_epidemic_1x
chmod +x scriptMain.sh
chmod +x script.sh
chmod +x script2.sh
./scriptMain.sh
cd ..


: <<'END'
END
###########################################################################

#cd cgr-spray_1x
#./scriptMain.sh
#cd ..






