import mujoco
import mujoco.viewer
import time
import socket
import json
import threading
import sys
import os

UDP_IP = "127.0.0.1"
UDP_PORT = 4210

target_jaw_angle = 0.0

def udp_listener():
    global target_jaw_angle
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.bind((UDP_IP, UDP_PORT))
        print(f"[Simulator] Listening for UDP intents on {UDP_IP}:{UDP_PORT}")
    except OSError as e:
        print(f"[Simulator] Error binding UDP port: {e}")
        return

    while True:
        try:
            data, addr = sock.recvfrom(1024)
            msg = json.loads(data.decode('utf-8'))
            
            if msg.get('type') == 'INTENT':
                payload = msg.get('payload', {})
                emotion = payload.get('emotion_primary')
                intensity = payload.get('intensity_level', 0.0)
                
                if emotion == "JAW":
                    # intensity is usually between 0.0 and 1.0. 
                    target_jaw_angle = intensity * 0.5 
        except Exception as e:
            pass

def control_callback(model, data):
    global target_jaw_angle
    try:
        # Exponential decay like the real hardware to prevent jaw getting stuck open
        if target_jaw_angle > 0.01:
            target_jaw_angle *= 0.8
        else:
            target_jaw_angle = 0.0
            
        motor_id = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_ACTUATOR, "jaw_motor")
        if motor_id != -1:
            data.ctrl[motor_id] = target_jaw_angle
    except Exception:
        pass

def main():
    print("Loading MuJoCo...")
    xml_path = os.path.join(os.path.dirname(__file__), "skull.xml")
    
    try:
        model = mujoco.MjModel.from_xml_path(xml_path)
        data = mujoco.MjData(model)
    except Exception as e:
        print(f"Failed to load MuJoCo XML: {e}")
        sys.exit(1)

    # Start UDP thread
    listener_thread = threading.Thread(target=udp_listener, daemon=True)
    listener_thread.start()

    # Set the global control callback
    mujoco.set_mjcb_control(control_callback)

    print("Simulator running. Close the window to exit.")
    
    # Run the viewer blocking (works natively on macOS)
    mujoco.viewer.launch(model, data)

if __name__ == '__main__':
    main()
