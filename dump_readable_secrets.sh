#!/bin/sh
rm -rf out && \
make -C dump_readable_secrets clean && \
make -C dump_readable_secrets && \
make -C launcher clean && \
cp dump_readable_secrets/tsec_fw.bin launcher/src/tsec-fw.bin && \
make -C launcher -j4 && \
mkdir out && \
cp launcher/out/tsec_payload.bin out/tsec_payload.bin
