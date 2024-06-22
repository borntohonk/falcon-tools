#!/bin/sh
rm -rf out && \
make -C requiem clean && \
make -C requiem && \
make -C launcher clean && \
cp requiem/tsec_fw.bin launcher/src/tsec-fw.bin && \
make -C launcher -j4 && \
mkdir out && \
cp launcher/out/tsec_payload.bin out/tsec_payload.bin