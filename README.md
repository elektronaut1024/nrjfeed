Transmitter
===========
x-board v2
The board has an optical sensor and an xbee interface.

sensor
------
lightsensor detects optical signal
and pairs the signal with a counter and a timer

sender
------
sends collected data through the serial-xbee interface


Transceiver
===========
x-board relay
The board is connected to the Ethernet and has an xbee interface.

receiver
--------
receives data through the serial-xbee interface and stores it in memory.

webserver
---------
provides an interface to access data stored in memory.


Transmission Protocol
=====================
The Transmitter transmits summaries of collected datapoints every x seconds.

format
------
<package> = <idx>:<data-length>:<data>
<idx> = [0-9]+
<data-length> = [0-9]+
<data> = <timestamp>(,<data>|)
<timestamp> = [0-9]+

idx: is a counter which allows to track lost packages.
data-length: is an indicator of how many times the sensor spiked in x seconds.
data: a comma separated list of timestamps, where every timestamp indicates an individual spike.


Example #1 (three spikes):
1:3:300,302,309

Example #2 (no spikes):
2:0:

Example #3 (one spike):
3:1:335

Real Example Log:
1:10:12,25,28,29,33,34,34,36,36,67
2:10:73,73,84,86,87,87,92,98,98,100
3:10:100,100,103,103,106,117,118,118,122,137
4:10:137,138,142,142,150,152,155,155,155,158
5:10:159,162,162,167,167,167,167,172,173,174
6:10:176,183,183,185,188,189,191,192,197,197
7:4:199,199,204,205
8:10:314,317,319,321,322,322,323,325,327,329
9:10:331,355,358,358,361,363,363,366,366,366
10:10:377,417,417,417,424,426,428,428,429,429
11:10:430,431,431,431,440,446,446,446,455,459
12:3:463,526,530
13:10:572,582,586,589,590,590,593,595,595,595
14:10:597,600,600,600,602,604,604,604,606,609
15:10:610,612,612,614,616,616,616,616,618,618