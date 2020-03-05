import os
import importlib
import sys

sys.path.append( os.path.dirname(os.path.realpath(__file__)) )

import supalib

importlib.reload( supalib )
supalib.init()

EPS = 0.01
TOLE = 0.2


COVER_FRONT_XSIZE = 210.0
COVER_BACK_XSIZE  = 140.0
COVER_YSIZE       = 245.0

SUPPORT_OFFSET = 20.0
COVER_HEIGH    = 100.0 + SUPPORT_OFFSET
BASE_THICK  = 1.6


SUPPORT_XSIZE = 30.0
SUPPORT_YSIZE = BASE_THICK
SUPPORT_ZSIZE = 10.0

HOLE_RAD       = 3.0/2.0 + TOLE

BATTERY_HOLEX_BACK = 35 
BATTERY_HOLEX_FRONT = 55 

POS_COVER_HOLE_UP = COVER_HEIGH -  BASE_THICK*2
ASSUMED_BASELAYER_THICK = 3.0

def create_cover_supports( cover_xsize ):
    bs1  = supalib.create_box( ( BASE_THICK*3, cover_xsize*4,  BASE_THICK*2 ), place=(-cover_xsize*(1/3) - BASE_THICK, -TOLE ,COVER_HEIGH - BASE_THICK - TOLE) )
    bs2  = supalib.create_box( ( BASE_THICK*3, cover_xsize*4,  BASE_THICK*2 ), place=(-cover_xsize*(2/3) - BASE_THICK, -TOLE ,COVER_HEIGH - BASE_THICK - TOLE) )

    bt3  = supalib.create_triangle( BASE_THICK*3, COVER_YSIZE, BASE_THICK*3, rotate=(0,1,0,180) )
    bt3  = supalib.relocate( bt3, place=( -cover_xsize + BASE_THICK*3 - EPS*2 , 0, COVER_HEIGH + EPS*2 ), relative=True )
   
    return supalib.create_union( ( bs1, bs2, bt3  ) )
    
def create_front( cover_xsize, create_adds = True, short_triangs = False ):
   COVER_LEN=5.0
    
    
   def relocate_to_front( w ):
       return supalib.relocate( w , place=(-cover_xsize,0,+SUPPORT_YSIZE), rotate=(0,1,0,180),relative=True )
   def relocate_to_right( w ):
       return supalib.relocate( w , place=(0.0,COVER_YSIZE+EPS,+SUPPORT_YSIZE), rotate=(1,0,0,180),relative=True )
      
  
   b   = supalib.create_box( ( cover_xsize + BASE_THICK, COVER_YSIZE + BASE_THICK*2, COVER_HEIGH + BASE_THICK ), place=(-cover_xsize-BASE_THICK,-BASE_THICK,+EPS) )
   b_m = supalib.create_box( ( cover_xsize, COVER_YSIZE, COVER_HEIGH ), place=(-cover_xsize,0, 0) )

   b_s = create_cover_supports( cover_xsize )


   triangle_size = cover_xsize
   if short_triangs:
       triangle_size -= COVER_LEN*1.5
       
   bt1  = supalib.create_triangle( BASE_THICK*3, triangle_size, BASE_THICK*3, rotate=(0,0,1,-90) )
   bt1  = supalib.relocate( bt1, place=(-cover_xsize,0,COVER_HEIGH-BASE_THICK*3), rotate=(1,0,0,-90), relative=True )
   
   bt2  = supalib.create_triangle( BASE_THICK*3, triangle_size, BASE_THICK*3, rotate=(0,0,1,-90) )
   bt2  = supalib.relocate( bt2, place=(-cover_xsize,COVER_YSIZE-BASE_THICK*3,COVER_HEIGH), rotate=(1,0,0,180), relative=True )
   #bt2  = supalib.relocate( bt2, place=(0,COVER_YSIZE/2.0,0 ), rotate=(1,0,0,-90), relative=True )
 
   
   b_s  = supalib.create_union( [b_s, bt1, bt2 ] )
   b_s  = supalib.create_intersection( [ b_s, b ] )
   
   
   cover = supalib.create_cut( b, b_m )
   cover = supalib.create_union( [cover, b_s] )
   
   def create_pad():
      b1 = supalib.create_box( ( COVER_LEN*2 , BASE_THICK, COVER_HEIGH - BASE_THICK ), place=(-COVER_LEN,-EPS,0) )
      b2 = supalib.create_box( ( COVER_LEN*2 , BASE_THICK, COVER_HEIGH - BASE_THICK ), place=(-COVER_LEN,COVER_YSIZE+EPS-BASE_THICK,0) )
      b3 = supalib.create_box( ( COVER_LEN*2 , COVER_YSIZE, BASE_THICK ), place=(-COVER_LEN,0,COVER_HEIGH-BASE_THICK-EPS) )
      return supalib.create_union( (b1,b2,b3) )
  
   if create_adds:
      pads = create_pad()
      cover = supalib.create_union( (cover, pads) )
   

 
   def create_w1():
      w1  = supalib.create_triangle( SUPPORT_XSIZE, SUPPORT_YSIZE, SUPPORT_ZSIZE, rotate=(1,0,0,-90) )
      supalib.relocate( w1, place=(-SUPPORT_XSIZE,-EPS,SUPPORT_YSIZE), relative=True )
      return w1
   
   w1_b = create_w1()
   w1_f = create_w1()
   
 
   w1_f = relocate_to_front( w1_f )
   
   w2_b = create_w1()
   w2_b = relocate_to_right( w2_b )
   w2_f = create_w1()
   w2_f = relocate_to_right( relocate_to_front(w2_f) )
   
   wings = supalib.create_union( (w1_f, w1_b, w2_f, w2_b) )
   wings = supalib.relocate( wings, place=(0,0,SUPPORT_OFFSET), relative=True)
   
   cover = supalib.create_union( (cover, wings) )
   
   cyls = []
   
   HOLE_OFF_X  = -cover_xsize + SUPPORT_XSIZE*0.5
   HOLE_OFF_XL = cover_xsize - 2*SUPPORT_XSIZE*0.5 
   HOLE_OFF_Y  = -BASE_THICK*2
   HOLE_OFF_Z  = SUPPORT_OFFSET - ASSUMED_BASELAYER_THICK - HOLE_RAD/2.0
   
   for xloop in [0,1]:
      s = supalib.create_cyl( place=( HOLE_OFF_X + xloop*HOLE_OFF_XL, +COVER_YSIZE+BASE_THICK*2, HOLE_OFF_Z), rotate=(1,0,0,90), radius = HOLE_RAD, size_z=COVER_YSIZE*2.0)
      cyls.append(s)
   
   cover = supalib.create_cut( cover, supalib.create_union( cyls ) )
   
   return cover
      
      
def create_front_holes():
    holes = []
    size_y = 20
    for bumber_x in [ 38, 80, 162, 204 ]:
        b = supalib.create_box( (BASE_THICK*4, size_y, 18 + SUPPORT_OFFSET ), place=( -COVER_FRONT_XSIZE - 2*BASE_THICK, bumber_x - size_y/2.0, 0.0 ) )
        holes.append(b)
    
    axel_size=20
    axel_offset = 95
    for yloop in [1,0]:
        b = supalib.create_box( (axel_size, BASE_THICK*4, 35 + SUPPORT_OFFSET ), place=( -COVER_FRONT_XSIZE - axel_size/2.0 + axel_offset, yloop * COVER_YSIZE - 2*BASE_THICK, 0.0 ) )
        holes.append(b)
    
    
    UI_OFFSET=-20
    UI_OFFSET_EXTRA=-10
    POS_BUTTON = +20
    POS_LED    = +35
    HOLE_BUT_RAD = 3.0 + 1.25*TOLE
    HOLE_LED_RAD = 2.5 + 1.25*TOLE
    
    for xloop in [0,1]:
        xpos = UI_OFFSET + xloop*UI_OFFSET_EXTRA
        
        s1 = supalib.create_cyl( place=( xpos, +POS_BUTTON, POS_COVER_HOLE_UP ), rotate=(1,0,0,0), radius = HOLE_BUT_RAD, size_z=COVER_YSIZE*2.0)
        s2 = supalib.create_cyl( place=( xpos, +POS_LED, POS_COVER_HOLE_UP ), rotate=(1,0,0,0), radius = HOLE_LED_RAD, size_z=COVER_YSIZE*2.0)
        holes += [s1, s2]
            
    bat_xsize = BATTERY_HOLEX_FRONT
    bay_ysize = 120
    bat_hole = supalib.create_box( ( bat_xsize*2, bay_ysize, BASE_THICK*4), place=(-bat_xsize, COVER_YSIZE/2 - bay_ysize/2, POS_COVER_HOLE_UP ) )   
    holes.append( bat_hole )
    return supalib.create_union( holes )

def create_back_holes():
    bat_xsize = BATTERY_HOLEX_BACK
    bay_ysize = 120
    bat_hole = supalib.create_box( ( bat_xsize*2, bay_ysize, BASE_THICK*4), place=(-bat_xsize, COVER_YSIZE/2 - bay_ysize/2, POS_COVER_HOLE_UP ) )   
    return bat_hole
    
front = create_front( COVER_FRONT_XSIZE )
front_holes = create_front_holes()
front = supalib.create_cut( front, front_holes, name="Front cover")

back  = create_front( COVER_BACK_XSIZE, create_adds = False, short_triangs = True )
back_holes = create_back_holes()
back = supalib.create_cut( back, back_holes, name="Back cover")


def create_split_support( cover_xsize, cover_hole_offset ):
    bs1  = supalib.create_box( ( cover_xsize - cover_hole_offset,  BASE_THICK*4,  BASE_THICK ), place=( -cover_xsize, COVER_YSIZE/2.0 - BASE_THICK*2 ,COVER_HEIGH - BASE_THICK + EPS ) )
    bs2  = supalib.create_box( ( BASE_THICK,  BASE_THICK*4,  COVER_HEIGH ), place=( -cover_xsize - EPS,  COVER_YSIZE/2.0 - BASE_THICK*2 , 0 ) )

    bb = supalib.create_union( ( bs1, bs2 ) )
    bs_m = create_cover_supports( cover_xsize )
    bb = supalib.create_cut( bb, bs_m )
    return bb

def split_left_right( item, xsize, xsize_offset ):
   splitter = supalib.create_box( ( COVER_FRONT_XSIZE*4, COVER_YSIZE*2, COVER_HEIGH*2 ), place=(-COVER_FRONT_XSIZE*2, COVER_YSIZE/2.0 , 0) )
   item_l = supalib.create_intersection( [item, splitter] )
   item_r = supalib.create_cut( item, splitter )
   sup = create_split_support( xsize, xsize_offset )
   #sup = supalib.create_cut( sup, item_l )
   
   item_r = supalib.create_union( (sup, item_r) )
   item_l.Label=item.Label + "_left"
   item_r.Label=item.Label + "_right" 
   return [ item_l, item_r ]

parts = []
parts += split_left_right( front, COVER_FRONT_XSIZE, BATTERY_HOLEX_FRONT )
parts += split_left_right( back, COVER_BACK_XSIZE, BATTERY_HOLEX_BACK )

supalib.relocate( parts[2] , place=(10, +COVER_YSIZE, 0), rotate=(0,0,1,180) )
supalib.relocate( parts[3] , place=(10, +COVER_YSIZE, 0), rotate=(0,0,1,180) )

App.ActiveDocument.recompute()
Gui.SendMsgToActiveView("ViewFit")


for item in parts:
    supalib.creta_mesh_from( item )
