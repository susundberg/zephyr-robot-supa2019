import supalib


SIZE_Z = 25
BASE_RAD = 81.5
BASE_THICK = 4.5
TEETH_THICK = 5
ADD_TEETH_THICK = 8
EPS = 0.1
ADD_HEIH = 10

def create_bounds():
    box_bound1 = supalib.create_box( (BASE_RAD+BASE_THICK,TEETH_THICK + EPS, SIZE_Z) )
    box_bound2 = supalib.create_box( (BASE_RAD+BASE_THICK, TEETH_THICK + EPS, SIZE_Z), rotate=(0,0,1,360/56.0) )
    cyl_bound  = supalib.create_cyl( radius=BASE_RAD + EPS , size_z = SIZE_Z )
    return supalib.create_union( (box_bound1, box_bound2, cyl_bound ) )



                              
tr = supalib.create_triangle( BASE_THICK + ADD_HEIH, ADD_TEETH_THICK, SIZE_Z )
tr = supalib.relocate( tr,  rotate=(0,0,1,180) )

tr2 = supalib.create_triangle( BASE_THICK + ADD_HEIH, ADD_TEETH_THICK, SIZE_Z/2.0 )
tr3 = supalib.create_triangle( BASE_THICK + ADD_HEIH, ADD_TEETH_THICK, SIZE_Z/2.0 )

tr2 = supalib.relocate( tr2,  rotate=(0,0,1,180), place=(0,0,SIZE_Z/2.0) )
tr3 = supalib.relocate( tr3,  rotate=(0,0,1,180) )
tr3 = supalib.relocate( tr3,  rotate=(1,0,0,180), place=(0,-ADD_TEETH_THICK,SIZE_Z/2.0) )
tr2 = supalib.create_union( (tr2,tr3) )

import math
full_inc_rad  = ( 2*math.pi*86 ) / 56.0
teeth_inc_rad = TEETH_THICK
remaining_inc_rad = ( full_inc_rad - TEETH_THICK ) / ( 2*math.pi*86 )
remaining_inc_rad *= 360
normal_dist = remaining_inc_rad + (360/56.0 - remaining_inc_rad)*0.5

bounds = create_bounds()

for loop,xxx in enumerate([tr,tr2]):
   xxx = supalib.relocate( xxx,  place=(BASE_RAD + BASE_THICK + ADD_HEIH,  0.5*ADD_TEETH_THICK,0) )
   xxx = supalib.relocate( xxx,  rotate=(0,0,1, normal_dist ) )
   item = supalib.create_cut( xxx, bounds )
   item.Label="Wheel_add_%d" % loop
   mesh = supalib.creta_mesh_from( item, save_to="/home/pauli/" )

supalib.finish()
