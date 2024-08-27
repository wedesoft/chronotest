## Chronotest

Simple examples to test the capabilities of Project Chrono (built under GNU/Linux).

### Dependencies

You need to install the core of [Project Chrono][1] version 9.0.1 which requires Eigen as well.
See [here][2] for installation instructions.

Further, you need to install [GLFW][3] and [GLEW][4] for visualisation.

### Build

```Shell
make
```

### Run
### Tumbling cuboid in space

[![Tumbling cuboid in space](https://i.ytimg.com/vi/sOVNxBt_VFk/hqdefault.jpg)](https://www.youtube.com/watch?v=sOVNxBt_VFk)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./tumble
```

### Orbiting point mass

[![Orbiting mass](https://i.ytimg.com/vi/d8NJRU075uM/hqdefault.jpg)](https://www.youtube.com/watch?v=d8NJRU075uM)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./orbit
```

### Falling stack of cuboids

[![Falling stack](https://i.ytimg.com/vi/ogPX6ZcIZ94/hqdefault.jpg)](https://www.youtube.com/watch?v=ogPX6ZcIZ94)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./stack
```

### Double pendulum

[![Double pendulum](https://i.ytimg.com/vi/wFeajyhTXfI/hqdefault.jpg)](https://www.youtube.com/watch?v=wFeajyhTXfI)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./pendulum
```

### Spring-damper system with prismatic joint

[![Spring-damper system](https://i.ytimg.com/vi/ZBWpwDY6iwA/hqdefault.jpg)](https://www.youtube.com/watch?v=ZBWpwDY6iwA)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./suspension
```

### Wheel touching the ground with speed

[![Spring-damper system](https://i.ytimg.com/vi/47Z3ELcNVW4/hqdefault.jpg)](https://www.youtube.com/watch?v=47Z3ELcNVW4)

```Shell
export LD_LIBRARY_PATH=/usr/local/lib
./wheel
```

### See also

* [Chrono tutorial (PDF)][5]

[1]: https://projectchrono.org/
[2]: https://api.projectchrono.org/development/tutorial_install_chrono.html
[3]: https://www.glfw.org/
[4]: https://glew.sourceforge.net/
[5]: https://www.projectchrono.org/tasora/download/lecture_chrono_tutorial.pdf
