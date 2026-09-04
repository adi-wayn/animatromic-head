"""Module for split_face.py."""

import numpy as np
import open3d as o3d

print("Loading face model...")
mesh = o3d.io.read_triangle_mesh("canonical_face_model.obj")
mesh.compute_vertex_normals()

# Get vertices
vertices = np.asarray(mesh.vertices)
triangles = np.asarray(mesh.triangles)

# Find the bounding box
min_bound = vertices.min(axis=0)
max_bound = vertices.max(axis=0)
print(f"Bounds: min={min_bound}, max={max_bound}")

# Let's split based on Y coordinate (vertical).
# Assuming Y is up, if not we'll check bounds.
y_min, y_max = min_bound[1], max_bound[1]
split_y = y_min + (y_max - y_min) * 0.35  # Bottom 35% is jaw

# Find which triangles belong to the upper face and which to the jaw
upper_triangles = []
jaw_triangles = []

for tri in triangles:
    # Get the Y coordinates of the 3 vertices
    y_coords = vertices[tri, 1]
    # If average Y is below split_y, it's jaw
    if np.mean(y_coords) < split_y:
        jaw_triangles.append(tri)
    else:
        upper_triangles.append(tri)

# Create upper face mesh
upper_mesh = o3d.geometry.TriangleMesh()
upper_mesh.vertices = o3d.utility.Vector3dVector(vertices)
upper_mesh.triangles = o3d.utility.Vector3iVector(upper_triangles)
upper_mesh.remove_unreferenced_vertices()
o3d.io.write_triangle_mesh("beautiful_upper_face.obj", upper_mesh)

# Create jaw mesh
jaw_mesh = o3d.geometry.TriangleMesh()
jaw_mesh.vertices = o3d.utility.Vector3dVector(vertices)
jaw_mesh.triangles = o3d.utility.Vector3iVector(jaw_triangles)
jaw_mesh.remove_unreferenced_vertices()
o3d.io.write_triangle_mesh("beautiful_lower_jaw.obj", jaw_mesh)

print("Split completed successfully!")
