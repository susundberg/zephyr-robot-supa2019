
import supalib

EPS = 0.001
TOLE = 0.2

THICK = 3
VERSION = 3


def motor_model():
    
    rad_large = (37+2*TOLE)*0.5
    rad_small = (35+2*TOLE)*0.5
    len_both  = 27
    len_output = 30
    bolt_offset = 31/2
    shaft_rad = (12+2*TOLE)*0.5
    bolt_rad  = (4.4)*0.5

    c1 = supalib.create_cyl( radius=rad_large, size_z=len_both, place=(rad_large,0,0) )
    c2 = supalib.create_cyl( radius=rad_small, size_z=len_both, place=(rad_large,0,len_both - EPS) )
    c3 = supalib.create_cyl( radius=shaft_rad, size_z=len_output, place=(11.5, 0, -len_output + EPS ) )
    c4 = supalib.create_cyl( radius=bolt_rad, size_z=len_output, place=(18.4, -bolt_offset, -len_output + EPS ) )
    c5 = supalib.create_cyl( radius=bolt_rad, size_z=len_output, place=(18.4, +bolt_offset, -len_output + EPS ) )
    return supalib.create_union( (c1,c2,c3,c4,c5) )


motor = motor_model()
fp_sx = 37 + 2*THICK
front_plate = supalib.create_box( ( fp_sx, fp_sx, THICK), place=( -THICK, -fp_sx*0.5, -THICK + 0.4 ) )
front_plate = supalib.create_cut( front_plate, motor )

#supalib.finish()

#front_plate.Label="robot_motor_fp"
#mesh = supalib.creta_mesh_from( front_plate, save_to="/home/pauli/", version=VERSION )


supalib.finish()


ms_y = 37 + 2*THICK 
ms_z = 27*2 
main_mount = supalib.create_box( (15, ms_y, ms_z), place=(0, -ms_y*0.5, -THICK + 0.4 - EPS ) )

sides=10
fs_z=ms_z
fs_y = ms_y + 2*sides
low_mount = supalib.create_box( (THICK, fs_y, fs_z), place=(-THICK, -fs_y*0.5, -THICK + 0.4 - EPS ) )

holes = []
hl_y = 0.5*(ms_y + sides)

for loopz in [-1,1]:
    for loopy in [-1,1]:
        c = supalib.create_cyl( radius=2.2, size_z = 4*THICK )
        c = supalib.relocate( c, place=( -2*THICK, loopy*hl_y, loopz*20 + 25 ), rotate=(0,1,0,90) )
        holes.append(c)
holes = supalib.create_union( holes )
low_mount = supalib.create_cut( low_mount, holes )
mounts = supalib.create_union( ( main_mount, low_mount ) )
mounts = supalib.create_cut( mounts, motor )

mount = supalib.create_union( (mounts, front_plate ) )
mount.Label="robot_motor_smallv2_mount"
mesh = supalib.creta_mesh_from( mount, save_to="/home/pauli/", version=VERSION )

supalib.finish()
