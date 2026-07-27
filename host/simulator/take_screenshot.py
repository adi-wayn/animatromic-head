import mujoco
import numpy as np
from PIL import Image

angles = [(0,0,0), (1.57,0,0), (0,1.57,0), (0,0,1.57), (-1.57,0,0)]
for i, (roll, pitch, yaw) in enumerate(angles):
    m = mujoco.MjModel.from_xml_path("skull.xml")
    
    # Optional: adjust euler dynamically if needed, but since we are just testing screenshot we can modify the m.body("skull_assembly") directly or just use what's in the XML.
    
    d = mujoco.MjData(m)
    mujoco.mj_step(m, d)
    
    renderer = mujoco.Renderer(m, 480, 640)
    # The xml has a camera named "cam" if we kept it... wait, I didn't add "cam" to skull.xml!
    # I should add the camera to skull.xml or use free camera.
    # Actually, I'll update the script to add a camera if not present, but wait I can't.
    # Let's use free camera and just look at the origin.
    renderer.update_scene(d)
    
    pixels = renderer.render()
    img = Image.fromarray(pixels)
    img.save(f"screenshot_final.png")
    print(f"Saved screenshot_final.png")
    break # only need one screenshot since the orientation is fixed in XML
    
    pixels = renderer.render()
    img = Image.fromarray(pixels)
    img.save(f"screenshot_{i}.png")
    print(f"Saved screenshot_{i}.png for angle {roll} {pitch} {yaw}")
