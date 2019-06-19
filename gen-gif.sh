#!/bin/bash
ffmpeg -framerate 40 -i "%05d.ppm" -vf "scale=400:400,vflip" clip.gif
