# Simple packet sniffer.

   * sniffd.c     - daemon program that gathers IP packets data.
   * cli.c        - command line interface to daemon.

### Build:

    make sniffd  - build sniffer daemon.
    make cli     - build cli to daemon.
    make all     - build sniffd & cli.


### Run:

    sudo ./sniffd <interface>  - start daemon independently.
    sudo ./cli                 - start cli.
