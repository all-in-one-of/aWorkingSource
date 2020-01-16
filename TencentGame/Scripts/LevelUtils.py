import unreal

# for i in dir(unreal.Actor()):
#     print i

# unreal.EditorLevelLibrary().new_level("/Game/MyFolder/MyMap")

world = unreal.EditorLevelLibrary.get_editor_world()
actor = unreal.EditorLevelLibrary().spawn_actor_from_class(unreal.StaticMeshActor, [0,0,0])

actor.static_mesh_component.set_static_mesh("StaticMesh'/Game/StarterContent/Props/SM_MatPreviewMesh_02.SM_MatPreviewMesh_02'")
# unreal.EditorLevelLibrary().destroy_actor(actor)
for i in dir(actor.static_mesh_component):
    print i
