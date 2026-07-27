import open3d as o3d
import os

base_dir = "../../docs/Animatronic Skull - 2456550/files/"
left_path = os.path.join(base_dir, "LeftSkull.stl")
right_path = os.path.join(base_dir, "RightSkull.stl")
face_path = os.path.join(base_dir, "face.stl")
jaw_path = os.path.join(base_dir, "jaw.stl")
teeth_path = os.path.join(base_dir, "teeth.stl")

# Load meshes
print("Loading meshes...")
left = o3d.io.read_triangle_mesh(left_path)
right = o3d.io.read_triangle_mesh(right_path)
face = o3d.io.read_triangle_mesh(face_path)

# Merge upper skull
print("Merging upper skull...")
upper_skull = left + right + face
# Decimate to avoid MuJoCo's 200k limit (though these 3 are small, just to be safe)
upper_skull = upper_skull.simplify_quadric_decimation(100000)

print(f"Upper skull has {len(upper_skull.triangles)} triangles")
o3d.io.write_triangle_mesh("upper_skull.obj", upper_skull)

# Let's also load and simplify the jaw, and add teeth to it if we want!
print("Loading jaw and teeth...")
jaw = o3d.io.read_triangle_mesh(jaw_path)
teeth = o3d.io.read_triangle_mesh(teeth_path)
teeth = teeth.simplify_quadric_decimation(20000)
merged_jaw = jaw + teeth
merged_jaw = merged_jaw.simplify_quadric_decimation(50000)

print(f"Merged jaw has {len(merged_jaw.triangles)} triangles")
o3d.io.write_triangle_mesh("merged_jaw.obj", merged_jaw)

print("Done!")
