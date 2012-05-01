#-------------------------------------------------------------------------------
# Name:        demo_masonry
#
# This file shows how to
#   - create a small stack of bricks,
#   - create a support that shakes like an earthquake, with imposed motion law
#   - simulate the bricks that fall
#   - output the postprocessing data for rendering the animation with POVray
#-------------------------------------------------------------------------------
#!/usr/bin/env python

def main():
    pass

if __name__ == '__main__':
    main()


# Load the Chrono::Engine unit and the postprocessing unit!!!
import ChronoEngine_PYTHON_core as chrono
import ChronoEngine_PYTHON_postprocess as postprocess

# We will create two directories for saving some files, we need this:
import os
import math


# Create a physical system,
my_system = chrono.ChSystem()


# Set the default outward/inward shape margins for collision detection,
# this is epecially important for very large or very small objects.
chrono.ChCollisionModel.SetDefaultSuggestedEnvelope(0.001)
chrono.ChCollisionModel.SetDefaultSuggestedMargin(0.001)


# Create a contact material (surface property)to share between all objects.
# The rolling and spinning parameters are optional - if enabled they double
# the computational time.
brick_material = chrono.ChMaterialSurfaceShared()
brick_material.SetFriction(0.5)
brick_material.SetDampingF(0.2)
brick_material.SetCompliance (0.0000001)
brick_material.SetComplianceT(0.0000001)
# brick_material.SetRollingFriction(rollfrict_param)
# brick_material.SetSpinningFriction(0)
# brick_material.SetComplianceRolling(0.0000001)
# brick_material.SetComplianceSpinning(0.0000001)



# Create the set of bricks in a vertical stack, along Y axis

nbricks_on_x = 1
nbricks_on_y = 6

size_brick_x = 0.25
size_brick_y = 0.12
size_brick_z = 0.12
density_brick = 1000;    # kg/m^3
mass_brick = density_brick * size_brick_x * size_brick_y * size_brick_z;
inertia_brick = 2/5*(pow(size_brick_x,2))*mass_brick; # to do: compute separate xx,yy,zz inertias

for ix in range(0,nbricks_on_x):
    for iy in range(0,nbricks_on_y):
        # create it
        body_brick = chrono.ChBodyShared()
        # set initial position
        body_brick.SetPos(chrono.ChVectorD(ix*size_brick_x, (iy+0.5)*size_brick_y, 0 ))
        # set mass properties
        body_brick.SetMass(mass_brick)
        body_brick.SetInertiaXX(chrono.ChVectorD(inertia_brick,inertia_brick,inertia_brick))
        # set collision surface properties
        body_brick.SetMaterialSurface(brick_material)

        # Collision shape
        body_brick.GetCollisionModel().ClearModel()
        body_brick.GetCollisionModel().AddBox(size_brick_x/2, size_brick_y/2, size_brick_z/2) # must set half sizes
        body_brick.GetCollisionModel().BuildModel()
        body_brick.SetCollide(True)

        # Visualization shape, for rendering animation
        body_brick_shape = chrono.ChBoxShapeShared()
        body_brick_shape.GetBoxGeometry().Size = chrono.ChVectorD(size_brick_x/2, size_brick_y/2, size_brick_z/2)
        if iy%2==0 :
            body_brick_shape.SetColor(chrono.ChColor(0.8, 0.8, 0.8)) # set gray color only for odd bricks
        body_brick.GetAssets().push_back(body_brick_shape)

        my_system.Add(body_brick)


# Create the room floor: a simple fixed rigid body with a collision shape
# and a visualization shape

body_floor = chrono.ChBodyShared()
body_floor.SetBodyFixed(True)
body_floor.SetPos(chrono.ChVectorD(0, -2, 0 ))
body_floor.SetMaterialSurface(brick_material)

# Collision shape (shared by all particle clones)
body_floor.GetCollisionModel().ClearModel()
body_floor.GetCollisionModel().AddBox(3, 1, 3) # hemi sizes
body_floor.GetCollisionModel().BuildModel()
body_floor.SetCollide(True)

# Visualization shape (shared by all particle clones)
body_floor_shape = chrono.ChBoxShapeShared()
body_floor_shape.GetBoxGeometry().Size = chrono.ChVectorD(3, 1, 3)
body_floor_shape.SetColor(chrono.ChColor(0.5,0.5,0.5))
body_floor.GetAssets().push_back(body_floor_shape)

my_system.Add(body_floor)



# Create the shaking table, as a box

size_table_x = 1;
size_table_y = 0.2;
size_table_z = 1;

body_table = chrono.ChBodyShared()
body_table.SetPos(chrono.ChVectorD(0, -size_table_y/2, 0 ))
body_table.SetMaterialSurface(brick_material)

# Collision shape (shared by all particle clones)
body_table.GetCollisionModel().ClearModel()
body_table.GetCollisionModel().AddBox(size_table_x/2, size_table_y/2, size_table_z/2) # hemi sizes
body_table.GetCollisionModel().BuildModel()
body_table.SetCollide(True)

# Visualization shape (shared by all particle clones)
body_table_shape = chrono.ChBoxShapeShared()
body_table_shape.GetBoxGeometry().Size = chrono.ChVectorD(size_table_x/2, size_table_y/2, size_table_z/2)
body_table_shape.SetColor(chrono.ChColor(0.4,0.4,0.5))
body_table.GetAssets().push_back(body_table_shape)

my_system.Add(body_table)


# Create a constraint that blocks free 3 x y z translations and 3 rx ry rz rotations
# of the table respect to the floor, and impose that the relative imposed position
# depends on a specified motion law.

link_shaker = chrono.ChLinkLockLockShared()
link_shaker.Initialize(body_table, body_floor, chrono.CSYSNORM)
my_system.Add(link_shaker)

# ..create the function for imposed x horizontal motion, etc.
mfunY = chrono.ChFunction_Sine(0,1.5,0.001)  # phase, frequency, amplitude
mfunY.thisown=0      # because the link will take care of deletion!
link_shaker.SetMotion_Y(mfunY)

# ..create the function for imposed y vertical motion, etc.
mfunZ = chrono.ChFunction_Sine(0,1.5,0.12)  # phase, frequency, amplitude
mfunZ.thisown=0      # because the link will take care of deletion!
link_shaker.SetMotion_Z(mfunZ)

# Note that you could use other types of ChFunction_ objects, or create
# your custom function by class inheritance (see demo_python.py), or also
# set a function for table rotation , etc.



#  Since we want to render a short animation by generating scripts
#  to be used with POV-Ray, a ChPovRay postprocessing tool must be created
#  and some parameters to be set..

pov_exporter = postprocess.ChPovRay(my_system)

 # Sets some file names for in-out processes.
pov_exporter.SetTemplateFile        ("../../../data/_template_POV.pov")
pov_exporter.SetOutputScriptFile    ("rendering_frames.pov")
if not os.path.exists("output"):
    os.mkdir("output")
if not os.path.exists("anim"):
    os.mkdir("anim")
pov_exporter.SetOutputDataFilebase("output/my_state")
pov_exporter.SetPictureFilebase("anim/picture")

pov_exporter.SetCamera(chrono.ChVectorD(1.2,0.5,2.5), chrono.ChVectorD(0,0.4,0), 35)
pov_exporter.SetLight(chrono.ChVectorD(-2,2,-1), chrono.ChColor(1.1,1.2,1.2), 1)
pov_exporter.SetPictureSize(640,480)
pov_exporter.SetAmbientLight(chrono.ChColor(2,2,2))

 # Add additional POV objects/lights/materials in the following way
pov_exporter.SetCustomPOVcommandsScript(
'''
light_source{ <1,3,1.5> color rgb<1.1,1.1,1.1> }
// Grid(0.05,0.04, rgb<0.7,0.7,0.7>, rgbt<1,1,1,1>)
''')

 # Tell which physical items you want to render
pov_exporter.AddAll()

 # Tell that you want to render the contacts
##pov_exporter.SetShowContacts(1,
##                            postprocess.ChPovRay.SYMBOL_VECTOR_SCALELENGTH,
##                            0.001,  # scale
##                            0.002,  # width
##                            0.2,    # max size
##                            1,0,0.5 ) # colormap on, blue at 0, red at 0.5

 # 1) Create the two .pov and .ini files for POV-Ray (this must be done
 #    only once at the beginning of the simulation).
pov_exporter.ExportScript()



# If you want to change some settings for the solver, here is the right place
# to do it. For example you might want to use SetIterLCPmaxItersSpeed to set
# the number of iterations per timestep, etc.

#my_system.SetLcpSolverType(chrono.ChSystem.LCP_ITERATIVE_BARZILAIBORWEIN) # precise, more slow
my_system.SetIterLCPmaxItersSpeed(70)


# Now perform the simulation by advancing the timestep in a
# loop. For each timestep also write the data for postprocessing.

while (my_system.GetChTime() < 2) :

    my_system.DoStepDynamics(0.005)

    print ('time=', my_system.GetChTime() )

    pov_exporter.ExportData()



