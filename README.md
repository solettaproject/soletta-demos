# Soletta Demos

Repository of samples for Soletta Project

## Building

Some samples depend on custom node types, so it's required to build
and install them in a path that can be found by sol-fbp-runner

To do so, execute the following commands in a terminal:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make install

## Foosball

This demo is composed by two parts: foosball table, where goals are detected
and provides a small interface with players, and a score counter.

These parts talk to each other using OIC protocol.

So, to properly work, both foosball/table/main.fbp and
foosball/scoreboard/main.fbp must be running.

### Testing Foosball on development machine

In order to test it in a development machine, open two terminals to
simulate a table and a scoreboard.

On table terminal run:

    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=foosball/table/conf-gtk.json
    $ sol-fbp-runner foosball/table/main.fbp

On scoreboad terminal run:

    $ export SOL_MACHINE_ID="666a3d6a9d194a23b90a24573558d2f4"
    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=foosball/scoreboard/sol-flow.json
    $ sol-fbp-runner foosball/scoreboard/main.fbp
