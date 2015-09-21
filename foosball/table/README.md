== Table ==

When testing it in a development machine, just use gtk conffiles.

    $ export SOL_FLOW_MODULE_RESOLVER_CONFFILE=foosball/table/conf-gtk.json
    $ sol-fbp-runner foosball/table/main.fbp

And a GUI should be displayed with controls.

When using Edison conf-edison.json must be used. To match this configuration,
sensors / actuators must be set on following physical pins:

    * DetectorLightSensor -> Analogic 0
    * DetectorLed -> Digital 0
    * MaxLed5 -> Digital 2
    * MaxLed10 -> Digital 3
    * MaxButton -> Digital 4
    * MainResetButton -> Digital 5

== Scoreboard ==
