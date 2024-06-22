# falcon-tools

A toolbox for researching and hacking NVIDIA Falcon microprocessors used in TSEC engines on the Tegra X1.

The generic goal is to provide a collection of tools, exploits and code for demystifying the Falcon and
its cryptographic functionality to ease up research for people interested in the cryptosystem and in
reversing Nintendo's TSEC firmwares in Package1 and nvservices.

## Components

* [requiem](./requiem): A template for writing fake-signed Falcon microcode that runs a payload in
Heavy Secure mode; Useful for research and reversing

* [dump_readable_secrets](./dump_readable_secrets): Example for dumping acl 0x03 secrets, using requiem as a base

* [launcher](./launcher) gpvl2 launcher borrowed from here: https://gitlab.com/Nxyoom/tsec-exploration/-/tree/main/launcher 
credits: @EliseZeroTwo

* [libfaucon](./libfaucon): A standard library for Falcon firmware development; Features implementations
of commonly used functions and definitions for MMIO registers

* [payloads](./payloads): A placeholder directory for Falcon firmware blobs which are exploited through
other components in this repository

* [tools](./tools): Helper scripts for working with TSEC firmware blobs

## Usage

With the components out of the way, the order for using these ROP chains on hardware is as following:

Prerequisites: Install Python 3.6+ on your machine and get the `PyCryptodome`, `PyCryptodomex` packages via `pip`. Additionally,
you will need [envytools](https://github.com/envytools/envytools), `make`, `m4` on your system.

1. Clone this repository and set up an environment for controlling a TSEC engine, e.g. through RCM payloads
on the Nintendo Switch.

2. run dump_readable_secrets.sh with PyCryptodome, PyCryptodome in your pip enviroment or venv.

3. launch the output tsec_payload.bin on an erista console, and obtain acl 0x03 (Insecure Readable) or acl 0x13 secrets (csigenc)

4. Refer to [this writeup](./requiem/README.md) to learn about fake-signing.

* Reverse engineering the behavior of certain crypto commands

* Dumping all the ACL 0x13 csecrets to SOR1 HDCP registers where they can be read out

## Credits

The exploits and tools collected in this repository were developed by [**Thog**](https://github.com/Thog)
and [**vbe0201**](https://github.com/vbe0201).

We credit the following people for their great contributions to this project:

* [Elise](https://github.com/EliseZeroTwo) for help and advise in the early stages

* [SciresM](https://github.com/SciresM) and [hexkyz](https://github.com/hexkyz) for being very helpful and
informative throughout our research

## Licensing

This software is licensed under the terms of the GNU GPLv2.

See the [LICENSE](./LICENSE) file for details.
