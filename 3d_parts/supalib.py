
import MeshPart
import FreeCAD as App
import FreeCADGui as Gui

import math
from pathlib import Path

EPS = 0.01



def show( source, visible ):
   source.ViewObject.Visibility=visible

GLOBAL_object_loop = 1

def make_name( name, prefix ):
    if name:
        return name
    
    global GLOBAL_object_loop
    
    name = "Supa_%s_%d" % (prefix, GLOBAL_object_loop )
    GLOBAL_object_loop += 1
    return name



def create_triangle( xsize, ysize, zsize, place=(0,0,0), rotate=(1,0,0,0) ):
    """ Create triangle that is triangle in XZ coordinages, with Y beeing the thickness" """
    
    b1 = create_box( (xsize,ysize,zsize), place=(0,0,0 ) )
    ms = max( xsize, zsize )
    angle = math.atan( zsize / xsize ) * 180 / math.pi
    b2 = create_box( (2*ms, 2*ysize, 2*ms), place=(0,-EPS,0), rotate=(0,1,0,-angle) )
    tr = create_cut( b1, b2 )
    return relocate( tr, place, rotate )
    
    
def create_cut( to_be_cutted, to_cut_with, name=None ):
    cutted = App.activeDocument().addObject("Part::Cut", make_name( None,"cut" ) )
    cutted.Base = to_be_cutted
    cutted.Tool = to_cut_with
    to_be_cutted.ViewObject.Visibility=False
    to_cut_with.ViewObject.Visibility=False
    App.ActiveDocument.recompute()    
    if name != None:
        cutted.Label=name
    return cutted

def create_union( shapes, name = None ):
    union = App.activeDocument().addObject("Part::MultiFuse",make_name( None,"union" ))
    for shape in shapes:
        shape.ViewObject.Visibility = False
    union.Shapes = shapes
    App.ActiveDocument.recompute()
    if name != None:
        union.Label = name
    return union

def create_bolt( radius_small, radius_large, size_z_small, size_z_large, place=(0,0,0), rotate=(1,0,0,0) ):
    cyl_large = create_cyl( place=(0,0,0), radius=radius_large, size_z=size_z_large + EPS)
    cyl_small = create_cyl( place=(0,0,size_z_large), radius=radius_small, size_z=size_z_small )
    cyls = create_union( [ cyl_large, cyl_small ] )
    vec_place = App.Vector( place[0], place[1], place[2] )
    vec_rot   = App.Rotation( App.Vector(rotate[0],rotate[1],rotate[2]), rotate[3] ) 
    show(cyl_large, False)
    show(cyl_small, False)
    cyls.Placement = App.Placement( vec_place, vec_rot )
    return cyls


def create_sphere( radius, place=(0,0,0), rotate=(1,0,0,0), name=None ):
    name = make_name( name, "Sphere" )
    sp = App.ActiveDocument.addObject("Part::Sphere", name )
    sp.Radius = '%f mm' % radius
    vec_place = App.Vector( place[0], place[1], place[2] )
    vec_rot   = App.Rotation( App.Vector(rotate[0],rotate[1],rotate[2]), rotate[3] ) 
    sp.Placement = App.Placement( vec_place, vec_rot )
    return sp

def create_cyl( radius, size_z, place=(0,0,0), rotate=(1,0,0,0), name=None ):
    name = make_name( name, "Cyl" )
    cyl = App.ActiveDocument.addObject("Part::Cylinder", name )
    cyl.Height = '%f mm' % size_z
    cyl.Radius = '%f mm' % radius
    vec_place = App.Vector( place[0], place[1], place[2] )
    vec_rot   = App.Rotation( App.Vector(rotate[0],rotate[1],rotate[2]), rotate[3] ) 
    cyl.Placement = App.Placement( vec_place, vec_rot )
    return cyl

def create_box( size, place=(0,0,0), rotate=(1,0,0,0), name=None ):
    name = make_name( name, "box" )
    box=  App.ActiveDocument.addObject("Part::Box", name )
    box.Width =  '%f mm' % size[1]
    box.Length = '%f mm' % size[0]
    box.Height = '%f mm' % size[2]
    vec_place = App.Vector( place[0], place[1], place[2] )
    vec_rot   = App.Rotation( App.Vector(rotate[0],rotate[1],rotate[2]), rotate[3] ) 
    box.Placement = App.Placement( vec_place, vec_rot )
    return box

def create_fillet( obj, radius = 1.0, edges = None, name = None ):
    name = make_name( name, "fillet" )
    fillet = App.ActiveDocument.addObject("Part::Fillet", name)
    fillet.Base = obj
    if edges == None:
        edges = obj.Shape.Edges
    fillet.Shape = obj.Shape.makeFillet( radius, edges )
    obj.ViewObject.Visibility = False
    App.ActiveDocument.recompute()
    return fillet

def create_chamfer( obj, edges, radius=1.0, name = None ):
    name = make_name( name, "chamfer" )
    cham = App.ActiveDocument.addObject("Part::Chamfer", name)
    cham.Base = obj
    cham.Shape = obj.Shape.makeChamfer( radius, edges )
    obj.ViewObject.Visibility = False
    App.ActiveDocument.recompute()
    return cham
    
    
def create_intersection( shapes, name = None ):
    name = make_name( name, "intersection" )
    comm = App.activeDocument().addObject("Part::MultiCommon",name )
    for shape in shapes:
        shape.ViewObject.Visibility = False
    comm.Shapes = shapes 
    App.ActiveDocument.recompute()
    return comm

 
def relocate( obj, place=(0,0,0), rotate=(1,0,0,0), relative=True ):
    vec_place = App.Vector( place[0], place[1], place[2] )
    vec_rot   = App.Rotation( App.Vector(rotate[0],rotate[1],rotate[2]), rotate[3] ) 
    
    
    if relative == True:
        pos = obj.Placement.toMatrix()
        new_place = App.Placement( vec_place, vec_rot ).toMatrix() * pos
        
    else:
        new_place = App.Placement( vec_place, vec_rot )
        
    obj.Placement = new_place
    return obj

def creta_mesh_from( source, name = None, save_to = None, save_format = "stl", version = 0 ):
   name = make_name( name, "mesh" )
   mesh = App.activeDocument().addObject("Mesh::Feature", name)
   __shape = source.Shape.copy(False)
   __shape.Placement = source.getGlobalPlacement()
   mesh.Mesh = MeshPart.meshFromShape(Shape=__shape, LinearDeflection=0.1, AngularDeflection=0.523599, Relative=False)
   mesh.Label=source.Label + " (Meshed)"
   del __shape
   show( source, False )
   
   save_version = "v%d" % version 
   if save_to:
       fn = ( save_to + source.Label + "_" + save_version ).lower() + "." + save_format
       mesh.Mesh.write( fn )
       print("Saved to: %s" % fn )
   return mesh 

def finish():
   App.ActiveDocument.recompute()
   Gui.SendMsgToActiveView("ViewFit")

def init( name ="Unnamed" ):
    try:
        App.closeDocument("Unnamed")
    except NameError:
        pass
    App.newDocument("Unnamed")
    App.setActiveDocument("Unnamed")
    App.ActiveDocument=App.getDocument("Unnamed")
    Gui.ActiveDocument=Gui.getDocument("Unnamed")
    Gui.activeDocument().activeView().viewDefaultOrientation()
