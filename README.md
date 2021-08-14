# logdraw
A utility for visualizing log files, written in C++ and OpenGL. It is currently on pause as I work on other projects, but I do plan on coming back to it.

![2021-08-13-174856_3200x900_scrot](https://user-images.githubusercontent.com/5828713/129430285-88dbfbea-6b5a-4fb6-9886-35bd312b481f.png)

## The General Idea
Often when writing a program it's useful to be able to write a bunch of information to a file; temperature in a greenhouse, XYZ positions of a robot, frametimes in a video game, and so on. But then this data needs to be visualized in order to be useful. This program is meant to be able to read and render the data in these files in real-time, as those files are being written to.

So, for example, you could start logdraw and open a file. With another program you could record temperature information along with a timestamp in that file. As you do, logdraw would create a line graph showing how the temperature changes over time. You could use this display to build an intuitive understanding of how the temperature is changing.

The primary use case is for videogames and simulation. The behavior of a virtual object can be difficult to determine based entirely on a list of numbers and often the problem can be detected almost instantly when properly visualized. Often the tools in the engine are insufficient (especcially in home-build engines, but frequently in pre-made engines) and the developer doesn't want to spend the time writing their own. They (e.g. I) would rather configure a tool once, then just throw everything into a file and let the tool handle it. As this is *just* a visualizer, it can focus on being a very good visualizer.

## Current State of the Project
The program is a long way from achieving the above. At the moment it can read in a log file and display it in a 3d graph rendered using OpenGL.

## Why is this an HTML project?
Most of the project is glm api documentation.
