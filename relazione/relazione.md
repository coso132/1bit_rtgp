# Relazione progetto real-time graphics programming
Franco Antonini

Il progetto consiste nel ricreare lo stile grafico (non photorealistic rendering) del videogioco "Return of the obra dinn". Lo stile del gioco consists in a 1 bit old schoo mac game rendering, aka the only output pixel colors are black and white. the game utilizes a vast number of techniques and methods to enable a pleasant aesthetic while utilizing a classic first person (fps like) camera.

out of the various graphical techniques in the game i prioritized the most impactful rendering and shader techniques, which i will now describe in a feature list:

- low resolution pixelated look -- the games render resolution is locked at 640x360, which is then nearearly upscaled to the screens resolution
- dithering instead of greyscale -- for the different levels of brightness in a "black and white" look, the game will use dithering to emulate the different brightness values of whatever pixel in the screen
    - this includes multiple dithering techniques, in order to create contrast between the different techniques and derivign gameplay meaning from then 
    - this includes the implementation of a novel dithering technique unique to this games development, aka the use of a "dither sphere" to stabilize the dither to camera rotation.
- stylized object outlines -- to accentuate their position int he world, make objects and their surroundings more visible to the player, and also to emulate the old mac games this games style was inspired by
- dust and dust clouds -- as a clever optimized particle effect, the game utilizes a certain "dust cloud" effect for aestheetic reasons and to place vision blocking fog throughout the map.


i will give a brief description of the rendering pipeline i implemented and then go further in detail for each render pass and their implementation in the code, the techniques algorithm and wtv else used.

the rendering pipeline is like this:
1. LIGHTING RENDER PASS -- here textures, directional lighting (with shadows), point lights, and dithering gets processed. the texture value is inserted in the yellow channle and lighting value in the green channel, then each channel gets dithered according to the relevant dithering method.
2. EDGE ACCENTUATION -- here every face of every polygon gets assigned a random color based on the objects id, the face normal, and its position in the world. this approximately guarantees a strong contrast between each face that will facilitate edge detection
2a. EDGE ACCENTUATION 2 -- here the wireframe (if enabled), normal textures (used for manual outline creation) and the dust clouds is rendered. the dust clouds are rendered with a full blue color so it will be properly treated in further rendering steps.
3. OUTLINE CREATION -- the first of the post processing steps, this operates on the edge accentuation render passes and outputs a white on black outline renderable models. this uses a simple kernel.
3a. THIN OUTLINE CREATION -- if enabled, this uses a differnt algorithm that includes depth checks in order to create more accurate an pixel thin outlines (like in obrad inn)
4. COMBINATION PASS -- here the results from the outline and lighting render pass get properly combined in order to pass on the final step 
5. UPSCALE -- the image gets upscaled using nearest neighbor and gets output to the main buffer