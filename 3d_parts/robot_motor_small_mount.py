
import supalib

EPS = 0.01
TOLE = 0.1
BASE_PLATE_INNER_XSIZE = 33 + 2*8
BASE_PLATE_INNER_YSIZE = 26
BASE_PLATE_INNER_ZSIZE = 32.0
BASE_PLATE_THICK = 5
NORMAL_THICK = 3




main_plate_side_xy  = supalib.create_box( size=(BASE_PLATE_INNER_XSIZE + EPS, BASE_PLATE_INNER_YSIZE + EPS, BASE_PLATE_THICK ) , place=(0,0,0)  )
main_plate_side_xz  = supalib.create_box( size=(BASE_PLATE_INNER_XSIZE + EPS, NORMAL_THICK, BASE_PLATE_INNER_ZSIZE + BASE_PLATE_THICK), place=(0, BASE_PLATE_INNER_YSIZE, 0) )
main_plate_side_yz  = supalib.create_box( size=(NORMAL_THICK, BASE_PLATE_INNER_YSIZE + NORMAL_THICK, BASE_PLATE_INNER_ZSIZE + BASE_PLATE_THICK), place=(BASE_PLATE_INNER_XSIZE, 0 , 0) )

cyls = []
HOLE_Z_SIZE = 9
HOLE_X_SIZE = 33/2.0
HOLE_X_OFFSET = BASE_PLATE_INNER_XSIZE*0.5 - 1.0 # The -1 is for tolerance
HOLE_Z_OFFSET = BASE_PLATE_THICK + (BASE_PLATE_INNER_ZSIZE )*0.5
for xloop in [-1,+1]:
    for yloop in [-1,+1]:
       cyl = supalib.create_cyl( place=(HOLE_X_OFFSET + xloop*HOLE_X_SIZE,BASE_PLATE_INNER_YSIZE - 1.0 ,yloop*HOLE_Z_SIZE + HOLE_Z_OFFSET ), rotate=(1,0,0,-90), radius = 1.5 + TOLE/2.0 , size_z = 20.0 )
       cyls.append(cyl)
cyls.append( supalib.create_cyl( place=(HOLE_X_OFFSET + HOLE_X_SIZE - 9.0 ,BASE_PLATE_INNER_YSIZE - 1.0 , HOLE_Z_OFFSET ), rotate=(1,0,0,-90), radius = 3.5, size_z = 20.0 ) )

    
cyls = supalib.create_union( cyls )
main_plate_side_xz = supalib.create_cut( main_plate_side_xz, cyls ) 

MOUNT_BOLT_RAD = 4
MOUNT_BOLT_DEPT = 3

MOUNT_Z_OFFSET = BASE_PLATE_THICK + EPS
MOUNT_X_OFFSET = BASE_PLATE_INNER_XSIZE*0.5
MOUNT_Y_OFFSET = BASE_PLATE_INNER_YSIZE*0.5
MOUNT_X_SIZE   = BASE_PLATE_INNER_XSIZE*0.5 - MOUNT_BOLT_RAD - 2.0
MOUNT_Y_SIZE   = BASE_PLATE_INNER_YSIZE*0.5 - MOUNT_BOLT_RAD - 2.0
bolts = []
for xloop in [-1,+1]:
    for yloop in [-1,+1]:
        bolt = supalib.create_bolt( radius_large=MOUNT_BOLT_RAD, radius_small=2.0, size_z_large=MOUNT_BOLT_DEPT, size_z_small=4, rotate=(1,0,0,180), place=( MOUNT_X_OFFSET + xloop*MOUNT_X_SIZE,  MOUNT_Y_OFFSET + yloop*MOUNT_Y_SIZE, MOUNT_Z_OFFSET ) )
        bolts.append( bolt )
                                
bolts = supalib.create_union( bolts )
main_plate_side_xy = supalib.create_cut( main_plate_side_xy, bolts) 


part = supalib.create_union( [ main_plate_side_xy, main_plate_side_xz, main_plate_side_yz ] )




