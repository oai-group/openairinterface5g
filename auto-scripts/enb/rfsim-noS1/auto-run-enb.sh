#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}/../../..
source oaienv
cd cmake_targets/ran_build/build
sudo RFSIMULATOR=enb ./lte-softmodem -O ${DIR}/enb.10MHz.b200.conf --rfsim --noS1