import os
import importlib
import sys

sys.path.append( os.path.dirname(os.path.realpath(__file__)) )

import supalib

importlib.reload( supalib )
supalib.init()

EPS = 0.01
TOLE = 0.2

MBOLT_RAD = 2.0 + 2*TOLE
MBOLT_X   = 12.0 

BASE_THICK  = 3.0
BASE_RADIUS = 37/2.0
SIZE_X = 30.0
SIZE_Y = BASE_RADIUS
AXEL_RADIUS = 12/2.0 + TOLE

m1   = supalib.create_cyl( (SIZE_X,0,0), size_z = BASE_THICK, radius = BASE_RADIUS )
m2   = supalib.create_cyl( (-SIZE_X,0,0), size_z = BASE_THICK, radius = BASE_RADIUS )
m3   = supalib.create_box( (2*(SIZE_X + EPS), 2*SIZE_Y, BASE_THICK), place=( (-SIZE_X-EPS),-SIZE_Y,-EPS) )



part = supalib.create_union( (m1,m2,m3) )

ANGLE = 120

holes = []
for angle in ( 0, ANGLE, -ANGLE ):
   b = supalib.create_cyl( (0,MBOLT_X,-TOLE), size_z = BASE_THICK*2, radius = MBOLT_RAD )
   b = supalib.relocate( b, place=(0,0,0), rotate=(0,0,1,angle), relative=True)
   holes.append( b )


ahole = supalib.create_cyl( (0,0,-TOLE), size_z = 2*BASE_THICK, radius = AXEL_RADIUS)
holes.append( ahole )

bhole1 = supalib.create_cyl( (SIZE_X,0,-TOLE), size_z = 2*BASE_THICK, radius = MBOLT_RAD)
bhole2 = supalib.create_cyl( (-SIZE_X,0,-TOLE), size_z = 2*BASE_THICK, radius = MBOLT_RAD)
holes.append( bhole1 )
holes.append( bhole2 )


part  = supalib.create_cut( part, supalib.create_union( holes ) )
part = supalib.create_fillet( part, radius=1.0, edges = ( part.Shape.Edges[24], ) )
App.ActiveDocument.recompute()
Gui.SendMsgToActiveView("ViewFit")

supalib.creta_mesh_from( part )

