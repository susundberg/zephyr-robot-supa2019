import supalib


SENSOR_SIZE=28/2
TOLE = 0.2
BASE_THICK = 2.0
HEIGHT = 35

sensor = supalib.create_cyl( size_z=HEIGHT, radius=SENSOR_SIZE, place=(0,0,0) )
sensor_m = supalib.create_cyl( size_z=HEIGHT + 10, radius=SENSOR_SIZE - BASE_THICK, place=(0,0,BASE_THICK) )

bolt_m = supalib.create_cyl( size_z = 10, radius=2.0 + TOLE, place=(0,0,-TOLE) )
wire_m = supalib.create_cyl( size_z = 10, radius=1.0 + TOLE, place=(0,2*SENSOR_SIZE/3,-TOLE) )

remove = supalib.create_union( ( bolt_m, sensor_m, wire_m) )

sensor = supalib.create_cut( sensor, remove )


sensor.Label="robot_lawn_sensor"
mesh = supalib.creta_mesh_from( sensor, save_to="/home/pauli/", version=1 )


supalib.finish()

