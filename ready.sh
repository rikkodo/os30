#!/bin/sh

DIR=$(dirname $0)
ORG=${PWD}
cd $DIR
DIR=${PWD}
export PATH="$DIR/bin:$PATH"
cd ${ORG}

export MY_PREFIX="<OS30> ${MY_PREFIX}"

bash
