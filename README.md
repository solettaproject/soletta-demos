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

    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=foosball/table/sol-flow.json
    $ sol-fbp-runner foosball/table/main.fbp

On scoreboad terminal run:

    $ export SOL_MACHINE_ID="666a3d6a9d194a23b90a24573558d2f4"
    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=foosball/scoreboard/sol-flow.json
    $ sol-fbp-runner foosball/scoreboard/main.fbp

## Factory monitor

This demo exemplifies a factory floor monitor: several terminals
that connect, using OIC, to several monitors. Each monitor outputs its
current temperature reading and a FAILURE state.
Terminals usually show monitor temperatures on a round robin fashion.
However, if any monitor sets its FAILURE state to true, terminals
will stop showing temperatures and will show the failure status.
When on failure state, one can use available buttons to 1) Dismiss
failure status 2) Report to some 'supervisor' - an action that dismisses
the failure state as well. After dismiss, terminals go back to showing
monitors temperatures.

### Testing on development machine

To simplify testing, this demo provides a sol-flow.json that uses GTK
widgets as input/output. It also provides a monitor-dummy.fbp that doesn't
really reads a temperature sensor, but instead show fake increasing
values. To use the monitor-dummy.fbp, search for `##Monitor nodes` on
`factory-monitor/fbp/main.fbp` and uncomment the line

    DECLARE=Monitor:fbp:monitor-dummy.fbp

Then, run `factory-monitor/fbp/main.fbo`:

    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=factory-monitor/fbp/sol-flow.json
    $ sol-fbp-runner factory-monitor/fbp/main.fbp

This will run both terminal and monitor. If desired, it's possible to
execute only the monitor - on different machines or VMs for instance - to
get different instances of monitors.

    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=factory-monitor/fbp/sol-flow.json
    $ sol-fbp-runner factory-monitor/fbp/monitor-dummy.fbp

If doing so, remember to change monitor_name to another name, so you
can differentiate them:

```sh
monitor_name(constant/string:value="Thermometer alpha")
```

If you run different monitors on the same machine, it's necessary to give
a different `SOL_MACHINE_ID` to each:


    $ SOL_MACHINE_ID=2f3089a1dbfb43d38cab64383bdf9380 sol-fbp-runner factory-monitor/fbp/monitor-dummy.fbp

**Note:** Running more than one instance of monitor **will not work** on
Linux. Client will only bind to one of them. Run different monitors on
different machines or virtual machines to workaround this.

### Testing on quark se development board

Use sol-flow-quark-se-devboard.json configuration file - the simplest way
is to rename it to `sol-flow.json` - but be careful to not overwrite GTK
one:

    $ mv factory-monitor/fbp/sol-flow.json factory-monitor/fbp/sol-flow-gtk.json
    $ mv factory-monitor/fbp/sol-flow-quark-se-devboard.json factory-monitor/fbp/sol-flow.json

Then, search for `##monitor nodes` on `factory-monitor/fbp/main.fbp` and
ensure that only the line

    DECLARE=Monitor:fbp:monitor-quark-se-devboard.fbp

is uncommented among the monitor `DECLARE`s.

### Testing on MinnowBoard MAX using 6LoWPAN with a CC2520 radio on Ostro

It's possible to run this demo using 6LoWPAN on a MinnowBoard MAX with
Ostro image installed. A CC2520 radio is required.

#### Wiring CC2520 on MinnowBoard MAX

The following connections must be made. For those using a CC2520EMK, it's
also provided pin numbers to be used.

```
MinnowBoard MAX pin         CC2520              CC2520EMK pin
4 (3v3)                     VDD                 P2.7
2 (GND)                     GND                 P1.19
5 (SPI_CS)                  SPI_CS              P1.14
7 (SPI_MISO)                SPI_MISO            P1.20
9 (SPI_MOSI)                SPI_MOSI            P1.18
11 (SPI_SCK)                SPI_SCK             P1.16
21 (GPIO_338)               FIFO                P1.7
22 (GPIO_504)               SFD                 P2.18
23 (GPIO_339)               FIFOP               P1.9
24 (GPIO_505)               RESET               P2.15
25 (GPIO_340)               CCA                 P1.12
26 (GPIO_464)               VREG                P1.10
```

#### Wiring buttons and led on a Calamari Lure

This sample uses three buttons and a led also. One can wire them directly,
but for simplicity, we used a Calamari lure. Note that we didn't wired
MinnowBoard MAX pins to their counterparts on Calamari. If wiring buttons
and led directly on a protoboard, remember to wire them to GPIO of
MinnowBoard MAX.

```
MinnowBoard MAX pin         Calamari Pin
4 (3v3)                     4 (3v3)
2 (GND)                     2 (GND)
14 (GPIO_472)               22 (LED1)
16 (GPIO_473)               14 (BTN1)
18 (GPIO_475)               10 (BTN2)
20 (GPIO_474)               12 (BTN3)
```

Note that you may want to use a breadboard to easy wiring.

#### Setup 6LoWPAN on Ostro

After wiring and booting MinnowBoard MAX with Ostro image, you'll need
to setup CC2520 radio and 6LoWPAN:

```
insmod /lib/modules/4.1.15-yocto-standard/extra/spi-minnow-cc2520.ko
insmod /lib/modules/4.1.15-yocto-standard/extra/spi-minnow-board.ko
ip link set wpan0 address a0:0:0:0:0:0:0:2
iz set wpan0 777 8002 11
ifconfig wpan0 up
ip link add link wpan0 name lowpan0 type lowpan
ifconfig lowpan0 up
```

If setting up two devices, remember to change address on second:
```
insmod /lib/modules/4.1.15-yocto-standard/extra/spi-minnow-cc2520.ko
insmod /lib/modules/4.1.15-yocto-standard/extra/spi-minnow-board.ko
ip link set wpan0 address a0:0:0:0:0:0:0:3
iz set wpan0 777 8003 11
ifconfig wpan0 up
ip link add link wpan0 name lowpan0 type lowpan
ifconfig lowpan0 up
```

If you create a script to do this setup, you may need to wait between
two commands, so the previous one has completed any system alteration
it does.

#### Running demo

One can run demo by running
```
sol-fbp-runner fbp/main.fbp
```

Remember to use `monitor-dummy.fbp`. Soletta shall use
`sol-flow-intel-minnow-max-linux_gt_3_17.json` configuration file
autmatically - in case it doesn't, rename it to `sol-flow.json` on
MinnowBoard MAX.

Calamari Button3 shall set FAILURE state. Buttons 1 & 2 shall control
demo normal flow.

Note that while showing available 'Temperatures', you may see somethig
like
```
lcd Thermometer B
6.000000 C (string)
lcd Thermometer B
6.000000 C (string)
lcd Thermometer B
8.000000 C (string)
lcd Thermometer A
8.000000 C (string)
lcd Thermometer A
8.000000 C (string)
lcd Thermometer A
6.000000 C (string)
lcd Thermometer B
6.000000 C (string)
lcd Thermometer B
6.000000 C (string)
lcd Thermometer B
8.000000 C (string)
lcd Thermometer A
8.000000 C (string)
lcd Failure on Thermometer A (string)
```

Those three repetitions are normal: only the last one is worth. This
happens because of how `string/concatenate` node type works: it'll
output a new string for each step of concatenation.
If using a real LCD display, this fact should be unnoticed.
