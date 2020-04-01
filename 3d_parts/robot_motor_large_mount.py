


EPS = 0.01
TOLE = 0.2




def crete_bottom_plate():
    BASE_WALL_THICK     = 4.0
    BASE_RADIUS         = 51.0/2.0 + BASE_WALL_THICK
    BASE_THR_HOLE_SIZE  = 26.0/2.0 + TOLE
    BASE_MOT_HOLE_SIZE  = 51.0/2.0 + TOLE
    BASE_WALL_HEI       = 20.0 + BASE_WALL_THICK
    BASE_THICK = 4.0
    BOLT_RAD = 2.0 + TOLE
    BOLT_SIZE = 41.0 / 2.0
    m1   = supalib.create_cyl( (0,0,0), size_z = BASE_WALL_HEI, radius = BASE_RADIUS )
    m1_h = supalib.create_cyl( (0,0,BASE_THICK), size_z = BASE_WALL_HEI + EPS, radius = BASE_MOT_HOLE_SIZE )
    m1_t = supalib.create_cyl( (0,0,-TOLE), size_z = 2*BASE_THICK , radius = BASE_THR_HOLE_SIZE )
    
    m1 = supalib.create_cut( m1, m1_h ) 
    m1 = supalib.create_cut( m1, m1_t )
    
    b_hol1 = supalib.create_cyl( (0,-BOLT_SIZE,-TOLE), size_z = 2*BASE_THICK, radius = BOLT_RAD )
    b_hol2 = supalib.create_cyl( (0,BOLT_SIZE,-TOLE), size_z = 2*BASE_THICK, radius = BOLT_RAD )
    
    m1 =  supalib.create_cut( m1,  supalib.create_union( (b_hol1, b_hol2 ) ), name="mot mount" )
    
    return m1

def create_round_pad( BASE_THICK = 10.0 ):
    BASE_RADIUS         = 25.0
    BASE_DEPT_SIZE      = 14.0
    BASE_THR_HOLE_SIZE  = 4.0 + TOLE
    MOUNT_DEP           = 3.0 + TOLE
    SCREW_SIZE          = 8.0
    SCREW_X             = 20.0
    m1   = supalib.create_cyl( (0,0,0), size_z = BASE_THICK, radius = BASE_RADIUS )
    m1_t = supalib.create_cyl( (0,0,BASE_THICK - MOUNT_DEP), size_z = BASE_THICK , radius = BASE_DEPT_SIZE )
    m1_a  = supalib.create_cyl( (0,0,-TOLE), size_z = 2*BASE_THICK, radius = BASE_THR_HOLE_SIZE )
    b_hol1 = supalib.create_cyl( (0,SCREW_X,-TOLE), size_z = 2*BASE_THICK, radius = SCREW_SIZE )
    b_hol2 = supalib.create_cyl( (0,-SCREW_X,-TOLE), size_z = 2*BASE_THICK, radius = SCREW_SIZE )
    m1 = supalib.create_cut( m1, m1_t )
    m1 = supalib.create_cut( m1, m1_a )
    m1 =  supalib.create_cut( m1,  supalib.create_union( (b_hol1, b_hol2 ) ), name="mot pad" )
    
    return m1

def create_bottom_wings( WING_OFFSET_X = 25, BASE_THICK = 4.0 ):
    ANGLE = (180 - 90 - 28.955)*2
    WING_SIZE_Y = 20
    
    WING_SIZE_X   = 43
    BOLT_LOC      = 35
    BOLT_RAD  = 2.0 + TOLE
    
    def create_plate():
        b = supalib.create_box( (WING_SIZE_X-WING_OFFSET_X, WING_SIZE_Y, BASE_THICK ), place = (WING_OFFSET_X,-WING_SIZE_Y/2.0,0) )
        b_hol1 = supalib.create_cyl( (BOLT_LOC,0,-TOLE), size_z = 2*BASE_THICK, radius = BOLT_RAD )
        m1 =  supalib.create_cut( b,  b_hol1 )
        tr1 = supalib.create_triangle( 10, 2, 10, place=( WING_OFFSET_X + 10 + 1.0, -8, 0.0, BASE_THICK ), rotate=(0,0,1,180) )
        tr2 = supalib.create_triangle( 10, 2, 10, place=( WING_OFFSET_X + 10 + 1.0, +8, 0.0, BASE_THICK ), rotate=(0,0,1,180) )
        return supalib.create_union( (m1, tr1, tr2 ) )
    
    boxes = []
    for angle in ( 0, ANGLE, -ANGLE ):
        b = create_plate()
        b = supalib.relocate( b, place=(0,0,0), rotate=(0,0,1,angle))
        boxes.append( b )
        
    boxes = supalib.create_union( boxes )
    cut = supalib.create_cyl( (0,0,-TOLE), size_z = 40, radius = WING_SIZE_X )
    boxes = supalib.create_intersection( (boxes, cut), name="Wings" )
    return boxes
    
 
    
main_bottom = crete_bottom_plate()
main_bottom = supalib.relocate( main_bottom, (0,0,EPS) )
wings = create_bottom_wings()


part = supalib.create_union( (main_bottom, wings) )
App.ActiveDocument.recompute()

part = supalib.create_chamfer( part, radius=1.0, edges=[ part.Shape.Edges[x] for x in [ 13, 1,  25 ] ] )
part.Label = "Motor mount"
pads1 = create_bottom_wings( BASE_THICK = 10.0, WING_OFFSET_X = 30.0 )
pads1 = supalib.relocate( pads1, (0,0, -20) )
pads1.Label = "Pads Wings"

pads2 = create_round_pad( BASE_THICK = 10.0 )
pads2 = supalib.relocate( pads2, (0,0, -20) )
pads2.Label = "Pads Round"


supalib.creta_mesh_from( part )
supalib.creta_mesh_from( pads1 )
supalib.creta_mesh_from( pads2 )

App.ActiveDocument.recompute()
Gui.SendMsgToActiveView("ViewFit")




