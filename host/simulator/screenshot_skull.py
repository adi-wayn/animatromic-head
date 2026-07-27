import mujoco
import numpy as np
from PIL import Image
import os

xml_path = os.path.join(os.path.dirname(__file__), "skull.xml")
model = mujoco.MjModel.from_xml_path(xml_path)
data = mujoco.MjData(model)

renderer = mujoco.Renderer(model, 400, 400)
mujoco.mj_forward(model, data)

# Manually set camera position and lookat
renderer.update_scene(data)
pixels = renderer.render()
img = Image.fromarray(pixels)
img.save("simulator/skull_screenshot.png")
print("Screenshot saved to simulator/skull_screenshot.png")
