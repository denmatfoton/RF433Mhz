# Tools for Raspberry Pi to record/send ASK RF codes

Nowdays there are a lot of devices remotely controlled thrue 433MHz frecuency. If we decide to do some automation and manage such devices using a single board computer like Raspberry Pi we may figure out a problem, that there are a lot of ASK protocols for this frecuency.
These tools use algorithm, which should recognize the most of protocols.

More about how to connect 433MHz modules to Raspberry Pi and the stucture of the signal you may read [here](http://www.instructables.com/id/Super-Simple-Raspberry-Pi-433MHz-Home-Automation/).

The idea of the algorithm is to record timings between the highs and lows and after some periodes of time analyse the data. Data analyser tries to find sequence of timings, which is similar to some RC signal (sequence of timings with alternative 2 different durations). If signal was detected, it tries to parse it. Then recorded signal may be written to a binary file to be able to reproduce it.
