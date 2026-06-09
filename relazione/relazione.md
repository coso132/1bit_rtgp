consegna
A detaile document must be produced
project details and choices
algorithms and techniques
implementation details
performance evluatioh (fps, etc)
struttura

- introduzione
spiega lo scopo del progetto e lista la pipeline di rendering, le fasi di rendering, la struttura del codice, dove é stato sviluppato, la macchina utilizzata.

- algorithms and techniques (che in realtá sará un overview di ogni step di rendering)
per ogni fase di rendering, spiega brevemente il codice, la shder usata, lo scopo e funzionamento, riferimenti a return of the obra dinn

-outro
mostra le performance, compara con return of the obra dinn.



# Relazione progetto real-time graphics programming
Franco Antonini

## Introduction
The project consists in a partial recreation of the rendering style of the 2018 video game "Return of the Obra Dinn", using C++/OpenGL. RotOD is a first-person mystery puzzle game where you play an insurance investigator in 1807, determining the fate of each crew member aboard a derelict merchant ship. Its "1-bit" aesthetic uses stark black-and-white pixels with dithered shading, creating a high-contrast, hand-drawn look reminiscent of early computer displays. The style is directly inspired by 1980s Macintosh games (e.g., Dark Castle or The Fool’s Errand), which used monochrome graphics and point‑and‑click interfaces due to hardware limitations.
//TODO migliora
The game uses this 1-bit aesthetic with various adjustments, techniques, and methods for a more pleasant experience i a classic first person (fps like) camera. 

Out of the various graphical techniques showcased in the game i prioritized what i deem as the most impactful towards the final look and feel of the game. Out of the various (simple or difficult alike) rendering and shader techniques, i implemented:

- low resolution pixelated look -- the games render resolution is locked at 640x360, which is then nearearly upscaled to the screens resolution
- dithering for pixel brightness approximation -- for the different levels of brightness in a "black and white" look, the game will use dithering to emulate the different brightness values of whatever pixel in the screen
    - this includes multiple dithering techniques, in order to create contrast between the different techniques and derivign gameplay meaning from then 
    - this includes the implementation of a novel dithering technique unique to this games development, aka the use of a "dither sphere" to stabilize the dither to camera rotation.
- stylized object outlines -- Legibility is the main challenge for a style like this, so the actual geometry needs to be pretty simple. In RotOD better legibility is achieved through stylized traced edges. This accentuates object's and faces' position in the world, making objects and their surroundings more visible to the player while emulating the old mac games this games style was inspired by. 
- dust and dust clouds -- as a clever optimized particle effect, the game utilizes a certain "dust cloud" effect for aestheetic reasons and to place vision blocking fog throughout the map.
- dynamic lighting with directional light shadows and point lights


These effects are achieved through a multi-stage deferred-like pipeline that separates lighting, edge detection, dithering, and culminates in a low-resolution, nearest-neighbor upscaled output.
I will give a brief description of the rendering pipeline i implemented and then go further in detail for each render pass and their implementation in the code, the techniques algorithm and wtv else used.
the rendering pipeline is like this:
1. LIGHTING RENDER PASS -- here textures, directional lighting (with shadows), point lights, and dithering gets processed. the texture value is inserted in the yellow channle and lighting value in the green channel, then each channel gets dithered according to the relevant dithering method.
2. EDGE ACCENTUATION -- here every face of every polygon gets assigned a random color based on the objects id, the face normal, and its position in the world. this approximately guarantees a strong contrast between each face that will facilitate edge detection
2a. EDGE ACCENTUATION 2 -- here the wireframe (if enabled), normal textures (used for manual outline creation) and the dust clouds is rendered. the dust clouds are rendered with a full blue color so it will be properly treated in further rendering steps.
3. OUTLINE CREATION -- the first of the post processing steps, this operates on the edge accentuation render passes and outputs a white on black outline renderable models. this uses a simple kernel.
3a. THIN OUTLINE CREATION -- if enabled, this uses a differnt algorithm that includes depth checks in order to create more accurate an pixel thin outlines (like in obrad inn)
4. COMBINATION PASS -- here the results from the outline and lighting render pass get properly combined in order to pass on the final step 
5. UPSCALE -- the image gets upscaled using nearest neighbor and gets output to the main buffer

## Project details and technical choices

The project uses the core profile of openGL 4.1, with the libraries GLFW, GLAD, FLM, Assimp, and stb_image.
The entire pipeline is based on chained FBOs, this allows for multi-pass rendering with immediately debuggable and visible intermediate results. This also allows for quick standardized post-processing methods without affecting the main scene.
The pipeline is thus not pure deferred, but a multi-pass forward renderer. This gives simplicity for quickly implementing and experimenting diverse shaders and render passes without managing a complex buffer. 

## Algorithms and Techniques

### Stylized Lighting and Dithering
True 1-bit (black/white) rendering loses all shading information //TODO SHOW IMAGE//, making shapes unreadable. The solution is to use dithering to simulate intermediate grays. Two different dithering types (with a sphere variant for each) have been implemented:
- Ordered Dithering (Bayer): using a 16x16 bayer matrix //TODO SHOW IMAGE AND EXPLAIN BETTRE// we can compare luminance values and create a regular, cross-hatch dither pattern. In the game this is used for //TODO LOOK IT UP//. This also means that objects can have differing dither patterns.
- Blue Noise Dithering: this creates a more "random" and organic film-grain look. In this project i use a precomputed 64x64 blue noise image. In the game this is used for //TODO Look it up//

### The "Dither Sphere"
(this is better explained and told here: //TODO INSERT LINK//)
Standard dithering is applied in screen space. When the camera rotates, the dither pattern rotates with it, creating a "swimming" artifact which is too unnatural. RotOD's developer implements a novel method to stabilize the dither to camera rotation, using a "dither sphere".

I implement this "dither sphere" by rendering an icosphere from the inside with the dither texture mapped via UVs. This sphere is rendered from the camera's position without translation, rotating with the view. The resulting image is a view-aligned dither pattern

### Edge Accentuation and Outline Creation


### "Dust" Particles 

## Implementation details


### Key Code Snippets

## Performance Evaluation

### Methodology (hw, scene(s), base frame time)

### Possible bottlenecks ( shadow map, dither sphere)

### Resolution scaling test

## Conclusion

### Discuss Edge detection problems