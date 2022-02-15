#!/bin/bash

showParams() {
    echo "   $1 $(${SCRIPTDIR}/templates/$1/run_scratch.sh params)"
}

listTemplates() {
    echo "usage: ./submit.sh [debug] <number of jobs> <template> <template parameters>"
    echo
    echo Templates:
    for dir in $(ls -d $SCRIPTDIR/templates/*); do
	template=$(basename $dir)
	showParams $template
    done
}

checkTemplate() {
    if [ ! -d "$SCRIPTDIR/templates/$1" ]; then
	echo "Template '$1' does not exist"
	exit 1
    fi
}

usage() {
    echo "usage: ./submit.sh [debug] <number of jobs> <template> <template parameters>"
    echo
    echo "list all templates: ./submit.sh list"
    echo
    echo "to show the template parameters use:"
    echo "   ./submit.sh show <template>"
}

doJob() {
    jobcommand=$1
    jobmessage=$2

    echo -e "\033[01;32m$jobmessage\033[00m"
    echo -e "\033[00;32mCommand used: \033[00m$jobcommand"
    if [ "$execCMD" == "eval" ]; then
	$execCMD $command
    fi
    echo
}

execCMD=eval
script=$(readlink -f $0)
SCRIPTDIR=$(dirname $script)

echo
case "$1" in
    show)    showParams $2; echo; exit 0
	     ;;
    list)    listTemplates; echo; exit 0
	     ;;
    debug)   execCMD=echo; shift
	     ;;
esac

if [ -z "$1" ]; then
    usage
    exit 0
fi

# set the number of jobs to be submitted
declare -i numberOfJobs=$1
if [ $numberOfJobs -le 0 ]; then
    echo "Wrong number of jobs $numberOfJobs"
    exit 0
fi
shift

# set template name and check if it exists
templateName=$1
shift
checkTemplate $templateName

# template arguments
templateArgs="$@"
templateMainArg=$1

templateDir=$SCRIPTDIR/templates/$templateName
if [ ! -d $templateDir ]; then
    echo "Template '$templateName' does not exist in '$SCRIPTDIR/templates'!"
    exit 1
fi


# define job base directory and create the output directory
starttime=$(date +%F_%H%M)
jobBase=/scratch8/tklemenz/simulation/sim/${templateName}_${starttime}
mkdir -p $jobBase/log
echo "'$jobBase'"
#mkdir /var/tmp/TESTWORKS


#______________________________________________________________________________________________
# submit the job
job1=$(sbatch --time=00:30:00 --mem-per-cpu=4000 --cpus-per-task=1 --array=1-$numberOfJobs -J ${templateName} -o ${jobBase}/log/test.%A_%a.out.log -e ${jobBase}/log/test.%A_%a.err.log $templateDir/run_test.sh  $starttime $jobBase $numberOfJobs $templateArgs)
echo $job1 "- $numberOfJobs job(s) for template '$templateName'"
