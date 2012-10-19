class Floor < Bullet::BtMotionState
  attr_accessor :rigid_body

  def initialize(dynamicsWorld, sceneMgr)
    super()

    x = 10 # width
    y = 1   # depth
    z = 10 # height
    mass = 0.0

    # create a physics object
    @cshape = Bullet::BtBoxShape.new(Bullet::BtVector3.new(x, 1, z))
    inertia = Bullet::BtVector3.new()
    @cshape.calculate_local_inertia(mass, inertia)
    @rigid_body = Bullet::BtRigidBody.new(mass, self, @cshape, inertia)
    dynamicsWorld.add_rigid_body(@rigid_body)

    # create a floor mesh resource
    Ogre::MeshManager::get_singleton().create_plane("floor", 
                                                    Ogre::ResourceGroupManager.DEFAULT_RESOURCE_GROUP_NAME,
                                                    Ogre::Plane.new(Ogre::Vector3.UNIT_Y, 0), 
                                                    x * 2, z * 2, 10, 10, true, 1, 10, 10, 
                                                    Ogre::Vector3.UNIT_Z)
    @entity = sceneMgr.create_entity("Floor", "floor")
    @entity.set_material_name("Examples/Rockwall")
    @entity.set_cast_shadows(false)
    sceneMgr.get_root_scene_node().attach_object(@entity)

    # set this object's position
    @transform = Bullet::BtTransform.new()
    @transform.set_identity()
    @transform.set_origin(Bullet::BtVector3.new(0, -y, 0))
    @rigid_body.set_center_of_mass_transform(@transform)
  end

  def set_world_transform(worldTrans)
  end

  def get_world_transform(worldTrans)
  end

end
