## teensy-asteroid-apocalypse
assessment project for cab230 microprocessors & digital systems - 2019 sem 1
to design, implement, and test, a simple interactive game called Asteroid Apocalypse which will run on the QUT TeensyPewPew device.

below is copied from task sheet.
#### Task introduction
In this game you control a starfighter which must defend the Earth from a catastrophic asteroid swarm. Asteroids enter the playfield at the top of the LCD display, and travel generally downwards. The starfighter is equipped with a potent plasma cannon mounted in a turret in its nose. When fired, this emits a coherent packet of highly energetic plasma (a plasma bolt) which travels in straight line from the launch point. The plasma cannon covers a 120-degree forward arc (i.e. in front of the starfighter), with default firing direction straight ahead. You can fire plasma bolts by aiming the turret at any point within the forward arc. When a plasma bolt hits an asteroid, the asteroid splits to form two boulders. Boulders continue to move downwards, at a new (different) oblique angle. When a plasma bolt strikes a boulder, the boulder splits to form two fragments, which in turn continue to move downwards at new oblique angles. The starfighter is protected by a deflector shield which extends horizontally across the display just in front of it. When the deflector shield is touched by an object (asteroid, boulder, or fragment), the object is immediately destroyed but the shield takes damage. Eventually, cumulative damage weakens the shield and the starfighter is destroyed by intense ionising radiation released as the shield collapses.

#### Functional Specification
Controls: The game will have controls on the Teensy and controls on the computer.
- Teensy Controls:
    - Joystick left: move spaceship left
    - Joystick right: move spaceship right
    - Joystick up: fire plasma bolts
    - Joystick down: send and display game status
    - Joystick centre: pause game
    - Left button: start/restart game
    - Right button: quit
    - Left potentiometer: set the aim of the turret
    - Right potentiometer: set the speed of the game
- Keyboard controls:
    - 'a' – move spaceship left
    - 'd' – move spaceship right
    - 'w' – fire plasma bolts
    - 's' – send and display game status
    - 'r' – start/reset game
    - 'p' – pause game
    - 'q' – quit
    - 't' – set aim of the turret
    - 'm' – set the speed of the game
    - 'l' – set the remaining useful life of the deflector shield
    - 'g' – set the score
    - '?' – print controls to computer screen (Putty)
    - 'h' – move spaceship to coordinate
    - 'j' – place asteroid at coordinate
    - 'k' – place boulder at coordinate
    - 'l''i' – place fragment at coordinate

