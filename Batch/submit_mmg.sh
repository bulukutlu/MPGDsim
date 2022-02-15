#!/bin/bash

cpu=5
mem=100

program=/scratch7/bulukutlu/MPGDsim/Garfield/
# define job base directory and create the output directory
starttime=$(date +%F_%H%M)
jobBase=/scratch7/bulukutlu/MPGDsim/Data/
echo "'$jobBase'"
MMG=MMG1
Edrift=100
#______________________________________________________________________________________________
# submit the job
job1=$(sbatch --time=00:30:00 --mem-per-cpu=4000 --cpus-per-task=1 --array=1-$numberOfJobs -J ${templateName} -o ${jobBase}/log/test.%A_%a.out.log -e ${jobBase}/log/test.%A_%a.err.log $templateDir/run_test.sh  $starttime $jobBase $numberOfJobs $templateArgs)
