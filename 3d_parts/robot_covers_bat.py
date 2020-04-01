

import supalib

EPS = 0.01
TOLE = 0.2
BASE_THICK=1.0

BOX_SIZE_X=100
BOX_SIZE_Y=120
BOX_SIZE_Z=50
BAT_SIZE_Z = 100

def create_battery():
    CYL_RAD = 28/2.0
    
    c1 = supalib.create_cyl( ( 10,0,0), CYL_RAD, BAT_SIZE_Z )
    c2 = supalib.create_cyl( (-10,0,0), CYL_RAD, BAT_SIZE_Z )
    b  = supalib.create_box( (20, CYL_RAD*2, BAT_SIZE_Z ), place=( -10,-CYL_RAD,0 )  )
    b2 = supalib.create_box( (10 + TOLE*4, 2 + TOLE*2, BAT_SIZE_Z), place=(-4.0,CYL_RAD-EPS,0) )
    bb = supalib.create_box( (75, 60, 50), place=( 24 - 75,-30,BAT_SIZE_Z))
    bat = supalib.create_union( (c1,c2,b, b2, bb) )
    return supalib.relocate( bat, place=(-10 - CYL_RAD,0,0)  )
    
def create_holding_box():

    b_m   = supalib.create_box( (BOX_SIZE_X,BOX_SIZE_Y,BOX_SIZE_Z*2), place=( 0,0,0 )  )
    b_out = supalib.create_box( (BOX_SIZE_X + BASE_THICK*2,BOX_SIZE_Y + BASE_THICK*2 ,BOX_SIZE_Z + BASE_THICK*2 ), place=( -BASE_THICK,-BASE_THICK,-BASE_THICK )  )
    return supalib.create_cut( b_out, b_m )
    


bat = create_battery()
box = create_holding_box()

bat = supalib.relocate( bat, place=(BOX_SIZE_X - 10, BOX_SIZE_Y/2.0, - BAT_SIZE_Z /3.0 ) )

box = supalib.create_cut( box, bat, name="Bat_box")
supalib.creta_mesh_from( box )
supalib.finish()
