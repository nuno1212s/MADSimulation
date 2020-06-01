# MADSimulation
A simulation made for Decision supporting Methods (https://www.dcc.fc.up.pt/~jpp/mad/trabalho-3.pdf). The objective is to simulate the costs of running a delivery service where people can pickup packages at the store and can become couriers for a certain fee. 

This software uses boost and ctpl (https://github.com/vit-vit/ctpl) as dependencies.

Compile with CMake version 3.16+.

The program will then ask to input the amount of observations, days in each observation and the Confidence level of the results obtained.

It will then ask you to choose from pre existing compensations and their corresponding probabilities. These can be found on the PDF of this assignment.

You can also input custom ones.

This program is parallelized and will run on as many cores as your computer has, to allow for larger amounts of observations without too long of a waiting period.
