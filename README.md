# Tools for Raspberry Pi to record/send ASK RF codes

Nowadays there are a lot of devices remotely controlled through 433MHz frequency. If we decide to make some automation and manage such devices using a single board computer like Raspberry Pi we may figure out a problem, that there are a lot of ASK protocols for this frequency.
These tools use algorithm, which should recognize the most of protocols.

More about how to connect 433MHz modules to Raspberry Pi and the stucture of the signal you may read [here](http://www.instructables.com/id/Super-Simple-Raspberry-Pi-433MHz-Home-Automation/).

The idea of the algorithm is to record timings between the highs and lows and after some periodes of time analyze the data. Data analyzer tries to find sequence of timings, which is similar to some RC signal (sequence of timings with alternative 2 different durations). If signal has been detected, it tries to parse it. Then recorded signal may be written to a binary file to be able to reproduce it.
