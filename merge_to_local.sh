#!/bin/bash

echo 'Copying git-remote-synced repository to local repository'

cp -r missions/line_follow_w_dyn_wpts_and_smpl_data ~/moos-ivp/ivp/missions/
cp -r src/pLineFollow/ src/pLineTurn/ src/pIncludeSampleData/ src/pSimDistanceGenerator/ ~/moos-ivp/ivp/src/
