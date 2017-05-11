#!/bin/sh

(
cd $(dirname ${BASH_SOURCE[0]})
eval $(scram ru -sh)
pushd ${CMSSW_BASE}/src/ZZMatrixElement/MELA/
./setup.sh "$@" || exit 1
popd
scramv1 b "$@"
)
