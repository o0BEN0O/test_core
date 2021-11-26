#!/bin/bash

echo "number:$#"
echo "scname:$0"
echo "first :$1"
echo "second:$2"
echo "argume:$@"

while getopts ':b:d:' OPT &> /dev/null;do
        case $OPT in
        b)
                echo "The options is b"
                echo $OPTARG ;;
        d)
                echo "The options is d"
                echo $OPTARG ;;
        *)
                echo "Wrong Options"
                exit 7 ;;
        esac
#       echo $OPT
#       echo $OPTARG
done
echo $OPTIND
echo $#
shift $[$OPTIND-1]
echo $#
