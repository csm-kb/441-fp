Kane Bruce // kbruce@mymail.mines.edu
FP // Stargazer
-------------------------
Hello! This project represents the culmination of much game/engine design research and all that I learned in
the course this semester. It is a from-scratch game engine that uses the SDL2 window framework (instead of glfw
-- I wanted to gain a complete understanding of how the entire process works from program launch to exit),
and OpenGL 4.1 using the Core profile.

The engine would be more separable, but the time crunch of the last couple weeks of the semester led to some
undesired but necessary coupling of game and rendering objects. If I had more time, I would make it so that this
basic game engine could be used standalone, with the Component design pattern being more heavily leveraged. But for
now, the engine is coupled to the second part of my final project, a game called Stargazer!

Stargazer is a little spaceship simulator that allows the user to control a spacecraft with four degrees of freedom
(I sacrificed roll, because I didn't have time) in a Newtonian environment around a small mock-up solar system. A basic
Phys engine allows objects to be affected by forces -- the framework is in place for objects (notably planets)
to be able to collide, emit gravity, and impact the spaceship's trajectory; but it is left unimplemented due to time.

The controls are available upon game boot, via a console window:
//
 ESC : quit the game (likely crash)
  W  : thrust in forward direction
  S  : thrust in reverse direction
  A  : yaw spacecraft left
  D  : yaw spacecraft right
  R  : pitch spacecraft up
  F  : pitch spacecraft down
SPACE: kill velocity to zero
  1  : third-person camera (default)
  2  : 'first-person' camera
\\
The user is able to look around with an arcball-style camera attached to the spacecraft, and also a 'first-person'
camera that is orientation-locked to the spacecraft's heading.

The spaceship is rendered and maintained hierarchically by the game engine. The

Texture applications include the post-processing framebuffer and render target, as well as the cubemapped skybox.

Nontrivial shader applications include the use of a geometry shader for dynamic vertex deformation (based on velocity),
a dynamic state-based post-processing shader, a discrete skybox shader, and a hand-made Blinn-Phong fragment shader.

The post-processing shader is state-based, with three states:
-   'shake', for shaking the screen.
-   'chaos', for applying chaotic edge detect and color inversion.
-   'confuse', for flipping the output vertically & horizontally and color inversion.

Lights include a partially-functioning point light on the spacecraft, as well as a global directional light.

Materials are custom-defined for each planet (> 2), as well as the spacecraft.

The only major bugs include: improper engine shutdown (which tends to result in a crash-for-exit instead of a
regular exit), the 'first-person' camera isn't locked to the nose of the spacecraft, and distant objects don't use LOD
so their triangles will flicker.
-------------------------
COMPILING INSTRUCTIONS:
-------------------------
Windows 10 64-bit (the only tested platform):
- in the CMakeLists, ensure that the CSCI441 common include directory exists and is pointed to.
- ensure that 'glew32.dll','glew32mx.dll', and 'SDL2.dll' exist in the root directory.
- ensure that the running directory is set to the root directory.
- ensure that the local lib folder contains the 'SDL2-2.0.3' development kit folder.
- run a build, and it should compile!

Additional steps for Unix platforms:
- in the CMakeLists, some work may need to be done to get it to correctly link the SDL2 libraries. They *are* there!
- otherwise, it should build in roughly the same manner!
-------------------------
The engine reads in .jpg files for resource loading. The engine can be quickly modified to support other resource types
in the future, including .obj files for complex meshes.
-------------------------
How long did this assignment take you? : ~42 hours
How much did the labs help you for this assignment? : 9
How fun was this assignment? : Everywhere from a 2 to a 10. Some parts had me pulling my hair out, while other parts had
    me in states of euphoric bliss!
    
    
    
/// P.S: there is a small easter egg in the game! DO NOT SCROLL, unless you are ready to give up on finding it!



































































































































































































































































---------------------------------------
-----------/// SPOILERS ///------------
---------------------------------------
Believe iitst or not, the easter egg is actually the ring elf! It may just be a cylinder primitive, but it can be likened to the rings in the Halo series. Ha!

>> another easter egg: check the line number of this exact line! hint: related to above...