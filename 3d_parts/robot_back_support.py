import supalib
import importlib

importlib.reload( supalib )
supalib.init()

EPS = 0.01
TOLE = 0.3


BASE_THICK=4
PAD_SIZE=40
PAD_HEIG = 12 
FULL_LEN = 58
SUPPORT_LEN = FULL_LEN - PAD_HEIG - BASE_THICK
SUPPORT_SIZE = BASE_THICK*3
SUPPORT_INTER = 4
MOUNT_SIZE=40

def create_pad():
    
    PAD_LARGE_RADIUS=PAD_SIZE*0.75
    INTER  = SUPPORT_INTER
    
    box = supalib.create_cyl( size_z=PAD_HEIG, radius=PAD_SIZE/2, place=(0,0,0) )
    sp      = supalib.create_sphere( radius=PAD_LARGE_RADIUS, place=(0,0,+PAD_LARGE_RADIUS) )
    pad = supalib.create_intersection( (box, sp) )
    
    supalib.relocate( pad, (0,0,-PAD_HEIG) )
    sup = supalib.create_cyl( size_z = INTER , radius=SUPPORT_SIZE/2.0 + TOLE , place=(0,0, -INTER + EPS)  )
    return supalib.create_cut( pad, sup, name="Pad" )


def creat_support():
    sup = supalib.create_cyl( size_z=SUPPORT_LEN + SUPPORT_INTER + EPS, radius=SUPPORT_SIZE/2.0 , place=(0,0,-SUPPORT_INTER)  )
    sup_mount = supalib.create_cyl( size_z = BASE_THICK , radius=MOUNT_SIZE/2.0, place=(0,0,SUPPORT_LEN) )
    sup_union = supalib.create_union( (sup, sup_mount), name="Supp_base" )
    sup_cham = supalib.create_chamfer( sup_union, radius=3.0, edges=( sup_union.Shape.Edges[0], ) )
    cyls = []
    HOLE_X_SIZE = (MOUNT_SIZE/2.0) - 5.0
    yloop = 0
    for xloop in [-1,+1]:
        cyl = supalib.create_cyl( place=(xloop*HOLE_X_SIZE, yloop*HOLE_X_SIZE, SUPPORT_LEN - 0.5  ), radius = 1.5 + TOLE , size_z = BASE_THICK + 1.0 )
        cyls.append(cyl)
    xloop = 0 
    for yloop in [-1,+1]:
        cyl = supalib.create_cyl( place=(xloop*HOLE_X_SIZE, yloop*HOLE_X_SIZE, SUPPORT_LEN - 0.5  ), radius = 1.5 + TOLE , size_z = BASE_THICK + 1.0 )
        cyls.append(cyl)
    cyls = supalib.create_union( cyls )
    return supalib.create_cut( sup_cham, cyls, name="Supp" ) 
    
    
                         
pad = create_pad()
sup = creat_support()


for x in [pad, sup ]:
   supalib.creta_mesh_from( x )
   
App.ActiveDocument.recompute()
Gui.SendMsgToActiveView("ViewFit")
