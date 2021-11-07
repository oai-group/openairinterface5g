#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}/../..
source oaienv
cd cmake_targets
sudo ./build_oai -c -w SIMU --eNB