# 412AirQualityCapstone
2023 Portland State University ECE 412 Air Quality Capstone Project
<br>
<p>
We spend most of our lives indoors, but most of us lack the tools to assess the quality of the air
we breathe in those environments. Personal tools are available, albeit expensive, and must be
inconveniently carried with us along with all the other tools of our daily lives. Instead, we want to
make it easy to instrument buildings, continuously monitor the indoor environment, and report
data back to us. We can make it easy by making cheap, reliable devices that don't need
recharging for a long time. Three features of indoor air are expected to be monitored: 2.5 μm
particle count or PM2.5, carbon dioxide (CO2) concentration, and ventilation air speed.</p>
<br>
<p>
Regarding CO2, "Statistically significant decrements occurred in cognitive performance
(decision making, problem resolution) starting at 1000 ppm (Satish et al., 2012)" [1]. It is also a
good proxy for ventilation; high CO2 levels mean the room is poorly ventilated, which increases
the risk of passing airborne diseases such as COVID. After I finish Zoom meeting in a closed
room at home, the CO2 level is often over 1200!</p>
<br>
<p>
Regarding PM2.5, the WHO recommends an upper limit of 5 ug/m3 (microgram per cubic
meter) average annually and 15 ug/m3 average over a 24 hour period [2]. As this project
document is being written, the outdoor air in Portland is over 30 ug/m3!</p>
<br>
<p>
An air speed sensor, or anemometer, can help us calculate how much air is flowing into or out of
a room and help understand why CO2 is high. The system should report an invalid value (e.g.,
not zero because zero airflow is a valid value) if the anemometer is not connected.</p>
<br>
<p>The system will be based on components past capstone teams have successfully incorporated
such as the TI MSP430 (including super low-power sleep modes). In this iteration, we will aim to
replace the closed-source proprietary SmartMesh IP wireless system with the OpenWSN
open-source wireless networking system.</p>
<br>
<h3>Project Goals</h3>
<br>
<p>Produce a fully wireless sensor data logger incorporating as many open-source components as
possible and able to be produced at a very low cost. A particular goal of this project is very low
power performance: sensors, communication system, and the microcontroller should be in as
low a power state as possible when not in use such that batteries need to be changed, at most,
annually. (The lifetime of a smoke detector would be even better!)</p>
<br>
<p>Data should be temporarily logged locally and wirelessly communicated to base station(s) as
appropriate. A stretch goal includes visualization dashboard to present the logged data
graphically for at-a-glance use. The device should be mechanically integrated into as small a
form-factor as is practical and contained in a robust housing appropriate for indoor
environments.</p>
