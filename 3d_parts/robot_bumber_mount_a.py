import supalib

EPS = 0.01
TOLE = 0.2

BUMBER_STICK_SEPARATION = 30
BUMBER_STICK_XSIZE = 10
BUMBER_STICK_YSIZE = 10
BUMBER_STICK_LEN   = 120

MOUNT_B_OFFSET     = -70
MOUNT_THICKNESS    = 4
MOUNT_YSIZE        = 30
MOUNT_STIC_SIZE    = 2*MOUNT_THICKNESS + BUMBER_STICK_YSIZE


def crete_single_mount( with_hole ):
   raw  = supalib.create_box( size=(MOUNT_STIC_SIZE , MOUNT_YSIZE , MOUNT_STIC_SIZE  ), place = (0, 0, 0 ) )
   
   if with_hole == False:
       offset_y = -EPS
   else:
       offset_y = MOUNT_THICKNESS*0.5
       
   hole = supalib.create_box( size=( BUMBER_STICK_XSIZE + 2*TOLE,  MOUNT_YSIZE +  2*EPS, BUMBER_STICK_YSIZE + 2*TOLE ), place = (MOUNT_THICKNESS - TOLE, offset_y, MOUNT_THICKNESS - TOLE) )
   return supalib.create_cut( raw, hole )

def create_base_mount():
   raw = supalib.create_box( place=(-BUMBER_STICK_SEPARATION - EPS, -EPS,-TOLE), size=( 2*BUMBER_STICK_SEPARATION + 2*EPS, MOUNT_YSIZE + 2*EPS, MOUNT_THICKNESS ) )
   BOLT_SEP = MOUNT_YSIZE/4.0
   hole1 = supalib.create_cyl( place=(0, MOUNT_YSIZE*0.5 + BOLT_SEP, -1.0), radius=1.5 + TOLE, size_z=10)
   hole2 = supalib.create_cyl( place=(0, MOUNT_YSIZE*0.5 - BOLT_SEP, -1.0), radius=1.5 + TOLE, size_z=10)
   holes = supalib.create_union(  (hole1, hole2 ) )
   return supalib.create_cut( raw, holes )


def create_full_part( with_hole, label ):
   base = create_base_mount()    
   mount_1 = crete_single_mount( with_hole )
   mount_2 = crete_single_mount( with_hole )
   supalib.relocate( mount_1, place=( BUMBER_STICK_SEPARATION - MOUNT_STIC_SIZE, 0, -EPS ) )
   supalib.relocate( mount_2, place=( -BUMBER_STICK_SEPARATION, 0, -EPS ) )
   mount = supalib.create_union( ( mount_1, mount_2, base ) )
   mount.Label = label
   return mount

STICK_PLACE_A=(BUMBER_STICK_SEPARATION - MOUNT_STIC_SIZE + (MOUNT_STIC_SIZE - BUMBER_STICK_XSIZE)*0.5 - TOLE*0.0 , MOUNT_B_OFFSET + 10.0, MOUNT_THICKNESS )
STICK_PLACE_B=(-BUMBER_STICK_SEPARATION + (MOUNT_STIC_SIZE - BUMBER_STICK_XSIZE)*0.5 + TOLE*0.0, MOUNT_B_OFFSET + 10.0, MOUNT_THICKNESS )
def create_stick():
  thick = (MOUNT_STIC_SIZE - BUMBER_STICK_XSIZE)*0.5
  bsize = BUMBER_STICK_XSIZE - 2 *TOLE
  b1 = supalib.create_box( size=( bsize,  BUMBER_STICK_LEN, bsize), place =STICK_PLACE_A  )
  b2 = supalib.create_box( size=( bsize,  BUMBER_STICK_LEN, bsize), place =STICK_PLACE_B  )
  b3 = supalib.create_box( size=( 2*BUMBER_STICK_SEPARATION - 2*EPS - MOUNT_STIC_SIZE + bsize*0.25,  bsize, bsize), place=( bsize*0.25 + -BUMBER_STICK_SEPARATION + thick + EPS, -BUMBER_STICK_XSIZE - TOLE, MOUNT_THICKNESS ) )
  b1 = supalib.create_fillet( b1 )
  b2 = supalib.create_fillet( b2 )
  part = supalib.create_union( ( b1,b2,b3) )
  part.Label = "Stick"
  return part

def create_bumber():
    BUMBER_SIZE_MINUS = 60 + BUMBER_STICK_SEPARATION
    BUMBER_SIZE_PLUS  = 30 + BUMBER_STICK_SEPARATION
    BUMBER_MOUNT_SIZE = BUMBER_STICK_XSIZE + 2*MOUNT_THICKNESS + TOLE
    def create_bmount():
        raw  = supalib.create_box( size=( BUMBER_MOUNT_SIZE,  MOUNT_THICKNESS, BUMBER_MOUNT_SIZE) )
        hole = supalib.create_box( size=( BUMBER_STICK_XSIZE + 2*TOLE,  MOUNT_THICKNESS*0.5, BUMBER_STICK_XSIZE + 2*TOLE  ), place=(MOUNT_THICKNESS - TOLE, 0.0, MOUNT_THICKNESS - TOLE) )
        return supalib.create_cut( raw, hole )

    m1 = create_bmount()
    m2 = create_bmount()
    
    s0 = supalib.create_box( size=( BUMBER_SIZE_MINUS + BUMBER_SIZE_PLUS,  MOUNT_THICKNESS, MOUNT_THICKNESS ), place=(-BUMBER_SIZE_MINUS,0,2*BUMBER_MOUNT_SIZE + MOUNT_THICKNESS) )
    s1 = supalib.create_box( size=( BUMBER_SIZE_MINUS + BUMBER_SIZE_PLUS,  MOUNT_THICKNESS, MOUNT_THICKNESS ), place=(-BUMBER_SIZE_MINUS,0,BUMBER_MOUNT_SIZE - EPS ) )
    s2 = supalib.create_box( size=( BUMBER_SIZE_MINUS + BUMBER_SIZE_PLUS,  MOUNT_THICKNESS, MOUNT_THICKNESS ), place=(-BUMBER_SIZE_MINUS,0,-MOUNT_THICKNESS - EPS) )
    s3 = supalib.create_box( size=( BUMBER_SIZE_MINUS + BUMBER_SIZE_PLUS,  MOUNT_THICKNESS, MOUNT_THICKNESS ), place=(-BUMBER_SIZE_MINUS,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    
    hsize = ( MOUNT_THICKNESS,  MOUNT_THICKNESS, 3*(MOUNT_THICKNESS + BUMBER_MOUNT_SIZE) + MOUNT_THICKNESS + EPS )
    d1 = supalib.create_box( size=hsize, place=(-BUMBER_SIZE_MINUS,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    d2 = supalib.create_box( size=hsize, place=( BUMBER_SIZE_PLUS - MOUNT_THICKNESS  ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    
    d3 = supalib.create_box( size=hsize, place=( -BUMBER_STICK_SEPARATION - MOUNT_THICKNESS  ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    d4 = supalib.create_box( size=hsize, place=( -BUMBER_STICK_SEPARATION + BUMBER_MOUNT_SIZE ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    d5 = supalib.create_box( size=hsize, place=( BUMBER_STICK_SEPARATION ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    d6 = supalib.create_box( size=hsize, place=( BUMBER_STICK_SEPARATION - BUMBER_MOUNT_SIZE - MOUNT_THICKNESS ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    
    
    d7 = supalib.create_box( size=hsize, place=( 0.5*( -BUMBER_STICK_SEPARATION - MOUNT_THICKNESS - BUMBER_SIZE_MINUS) ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    
    #d5 = supalib.create_box( size=hsize, place=( 2*BUMBER_STICK_SEPARATION  ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )
    #d6 = supalib.create_box( size=hsize, place=( 2*BUMBER_STICK_SEPARATION -BUMBER_MOUNT_SIZE -MOUNT_THICKNESS  ,0,-2*MOUNT_THICKNESS - BUMBER_MOUNT_SIZE) )

    sup  = supalib.create_union( (s0, s1,s2,s3,d1,d2,d3, d4, d5, d6, d7 ) )
    # ,d5,d6
    supalib.relocate( m1, place=(STICK_PLACE_A[0] - MOUNT_THICKNESS, STICK_PLACE_A[1] + BUMBER_STICK_LEN, 0 ) )
    supalib.relocate( m2, place=(STICK_PLACE_B[0] - MOUNT_THICKNESS, STICK_PLACE_A[1] + BUMBER_STICK_LEN, 0 ) )
    supalib.relocate( sup, place=(0.0, STICK_PLACE_A[1] + BUMBER_STICK_LEN, 0 ) )
    bumber = supalib.create_union( (sup, m1, m2 ) )
    bumber.Label="Bumber"
    return bumber
    
    
bumber = create_bumber()   
mount_a = create_full_part ( False, "Mount_front" )
mount_b = create_full_part ( True, "Mount_rear" )
supalib.relocate( mount_b, place=( 0.0, MOUNT_B_OFFSET , 0.0) )

stick = create_stick()


for x in [ bumber, mount_a, mount_b, stick ]:
   supalib.creta_mesh_from( x )

supalib.finish()




