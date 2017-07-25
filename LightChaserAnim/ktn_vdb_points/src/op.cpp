// Description: <Description of your Op>
// Author: john
// Date: 2016-03-29

#include <iostream>
#include <string>
#include <stdio.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

// FnDefaultAssetPlugin and FnDefaultFileSequencePlugin initialization is
// currently active.
// The FNGEOLIBOP_INIT_FNASSET_HOSTS flag will be also passed to the compiler
// when building this Op.
//
// Notice that initializing FnDefaultAsset and FnDefaultFileSequence plug-ins
// will increase the library size and the compile time.
//
// Use 'CreateOp -a' or CreateOp --no-assetapi-init' to avoid adding compiler
// flags and code to initialize FnAsset API for an Op.
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>

#include <openvdb/openvdb.h>
#include <openvdb/tree/LeafManager.h>

#define OP_ERROR(MSG) { interface.setAttr("type", FnAttribute::StringAttribute("error"));               \
            interface.setAttr("errorMessage", FnAttribute::StringAttribute(MSG));                       \
            interface.stopChildTraversal();                                                             \
            return;}
const std::string cVdbValueGridName("density");
enum VisualizeType{
    kVisualizeTypeActiveVoxel,
    kVisualizeTypeTiles
};

namespace { //anonymous

class VdbPointsOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface);
   
private:
    static FnAttribute::Attribute buildTransform(openvdb::FloatGrid::Ptr grid_ptr)
    {
        openvdb::math::Transform::ConstPtr xform_ptr = grid_ptr->transformPtr();
        const openvdb::math::Vec3d scale = xform_ptr->voxelSize();
        const openvdb::Vec3d origin = xform_ptr->indexToWorld(openvdb::Coord(0,0,0));

        FnKat::GroupBuilder gb;

        const double translate[] = { origin[0], origin[1], origin[2] };
        gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));

        const double rxValues[] = { 0.0, 1.0, 0.0, 0.0 };
        const double ryValues[] = { 0.0, 0.0, 1.0, 0.0 };
        const double rzValues[] = { 0.0, 0.0, 0.0, 1.0 };
        gb.set("rotateX", FnKat::DoubleAttribute(rxValues, 4, 4));
        gb.set("rotateY", FnKat::DoubleAttribute(ryValues, 4, 4));
        gb.set("rotateZ", FnKat::DoubleAttribute(rzValues, 4, 4));

        // const double scaleValues[] = { scale, scale, scale };
        gb.set("scale", FnKat::DoubleAttribute(scale.asPointer(), 3, 3));
        gb.setGroupInherit(false);
        return gb.build();
    } 

    static float genRandom(unsigned seed) {
        srand(seed);
        return ((float)rand())/((float)RAND_MAX+1);
    }

};

void VdbPointsOp::cook(Foundry::Katana::GeolibCookInterface &interface)
{
    FnAttribute::StringAttribute vdb_path_arg = interface.getOpArg("vdb_path");
    if (!vdb_path_arg.isValid())
        OP_ERROR("No Vdb file specified");
    const std::string vdb_path = vdb_path_arg.getValue();
    if (vdb_path=="")
        OP_ERROR("vdb_path is empty");

    // open and read
    openvdb::initialize();
    openvdb::io::File vdb_file(vdb_path);
    try {
        vdb_file.open();
    } catch(...) {
        OP_ERROR("Failed to open vdb file");
    }
    openvdb::GridBase::Ptr grid_generic = vdb_file.readGrid(cVdbValueGridName);
    vdb_file.close();

    // parameters
    FnAttribute::GroupAttribute a = interface.getOpArg();

    FnAttribute::IntAttribute visualize_type_arg = a.getChildByName("visualize_type");
    FnAttribute::FloatAttribute visualize_active_percent_arg = a.getChildByName("visualize_active_percent");
    int visualize_type = visualize_type_arg.getValue(kVisualizeTypeActiveVoxel, false);
    float visualize_active_percent = visualize_active_percent_arg.getValue(0.1, false);

    // process
    openvdb::FloatGrid::Ptr grid_ptr = openvdb::gridPtrCast<openvdb::FloatGrid>(grid_generic);
    openvdb::FloatTree::Ptr tree = grid_ptr->treePtr();
    std::vector<float> P;
    unsigned i = 0;
    if (visualize_type == kVisualizeTypeActiveVoxel) {
        for (openvdb::FloatTree::ValueOnCIter iter=tree->cbeginValueOn(); iter; ++iter){
            if (genRandom(i++) > visualize_active_percent) continue;
            const openvdb::Coord coord = iter.getCoord();
            P.push_back((float)coord[0]);
            P.push_back((float)coord[1]);
            P.push_back((float)coord[2]);
        }
    }
    else if (visualize_type == kVisualizeTypeTiles) {
        typedef openvdb::tree::LeafManager<openvdb::FloatTree> FloatLM;
        FloatLM lm(*tree);
        FloatLM::LeafRange range = lm.leafRange();
        P.resize(lm.leafCount()*3);
        float offset = FloatLM::LeafNodeType::dim()*.5;
        for(FloatLM::LeafRange::Iterator iter = range.begin(); iter!=range.end(); ++iter){
            const openvdb::Coord coord = iter->origin();
            P.push_back((float)coord[0] + offset);
            P.push_back((float)coord[1] + offset);
            P.push_back((float)coord[2] + offset);
        }
    } else{
        OP_ERROR("Ridiculous");
    }

    // attribute visualization
    interface.setAttr("geometry.point.P", FnAttribute::FloatAttribute(P.data(), P.size(), 3));
    interface.setAttr("type", FnAttribute::StringAttribute("pointcloud"));
    interface.setAttr("xform", buildTransform(grid_ptr));
    interface.stopChildTraversal();
}

DEFINE_GEOLIBOP_PLUGIN(VdbPointsOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(VdbPointsOp, "VdbPoints", 0, 1);
    std::cout<<"[LCA PLUGIN] Register VdbPoints v1.0"<<std::endl;
}

