#!/bin/bash

mkdir -p staging
cp -r addons staging
cd staging
zip -r voxel-terrain.zip addons
cp voxel-terrain.zip ../
cd ../
rm -r staging
